/*===================================================================
 * Filename: common.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: ͨ��ģ�飬���õĺ��������ﶨ��
 *
 * Date:   2013-5-13 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "common.h"



/*========================================================================
*
* ������:	IxCreateFile
*
* ����:		PHANDLE		[OUT]	pFileHandle		- ���ص��ļ����ָ��
*			LPCWSTR		[IN]	FileName		- �ļ���
*			ACCESS_MASK	[IN]	DesiredAccess	-
*			ULONG		[IN]	ShareAccess		- 
*
* ��������:	���ļ�
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS
IxCreateFile(
			   PHANDLE	pFileHandle,
			   LPCWSTR	FileName,
			   ACCESS_MASK DesiredAccess,
			   ULONG ShareAccess
			   )
{
	NTSTATUS			Status;
	UNICODE_STRING		uniFileName;
	OBJECT_ATTRIBUTES	objectAttributes;
	IO_STATUS_BLOCK     ioStatus;

	if (KeGetCurrentIrql() > PASSIVE_LEVEL)
	{
		return STATUS_UNSUCCESSFUL;
	}

	RtlInitUnicodeString( &uniFileName, FileName );

	InitializeObjectAttributes(&objectAttributes, &uniFileName,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

	Status = IoCreateFile(
					pFileHandle,
					DesiredAccess | SYNCHRONIZE,
					&objectAttributes,
					&ioStatus,
					0,
					FILE_READ_ATTRIBUTES,
					ShareAccess,
					FILE_OPEN,
					FILE_SYNCHRONOUS_IO_NONALERT,
					NULL,
					0,
					0,
					NULL,
					IO_NO_PARAMETER_CHECKING
					);

	return Status;
}

/*========================================================================
*
* ������:	IxIoCallDriver
*
* ����:		[IN]	DeviceObject	-DeviceObjectΪ��һ���豸����ָ��
*			[IN]	Irp				-IRPָ��
*
* ��������:	ʵ�ֽ�IRP���͸���һ�����豸��
*
* ����ֵ:	�μ�WRK/DDK�е�IoCallDriver
*
=========================================================================*/
NTSTATUS
IxIoCallDriver(
			   IN PDEVICE_OBJECT DeviceObject,
			   IN OUT PIRP Irp
			   )
{
	PIO_STACK_LOCATION irpSp;
	PDRIVER_OBJECT driverObject;
	NTSTATUS status;

	//KdPrint(("[MyIoCallDriver] ���дμ��豸\n"));

	//
	// ȷ������һ��I/O Request Packet.
	//
	ASSERT( Irp->Type == IO_TYPE_IRP );
	//
	// ��IRP stackָ��ָ����һ���豸���ڴ�ռ�.
	//
	Irp->CurrentLocation--;

	if (Irp->CurrentLocation <= 0) {
		KeBugCheckEx( NO_MORE_IRP_STACK_LOCATIONS, (ULONG_PTR) Irp, 0, 0 ,0);
	}

	irpSp = IoGetNextIrpStackLocation( Irp );
	Irp->Tail.Overlay.CurrentStackLocation = irpSp;

	//
	//���µ�ǰIRPջΪ��һ���豸��Ӧ��ջ�ռ�
	//
	irpSp->DeviceObject = DeviceObject;

	//
	// ��ȡ�²��豸��Ӧ����������
	//
	driverObject = DeviceObject->DriverObject;

	//
	// ͨ�����������еĺ���ָ����ö�Ӧ����
	//
	status = driverObject->MajorFunction[irpSp->MajorFunction]( DeviceObject, Irp );

	return status;
}

/*========================================================================
*
* ������:	GetInfoTable
*
* ����:		SYSTEM_INFORMATION_CLASS		[IN]	SystemInformationClass
*
* ��������:	QuerySystemInformation
*
* ����ֵ:	PVOID
*
=========================================================================*/
PVOID GetInfoTable( SYSTEM_INFORMATION_CLASS SystemInformationClass )
{
	ULONG		mSize;
	PVOID		mPtr;
	NTSTATUS	Status;

	mSize = 0x4000;
	mPtr = NULL;

	do
	{
		mPtr = ExAllocatePool( PagedPool, mSize );
		memset( mPtr, 0, mSize );
		if ( mPtr != NULL )
		{
			Status = ZwQuerySystemInformation( SystemInformationClass, mPtr, mSize, NULL );
		}
		else
		{
			return NULL;
		}

		if ( Status == STATUS_INFO_LENGTH_MISMATCH )
		{
			ExFreePool( mPtr );
			mSize = mSize * 2;
		}

	} while ( Status == STATUS_INFO_LENGTH_MISMATCH );

	if (Status == STATUS_SUCCESS)
	{
		return mPtr;
	}

	ExFreePool( mPtr );
	return NULL;
}

/*========================================================================
*
* ������:	IxExFreePool
*
* ����:		[IN]	-pPool	�ڴ��ָ��
*
* ��������:	��ȫ�ͷŶ��ڴ��
*
* ����ֵ:	VOID
*
=========================================================================*/
VOID	IxExFreePool( PVOID pPool )
{
	if ( pPool )
	{
		ExFreePool( pPool );
		pPool = NULL;
	}
}

/*========================================================================
*
* ������:	IxObDereferenceObject
*
* ����:		[IN]	-pObject	����ָ��
*
* ��������:	��ȫ�����ö���
*
* ����ֵ:	VOID
*
=========================================================================*/
VOID	IxObDereferenceObject( PVOID pObject )
{
	if ( pObject )
	{
		ObDereferenceObject( pObject );
		pObject = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣�����һ����д���ڴ��MDL
//	���������MemAddressΪҪ�����Ŀ���׵�ַ��
//			  MdlLengthΪMDL��ĳ���
//			  pMdlPointerΪ������MDL���ָ��
//	�������������MDLָ����ڴ����ָ��
//	��ע���ô˺���������д��MDL�����Ҫ��FreeWriteMDL()�����ͷ�
///////////////////////////////////////////////////////////////////////////////////
PVOID
CreateWriteMDL(IN	ULONG	MemAddress,
			   IN	ULONG	MdlLength,
			   IN	OUT	PMDL	*pMdlPointer
			   )
{
	PMDL	pmdlPointer=NULL;
	PVOID	*MappedPointer=NULL;

	KdPrint(("[CreateWriteMDL] ����MDL\n"));

	pmdlPointer=MmCreateMdl(NULL,(PVOID)MemAddress,MdlLength);
	if(!pmdlPointer)
	{
		KdPrint(("[CreateWriteMDL] ����MDL��ʧ��!\n"));
		return NULL;
	}
	MmBuildMdlForNonPagedPool(pmdlPointer);
	pmdlPointer->MdlFlags = pmdlPointer->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA;
	MappedPointer= MmMapLockedPages(pmdlPointer, KernelMode);
	*pMdlPointer=pmdlPointer;
	return MappedPointer;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣��ͷ�CreateWriteMDL������MDL��
//	���������pMappedPointerΪMDL�鶨����ڴ���ָ��
//			  pMdlPointerΪ������MDL���ָ��
//	�������������MDLָ����ڴ����ָ��
//
///////////////////////////////////////////////////////////////////////////////////
VOID
FreeWriteMDL(IN	PMDL	pMdlPointer,IN	PVOID	pMappedPointer)
{
	KdPrint(("[FreeWriteMDL] �ͷ�MDL��!\n"));

	MmUnmapLockedPages(pMappedPointer,pMdlPointer);
	IoFreeMdl(pMdlPointer);
}