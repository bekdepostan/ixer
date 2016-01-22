/*===================================================================
 * Filename: common.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 通用模块，公用的函数在这里定义
 *
 * Date:   2013-5-13 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "common.h"



/*========================================================================
*
* 函数名:	IxCreateFile
*
* 参数:		PHANDLE		[OUT]	pFileHandle		- 返回的文件句柄指针
*			LPCWSTR		[IN]	FileName		- 文件名
*			ACCESS_MASK	[IN]	DesiredAccess	-
*			ULONG		[IN]	ShareAccess		- 
*
* 功能描述:	打开文件
*
* 返回值:	NTSTATUS
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
* 函数名:	IxIoCallDriver
*
* 参数:		[IN]	DeviceObject	-DeviceObject为次一级设备对象指针
*			[IN]	Irp				-IRP指针
*
* 功能描述:	实现将IRP发送给下一级的设备上
*
* 返回值:	参见WRK/DDK中的IoCallDriver
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

	//KdPrint(("[MyIoCallDriver] 呼叫次级设备\n"));

	//
	// 确定这是一个I/O Request Packet.
	//
	ASSERT( Irp->Type == IO_TYPE_IRP );
	//
	// 将IRP stack指针指向下一层设备的内存空间.
	//
	Irp->CurrentLocation--;

	if (Irp->CurrentLocation <= 0) {
		KeBugCheckEx( NO_MORE_IRP_STACK_LOCATIONS, (ULONG_PTR) Irp, 0, 0 ,0);
	}

	irpSp = IoGetNextIrpStackLocation( Irp );
	Irp->Tail.Overlay.CurrentStackLocation = irpSp;

	//
	//更新当前IRP栈为下一层设备对应的栈空间
	//
	irpSp->DeviceObject = DeviceObject;

	//
	// 获取下层设备对应的驱动对象
	//
	driverObject = DeviceObject->DriverObject;

	//
	// 通过驱动对象中的函数指针调用对应函数
	//
	status = driverObject->MajorFunction[irpSp->MajorFunction]( DeviceObject, Irp );

	return status;
}

/*========================================================================
*
* 函数名:	GetInfoTable
*
* 参数:		SYSTEM_INFORMATION_CLASS		[IN]	SystemInformationClass
*
* 功能描述:	QuerySystemInformation
*
* 返回值:	PVOID
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
* 函数名:	IxExFreePool
*
* 参数:		[IN]	-pPool	内存池指针
*
* 功能描述:	安全释放堆内存池
*
* 返回值:	VOID
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
* 函数名:	IxObDereferenceObject
*
* 参数:		[IN]	-pObject	对象指针
*
* 功能描述:	安全解引用对象
*
* 返回值:	VOID
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
//	功能实现：创建一个可写的内存块MDL
//	输入参数：MemAddress为要创建的块的首地址；
//			  MdlLength为MDL块的长度
//			  pMdlPointer为创建的MDL块的指针
//	输出参数：返回MDL指向的内存给的指针
//	备注：用此函数创建可写的MDL块后，需要用FreeWriteMDL()函数释放
///////////////////////////////////////////////////////////////////////////////////
PVOID
CreateWriteMDL(IN	ULONG	MemAddress,
			   IN	ULONG	MdlLength,
			   IN	OUT	PMDL	*pMdlPointer
			   )
{
	PMDL	pmdlPointer=NULL;
	PVOID	*MappedPointer=NULL;

	KdPrint(("[CreateWriteMDL] 申请MDL\n"));

	pmdlPointer=MmCreateMdl(NULL,(PVOID)MemAddress,MdlLength);
	if(!pmdlPointer)
	{
		KdPrint(("[CreateWriteMDL] 创建MDL块失败!\n"));
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
//	功能实现：释放CreateWriteMDL创建的MDL块
//	输入参数：pMappedPointer为MDL块定义的内存块的指针
//			  pMdlPointer为创建的MDL块的指针
//	输出参数：返回MDL指向的内存给的指针
//
///////////////////////////////////////////////////////////////////////////////////
VOID
FreeWriteMDL(IN	PMDL	pMdlPointer,IN	PVOID	pMappedPointer)
{
	KdPrint(("[FreeWriteMDL] 释放MDL块!\n"));

	MmUnmapLockedPages(pMappedPointer,pMdlPointer);
	IoFreeMdl(pMdlPointer);
}