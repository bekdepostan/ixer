//////////////////////////////////////////////////////////////////////////
// Net.c

#include "precomp.h"
#include <tdiinfo.h>
#include "common.h"
#include "Net.h"


///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：根据设备名获取文件句柄或文件对象指针
//	输入参数：FileHandle是要输出的文件句柄指针;
//			  FileObject是要输出的文件对象指针
//			  DeviceName是要获取设备的设备名
//	输出参数：返回NTSTATUS类型的值
//
///////////////////////////////////////////////////////////////////////////////////
NTSTATUS	
GetObjectByName( OUT HANDLE	*FileHandle, OUT PFILE_OBJECT	*FileObject, IN WCHAR	*DeviceName )
{
	UNICODE_STRING		deviceTCPUnicodeString;
	OBJECT_ATTRIBUTES	TCP_object_attr;
	NTSTATUS			status;
	IO_STATUS_BLOCK		IoStatus;

	ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);
	RtlInitUnicodeString(&deviceTCPUnicodeString,DeviceName);

	InitializeObjectAttributes( &TCP_object_attr,
								&deviceTCPUnicodeString,
								OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
								0,
								0 );

	status=ZwCreateFile(FileHandle,
						GENERIC_READ|GENERIC_WRITE|SYNCHRONIZE,
						&TCP_object_attr,
						&IoStatus,
						0,
						FILE_ATTRIBUTE_NORMAL,
						FILE_SHARE_READ,
						FILE_OPEN,
						0,
						0,
						0);
	if(!NT_SUCCESS(status))
	{
		KdPrint(("Failed to open"));
		return STATUS_UNSUCCESSFUL;
	}

	status=ObReferenceObjectByHandle( *FileHandle,
										FILE_ANY_ACCESS,
										0,
										KernelMode,
										(PVOID*)FileObject,
										NULL );

	return status;
}

///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：枚举网络连接端口信息
//	输入参数：OutLength为输出缓冲区的大小
//			  PortType为要枚举的端口类型
//				TCPPORT-TCP端口
//				UDPPORT-UDP端口
//	输出参数：返回NTSTATUS类型的值
//
///////////////////////////////////////////////////////////////////////////////////
PVOID		
EnumPortInformation(OUT PULONG	OutLength,IN USHORT	PortType)
{
	ULONG	BufLen=PAGE_SIZE;
	PVOID	pInputBuff=NULL;
	PVOID	pOutputBuff=NULL;
	PVOID	pOutBuf=NULL;
	NTSTATUS status = STATUS_SUCCESS;
	HANDLE FileHandle=NULL;
	UNICODE_STRING	DeviceName;
	PFILE_OBJECT pFileObject=NULL;
	PDEVICE_OBJECT pDeviceObject=NULL;
	KEVENT	Event ;
	IO_STATUS_BLOCK StatusBlock;
	PIRP		pIrp;
	PIO_STACK_LOCATION StackLocation ;
	ULONG		NumOutputBuffers;
	ULONG		i;
	TCP_REQUEST_QUERY_INFORMATION_EX		TdiId;

	RtlZeroMemory(&TdiId,sizeof(TCP_REQUEST_QUERY_INFORMATION_EX));

	if( TCPPORT == PortType )
	{
		TdiId.ID.toi_entity.tei_entity = CO_TL_ENTITY;
	}

	if( UDPPORT == PortType )
	{
		TdiId.ID.toi_entity.tei_entity = CL_TL_ENTITY;
	}

	TdiId.ID.toi_entity.tei_instance = ENTITY_LIST_ID;
	TdiId.ID.toi_class = INFO_CLASS_PROTOCOL;
	TdiId.ID.toi_type = INFO_TYPE_PROVIDER;
	TdiId.ID.toi_id = 0x102;

	pInputBuff = (PVOID)&TdiId;

	__try
	{
		if( UDPPORT == PortType )
		{
			BufLen *= 3;
		}

		pOutputBuff = ExAllocatePool( NonPagedPool, BufLen );
		if( NULL == pOutputBuff )
		{
			KdPrint(("输出缓冲区内存分配失败！\n"));

			*OutLength=0;
			__leave;
		}

		if ( TCPPORT == PortType )
		{
			status = GetObjectByName( &FileHandle, &pFileObject, L"\\Device\\Tcp" );
		}

		if ( UDPPORT == PortType )
		{
			status = GetObjectByName( &FileHandle, &pFileObject, L"\\Device\\Udp" );
		}

		if ( !NT_SUCCESS(status) )
		{
			KdPrint(("获取设备名失败！\n"));

			*OutLength=0;
			__leave;
		}

		pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

		if ( NULL == pDeviceObject )
		{
			KdPrint(("获取设备对象失败！\n"));

			*OutLength=0;
			__leave;
		}

		KdPrint(("Tcpip Driver Object:%08lX\n", pDeviceObject->DriverObject));
		KeInitializeEvent(&Event, 0, FALSE);

		pIrp = IoBuildDeviceIoControlRequest( IOCTL_TCP_QUERY_INFORMATION_EX,
												pDeviceObject, pInputBuff, sizeof(TCP_REQUEST_QUERY_INFORMATION_EX),
													pOutputBuff, BufLen, FALSE, &Event, &StatusBlock );
		if (NULL == pIrp)
		{
			KdPrint(("IRP生成失败！\n"));

			*OutLength=0;
			__leave;
		}

		StackLocation = IoGetNextIrpStackLocation(pIrp);
		StackLocation->FileObject = pFileObject;	//不设置这里会蓝屏
		StackLocation->DeviceObject = pDeviceObject;

		status  = IoCallDriver(pDeviceObject, pIrp);

		KdPrint(("STATUS:%08lX\n", status));

		if ( STATUS_BUFFER_OVERFLOW == status )
		{
			KdPrint(("缓冲区太小！%d\n",StatusBlock.Information));
		}

		if (STATUS_PENDING == status)
		{
			KdPrint(("STATUS_PENDING"));

			status = KeWaitForSingleObject( &Event, 0, 0, 0, 0 );
		}

		if( STATUS_CANCELLED == status )
		{
			KdPrint(("STATUS_CANCELLED"));
		}

		if( status == STATUS_SUCCESS )
		{
			*OutLength=StatusBlock.Information;
			pOutBuf=pOutputBuff;
		}
	}
	__finally
	{
		IxObDereferenceObject(pFileObject); 
		if(FileHandle)
		{
			ZwClose(FileHandle);
		}
	}

	return pOutBuf;
}