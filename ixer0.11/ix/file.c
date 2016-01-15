/*===================================================================
 * Filename file.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 文件
 *
 * Date:   2013-5-21 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "..\Common\Surface.h"
#include "common.h"
#include "proc.h"
#include "file.h"

/*========================================================================
*
* 函数名:	QueryDirectoryFileInfo
*
* 参数:		[IN]	pDirName	-指定的目录
*			[OUT]	dwRetSize	-dwRetSize为列表信息缓冲区的长度
*
* 功能描述:	获取指定目录下的文件列表
*
* 返回值:	指向文件列表信息缓冲区的指针
*
=========================================================================*/
PVOID	QueryDirectoryFileInfo( PCHAR pDirName, PULONG dwRetSize )
{
	NTSTATUS			status;
	ULONG				dwBytesReted;
	OBJECT_ATTRIBUTES	objAttr; 
	PDEVICE_OBJECT		pDevObject; 
	KEVENT				kEvent;
	PIO_STACK_LOCATION	irpSp;
	IO_STATUS_BLOCK		ioStus; 
	HANDLE				hFile; 
	PVOID				pSystemBuffer;
	PDIRECTORY_INFO		pDirInfo; 
	PFILE_OBJECT		pFileObject; 
	UNICODE_STRING		unFileName; 
	UNICODE_STRING		unico; 
	ANSI_STRING			ansFileName;  
	PUCHAR				pNext;
	CHAR				buffer[1024];
	PIRP				irp = NULL;
	PFILE_DIRECTORY_INFORMATION pInformation; 
	PFILE_DIRECTORY_INFORMATION pRealInformation; 

	status = STATUS_UNSUCCESSFUL;
	dwBytesReted = 0;
	RtlZeroMemory( buffer, 1024 );

	//得到文件夹路径的DosName
	strcpy( buffer, "\\DosDevices\\" ); 
	strcat( buffer, pDirName ); 
	RtlInitAnsiString( &ansFileName, buffer ); 
	RtlAnsiStringToUnicodeString( &unFileName, &ansFileName, TRUE );

	//初始化对象属性
	InitializeObjectAttributes( &objAttr, &unFileName, OBJ_CASE_INSENSITIVE+OBJ_KERNEL_HANDLE, NULL, NULL );

	//首先打开这个文件 得到这个文件的句柄 hFile
	status = ZwOpenFile( &hFile,
						FILE_LIST_DIRECTORY + SYNCHRONIZE+FILE_ANY_ACCESS,
						&objAttr,
						&ioStus,
						FILE_SHARE_READ + FILE_SHARE_WRITE + FILE_SHARE_DELETE,
						FILE_DIRECTORY_FILE + FILE_SYNCHRONOUS_IO_NONALERT); 
	if(!NT_SUCCESS(status))  
	{
		goto endcddir;  
	}

	//由文件句柄找到文件对象
	status = ObReferenceObjectByHandle( hFile,
										FILE_LIST_DIRECTORY + SYNCHRONIZE,
										0,
										KernelMode,
										&pFileObject,
										NULL ); 
	if(!NT_SUCCESS(status)) 
	{ 
		ZwClose(hFile); 
		goto endcddir; 
	}

	//获得这个文件对象的相对设备对象
	pDevObject = IxIoGetRelatedDeviceObject( pFileObject );

	//分配一个新的IRP结构体 构建查询
	irp = IoAllocateIrp( pDevObject->StackSize, FALSE ); 
	if( !irp ) 
	{ 
//		DbgPrint("QueryDirectoryFileInfo(): allocate irp failed!\n");

		ObDereferenceObject(pFileObject);
		ZwClose(hFile);
		goto endcddir;
	}

	//初始化一个同步事件
	KeInitializeEvent( &kEvent, SynchronizationEvent, FALSE);

	//分配内存 文件信息 和 查询结果
	pInformation = ExAllocatePool( PagedPool, 655350 ); 
	pSystemBuffer = ExAllocatePool( PagedPool, 655350 );

	//内存清零
	RtlZeroMemory( pSystemBuffer, 655350 ); 
	RtlZeroMemory( pInformation, 655350 );

	/* 构造 IRP */
	irp->UserEvent = &kEvent; 
	irp->UserBuffer = pInformation; 
	irp->AssociatedIrp.SystemBuffer = pInformation; 
	irp->MdlAddress = NULL; 
	irp->Flags = 0; 
	irp->UserIosb = &ioStus; 
	irp->Tail.Overlay.OriginalFileObject = pFileObject; //原始文件对象
	irp->Tail.Overlay.Thread = PsGetCurrentThread(); 
	irp->RequestorMode = KernelMode; 
	irpSp = IoGetNextIrpStackLocation(irp);				//IRP栈空间
	irpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;	//MajorFunction
	irpSp->MinorFunction = IRP_MN_QUERY_DIRECTORY;		//MinorFunction
	irpSp->FileObject = pFileObject;
	irpSp->DeviceObject = pDevObject;
	irpSp->Flags = SL_RESTART_SCAN; 
	irpSp->Control = 0; 
	irpSp->Parameters.QueryDirectory.FileIndex = 0; 
	irpSp->Parameters.QueryDirectory.FileInformationClass = FileDirectoryInformation;
	irpSp->Parameters.QueryDirectory.FileName = NULL;
	irpSp->Parameters.QueryDirectory.Length = 655350;

	//完成例程
	IoSetCompletionRoutine( irp, EventCompletion, 0, TRUE, TRUE, TRUE);

	//调用驱动
	status = IxIoCallDriver( pDevObject, irp );

	//事件完成 完成例程被调用
	KeWaitForSingleObject( &kEvent, Executive, KernelMode, TRUE, NULL );

	//得到了文件列表信息 pInformation
	pRealInformation = pInformation; 
	//头指针保存在pSystemBuffer
	pDirInfo = (PDIRECTORY_INFO)pSystemBuffer;

	//开始遍历文件列表信息
	while ( 1 )
	{
		//文件名长度
		unico.Length = (USHORT)pInformation->FileNameLength;
		unico.MaximumLength = (USHORT)pInformation->FileNameLength;

		//Unicode文件名
		unico.Buffer = &(pInformation->FileName[0]);

		//把Unicode字符串转化为Ansi字符串
		RtlUnicodeStringToAnsiString( &ansFileName, &unico, TRUE ); 

		//文件名
		strcpy( pDirInfo->FileName, ansFileName.Buffer);
		RtlFreeAnsiString(&ansFileName);

		//文件大小
		pDirInfo->AllocationSize=pInformation->AllocationSize;
		//文件属性
		pDirInfo->FileAttributes=pInformation->FileAttributes;
		//创建时间
		RtlTimeToTimeFields( &(pInformation->CreationTime), &(pDirInfo->CreationTime));
		//最后访问时间
		RtlTimeToTimeFields(&(pInformation->LastAccessTime),&(pDirInfo->LastAccessTime));
		//最后写入时间
		RtlTimeToTimeFields(&(pInformation->LastWriteTime),&(pDirInfo->LastWriteTime));
		//修改时间 
		RtlTimeToTimeFields(&(pInformation->ChangeTime),&(pDirInfo->ChangeTime));

		//累计需要的缓冲区字节数
		dwBytesReted += sizeof(DIRECTORY_INFO);

		//遍历结束，则退出
		if( !pInformation->NextEntryOffset ) 
			goto exit;

		//下一个文件信息
		pNext = (PUCHAR)pInformation;
		pNext += pInformation->NextEntryOffset;
		pInformation = (PFILE_DIRECTORY_INFORMATION)(pNext);
		//输出缓冲区的下一个指针
		pDirInfo++; 
	}

endcddir:
	RtlFreeUnicodeString(&unFileName);
	return NULL;

exit:
	ExFreePool( pRealInformation );
	ObDereferenceObject( pFileObject );
	ZwClose( hFile );
	RtlFreeUnicodeString( &unFileName );

	*dwRetSize = dwBytesReted;

	return pSystemBuffer;
}

/*========================================================================
*
* 函数名:	EventCompletion
*
* 参数:		[IN] pDirName	-参见DDK中完成例程的声明说明
*
* 功能描述:	文件操作的完成例程
*
* 返回值:	返回NTSTATUS信息，参见DDK中的说明
*
=========================================================================*/
NTSTATUS 
EventCompletion( 
				IN PDEVICE_OBJECT DeviceObject, 
				IN PIRP Irp, 
				IN PVOID Context 
				) 
{
	Irp->UserIosb->Status = Irp->IoStatus.Status;
	Irp->UserIosb->Information  = Irp->IoStatus.Information;

	KeSetEvent( Irp->UserEvent, 0, FALSE);
	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
} 

/*========================================================================
*
* 函数名:	IxIoGetRelatedDeviceObject
*
* 参数:		[IN] FileObject	-参见DDK中IoGetRelatedDeviceObject的声明说明
*
* 功能描述:	自定义的获取相对文件系统设备指针的函数
*
* 返回值:	返回文件设备的对象指针
*
=========================================================================*/
PDEVICE_OBJECT
IxIoGetRelatedDeviceObject( IN PFILE_OBJECT FileObject )
{
	PDEVICE_OBJECT devObject;

	if ( FileObject->Vpb != NULL &&
		 FileObject->Vpb->DeviceObject != NULL && 
		( !(FileObject->Flags & FO_DIRECT_DEVICE_OPEN) ) )	/* 非直接打开 */
	{
		devObject = FileObject->Vpb->DeviceObject;
	} else if ( !(FileObject->Flags & FO_DIRECT_DEVICE_OPEN) &&
						FileObject->DeviceObject->Vpb != NULL &&
							FileObject->DeviceObject->Vpb->DeviceObject != NULL )
	{
			devObject = FileObject->DeviceObject->Vpb->DeviceObject;
	} else
		 {
				devObject = FileObject->DeviceObject;
		 }
	if ( devObject->AttachedDevice != NULL )
	{
		devObject = IxIoGetAttachedDevice( devObject );
	}

	return devObject;
}

/*========================================================================
*
* 函数名:	IoGetAttachedDevice
*
* 参数:		PDEVICE_OBJECT		[IN]	DeviceObject
*
* 功能描述:	来自ReactOS
*
* 返回值:	PDEVICE_OBJECT
*
=========================================================================*/
PDEVICE_OBJECT
__stdcall
IxIoGetAttachedDevice( PDEVICE_OBJECT DeviceObject )
{
	PDEVICE_OBJECT Current = DeviceObject;

	/* Get the last attached device */
	while (Current->AttachedDevice)
	{
		Current = Current->AttachedDevice;
	}

	/* Return it */
	return Current;
}

/*========================================================================
*
* 函数名:	IxDeleteFile
*
* 参数:		LPCWSTR				[IN]	FileFullPath
*			BOOLEAN				[IN]	bForce
*
* 功能描述:	删除文件
*
* 返回值:	NTSTATUS
*
=========================================================================*/
NTSTATUS
IxDeleteFile( LPCWSTR FileFullPath )
{
	NTSTATUS			Status;
	OBJECT_ATTRIBUTES	ObjectAttributes;
	HANDLE				FileHandle;
	IO_STATUS_BLOCK		Iosb;
	BOOLEAN				bRestartScan;

	Status = STATUS_SUCCESS;
	bRestartScan = TRUE;

	if ( !IsValidAddr((ULONG)FileFullPath) )
	{
		Status = STATUS_INVALID_PARAMETER;

#ifdef _CHECKED
		DbgPrint( "$ixer:\tDeleteFile:\tSTATUS_INVALID_PARAMETER" );
#endif

		return Status;
	}

	Status = IxCreateFile( &FileHandle, FileFullPath, FILE_READ_ATTRIBUTES, FILE_SHARE_DELETE );
	if (!NT_SUCCESS(Status))
	{
#ifdef _CHECKED
		DbgPrint( "$ixer:\tZwOpenFile:0x%08x", Status );
#endif

		return Status;
	}

	/* 设置文件属性为一般 */
	Status = StripFileAttributes( FileHandle );
	if (!NT_SUCCESS(Status))
	{
		ZwClose(FileHandle);

		return Status;
	}

	/* 删除文件 */
	Status = DeleteFileByHandle( FileHandle );

	ZwClose(FileHandle);

#ifdef _CHECKED
	DbgPrint( "ixer:DeleteFile() end:0x%08x", Status );
#endif

	return Status;
}

/*========================================================================
*
* 函数名:	DeleteFileByHandle
*
* 参数:		HANDLE				[IN]	FileHandle
*
* 功能描述:	删除文件
*
* 返回值:	NTSTATUS
*
=========================================================================*/
NTSTATUS
DeleteFileByHandle( HANDLE FileHandle )
{
	NTSTATUS						Status;
	PFILE_OBJECT					FileObject;
	PDEVICE_OBJECT					DeviceObject;
	PIRP							irp;
	PIO_STACK_LOCATION				irpSp;
	FILE_DISPOSITION_INFORMATION	FileInformation;
	KEVENT							kEvent;
	IO_STATUS_BLOCK					ioStatus;
	KIRQL							kirql;
	PSECTION_OBJECT_POINTERS		pbakSectionObj;

	Status = STATUS_SUCCESS;

	if ( FileHandle == NULL )
	{
		Status = STATUS_INVALID_PARAMETER;

#ifdef _CHECKED
		DbgPrint( "$ixer:\tDeleteFileByHandle:\tSTATUS_INVALID_PARAMETER" );
#endif

		return Status;
	}

	Status = ObReferenceObjectByHandle(
										FileHandle,
										DELETE,
										*IoFileObjectType,
										KernelMode,
										(PVOID *) &FileObject,
										NULL );
	if (!NT_SUCCESS( Status ))
	{
#ifdef _CHECKED
		DbgPrint( "$ixer:\tObReferenceObjectByHandle:\t0x%08x", Status );
#endif

		return Status;
	}

#ifdef _CHECKED
	DbgPrint( "ixer:\tFileObject:\t0x%08x", FileObject );
#endif

	DeviceObject=IxIoGetRelatedDeviceObject( FileObject );

#ifdef _CHECKED
	DbgPrint( "$ixer:DeviceObject:\t0x%08x", DeviceObject );
#endif

	/* 取最底层的设备对象 */
	while ( (*(PULONG)((ULONG)(DeviceObject->DeviceObjectExtension) + 0x18)) != 0 )
	{
		DeviceObject = (PDEVICE_OBJECT)( *(PULONG)( (ULONG)(DeviceObject->DeviceObjectExtension) + 0x18 ));
	}

#ifdef _CHECKED
	DbgPrint( "ixer:Base deviceObject:\t0x%08x", DeviceObject );
#endif

	/* 分配 IRP */
	if ( (irp = IoAllocateIrp(DeviceObject->StackSize, FALSE)) == NULL )
	{
#ifdef _CHECKED
		DbgPrint( "$ixer:IoAllocateIrp failed!\n" );
#endif

		ObDereferenceObject(FileObject);
		Status = STATUS_INSUFFICIENT_RESOURCES;
		return Status;
	}

	KeInitializeEvent(&kEvent, SynchronizationEvent, FALSE);

	memset( &FileInformation, 0, sizeof(FILE_DISPOSITION_INFORMATION) );

	FileInformation.DeleteFile = TRUE;

	irp->AssociatedIrp.SystemBuffer = &FileInformation;
	irp->UserEvent = &kEvent;
	irp->UserIosb = &ioStatus;
	irp->Tail.Overlay.OriginalFileObject = FileObject;
	irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
	irp->RequestorMode = KernelMode;

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
	irpSp->DeviceObject = DeviceObject;
	irpSp->FileObject = FileObject;
	irpSp->Parameters.SetFile.Length = sizeof(FILE_DISPOSITION_INFORMATION);
	irpSp->Parameters.SetFile.FileInformationClass = FileDispositionInformation;
	irpSp->Parameters.SetFile.FileObject = FileObject;

	IoSetCompletionRoutine(
							irp,
							SetFileCompletion,
							&kEvent,
							TRUE,
							TRUE,
							TRUE );

	/*  send Irp packet */
	Status = IxIoCallDriver( DeviceObject, irp );
	if ( Status == STATUS_PENDING )
	{
		Status = KeWaitForSingleObject( &kEvent, Executive, KernelMode, TRUE, NULL );
	}

	ObDereferenceObject(FileObject);

#ifdef _CHECKED
	DbgPrint( "ixer:\tDeleteFileByHandle() end:\t0x%08x", Status );
#endif

	return Status;
}

/*========================================================================
*
* 函数名:	StripFileAttributes
*
* 参数:		HANDLE				[IN]	FileHandle
*
* 功能描述:	调整文件属性
*
* 返回值:	NTSTATUS
*
=========================================================================*/
NTSTATUS
StripFileAttributes( HANDLE FileHandle )
{
	NTSTATUS				Status;
	PFILE_OBJECT			FileObject;
	PDEVICE_OBJECT			DeviceObject;
	PIRP					irp;
	PIO_STACK_LOCATION		irpSp;
	FILE_BASIC_INFORMATION	FileInformation;
	KEVENT					kEvent;
	IO_STATUS_BLOCK			ioStatus;

	Status = STATUS_SUCCESS;

#ifdef _CHECKED
	DbgPrint( "ixer:\tStripFileAttributes( 0x%08x )", FileHandle );
#endif

	if ( FileHandle == NULL )
	{
		Status = STATUS_INVALID_PARAMETER;

#ifdef _CHECKED
		DbgPrint( "$ixer:\tStripFileAttributes:\tSTATUS_INVALID_PARAMETER" );
#endif

		return Status;
	}

	Status = ObReferenceObjectByHandle(	FileHandle,
										DELETE,
										*IoFileObjectType,
										KernelMode,
										(PVOID *) &FileObject,
										NULL );
	if (!NT_SUCCESS( Status ))
	{
#ifdef _CHECKED
		DbgPrint( "$ixer:\tObReferenceObjectByHandle:\t0x%08x", Status );
#endif

		return Status;
	}

#ifdef _CHECKED
	DbgPrint( "$ixer:\tFileObject:\t0x%08x", FileObject );
#endif

	DeviceObject=IxIoGetRelatedDeviceObject( FileObject );
/*
	/* 是否直接打开 /
	if (FileObject->Flags & FO_DIRECT_DEVICE_OPEN)
	{
#ifdef _CHECKED
		DbgPrint( "$ixer:\tFO_DIRECT_DEVICE_OPEN" );
#endif

		DeviceObject = IxIoGetAttachedDevice(FileObject->DeviceObject);
	}
	else
	{
		DeviceObject = IoGetRelatedDeviceObject(FileObject);
	}
*/
#ifdef _CHECKED
	DbgPrint( "$ixer:\tDeviceObject:\t0x%08x", DeviceObject );
#endif

	/* 取最底层的设备对象 */
	while ( (*(PULONG)((ULONG)(DeviceObject->DeviceObjectExtension) + 0x18)) != 0 )
	{
		DeviceObject = (PDEVICE_OBJECT)(*(PULONG)((ULONG)(DeviceObject->DeviceObjectExtension) + 0x18));
	}

#ifdef _CHECKED
	DbgPrint( "$ixer:\tDeviceObject:\t0x%08x", DeviceObject );
#endif

	/* 分配 IRP */
	if ( (irp = IoAllocateIrp(DeviceObject->StackSize, FALSE)) == NULL )
	{
#ifdef _CHECKED
		DbgPrint( "$ixer:\tIoAllocateIrp" );
#endif

		ObDereferenceObject(FileObject);
		Status = STATUS_INSUFFICIENT_RESOURCES;
		return Status;
	}

	KeInitializeEvent( &kEvent, SynchronizationEvent, FALSE );

	memset( &FileInformation, 0, sizeof(FILE_BASIC_INFORMATION) );

	//修改文件属性
	FileInformation.FileAttributes = FILE_ATTRIBUTE_NORMAL;

	//填充构造irp包
	irp->AssociatedIrp.SystemBuffer = &FileInformation;
	irp->UserEvent = &kEvent;
	irp->UserIosb = &ioStatus;
	irp->Tail.Overlay.OriginalFileObject = FileObject;
	irp->Tail.Overlay.Thread = (PETHREAD)KeGetCurrentThread();
	irp->RequestorMode = KernelMode;

	irpSp = IoGetNextIrpStackLocation(irp);
	irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
	irpSp->DeviceObject = DeviceObject;
	irpSp->FileObject = FileObject;
	irpSp->Parameters.SetFile.Length = sizeof(FILE_BASIC_INFORMATION);
	irpSp->Parameters.SetFile.FileInformationClass = FileBasicInformation;
	irpSp->Parameters.SetFile.FileObject = FileObject;

	IoSetCompletionRoutine(	irp,
							SetFileCompletion,
							&kEvent,
							TRUE,
							TRUE,
							TRUE );

	/* 调用驱动 */
	Status = IxIoCallDriver( DeviceObject, irp );
	if ( Status == STATUS_PENDING )
	{
		Status = KeWaitForSingleObject(&kEvent, Executive, KernelMode, TRUE, NULL);
	}

	ObDereferenceObject(FileObject);

#ifdef _CHECKED
	DbgPrint( "$ixer:\tStripFileAttributes() end:\t0x%08x", Status );
#endif

	return Status;
}

NTSTATUS
SetFileCompletion(
				  IN PDEVICE_OBJECT DeviceObject,
				  IN PIRP Irp,
				  IN PVOID Context
				  )
{
	Irp->UserIosb->Status = Irp->IoStatus.Status;
	Irp->UserIosb->Information = Irp->IoStatus.Information;

	KeSetEvent(Irp->UserEvent, IO_NO_INCREMENT, FALSE);
	IoFreeIrp(Irp);

	return STATUS_MORE_PROCESSING_REQUIRED;
}