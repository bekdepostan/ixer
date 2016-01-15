/*===================================================================
 * Filename file.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: �ļ�
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
* ������:	QueryDirectoryFileInfo
*
* ����:		[IN]	pDirName	-ָ����Ŀ¼
*			[OUT]	dwRetSize	-dwRetSizeΪ�б���Ϣ�������ĳ���
*
* ��������:	��ȡָ��Ŀ¼�µ��ļ��б�
*
* ����ֵ:	ָ���ļ��б���Ϣ��������ָ��
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

	//�õ��ļ���·����DosName
	strcpy( buffer, "\\DosDevices\\" ); 
	strcat( buffer, pDirName ); 
	RtlInitAnsiString( &ansFileName, buffer ); 
	RtlAnsiStringToUnicodeString( &unFileName, &ansFileName, TRUE );

	//��ʼ����������
	InitializeObjectAttributes( &objAttr, &unFileName, OBJ_CASE_INSENSITIVE+OBJ_KERNEL_HANDLE, NULL, NULL );

	//���ȴ�����ļ� �õ�����ļ��ľ�� hFile
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

	//���ļ�����ҵ��ļ�����
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

	//�������ļ����������豸����
	pDevObject = IxIoGetRelatedDeviceObject( pFileObject );

	//����һ���µ�IRP�ṹ�� ������ѯ
	irp = IoAllocateIrp( pDevObject->StackSize, FALSE ); 
	if( !irp ) 
	{ 
//		DbgPrint("QueryDirectoryFileInfo(): allocate irp failed!\n");

		ObDereferenceObject(pFileObject);
		ZwClose(hFile);
		goto endcddir;
	}

	//��ʼ��һ��ͬ���¼�
	KeInitializeEvent( &kEvent, SynchronizationEvent, FALSE);

	//�����ڴ� �ļ���Ϣ �� ��ѯ���
	pInformation = ExAllocatePool( PagedPool, 655350 ); 
	pSystemBuffer = ExAllocatePool( PagedPool, 655350 );

	//�ڴ�����
	RtlZeroMemory( pSystemBuffer, 655350 ); 
	RtlZeroMemory( pInformation, 655350 );

	/* ���� IRP */
	irp->UserEvent = &kEvent; 
	irp->UserBuffer = pInformation; 
	irp->AssociatedIrp.SystemBuffer = pInformation; 
	irp->MdlAddress = NULL; 
	irp->Flags = 0; 
	irp->UserIosb = &ioStus; 
	irp->Tail.Overlay.OriginalFileObject = pFileObject; //ԭʼ�ļ�����
	irp->Tail.Overlay.Thread = PsGetCurrentThread(); 
	irp->RequestorMode = KernelMode; 
	irpSp = IoGetNextIrpStackLocation(irp);				//IRPջ�ռ�
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

	//�������
	IoSetCompletionRoutine( irp, EventCompletion, 0, TRUE, TRUE, TRUE);

	//��������
	status = IxIoCallDriver( pDevObject, irp );

	//�¼���� ������̱�����
	KeWaitForSingleObject( &kEvent, Executive, KernelMode, TRUE, NULL );

	//�õ����ļ��б���Ϣ pInformation
	pRealInformation = pInformation; 
	//ͷָ�뱣����pSystemBuffer
	pDirInfo = (PDIRECTORY_INFO)pSystemBuffer;

	//��ʼ�����ļ��б���Ϣ
	while ( 1 )
	{
		//�ļ�������
		unico.Length = (USHORT)pInformation->FileNameLength;
		unico.MaximumLength = (USHORT)pInformation->FileNameLength;

		//Unicode�ļ���
		unico.Buffer = &(pInformation->FileName[0]);

		//��Unicode�ַ���ת��ΪAnsi�ַ���
		RtlUnicodeStringToAnsiString( &ansFileName, &unico, TRUE ); 

		//�ļ���
		strcpy( pDirInfo->FileName, ansFileName.Buffer);
		RtlFreeAnsiString(&ansFileName);

		//�ļ���С
		pDirInfo->AllocationSize=pInformation->AllocationSize;
		//�ļ�����
		pDirInfo->FileAttributes=pInformation->FileAttributes;
		//����ʱ��
		RtlTimeToTimeFields( &(pInformation->CreationTime), &(pDirInfo->CreationTime));
		//������ʱ��
		RtlTimeToTimeFields(&(pInformation->LastAccessTime),&(pDirInfo->LastAccessTime));
		//���д��ʱ��
		RtlTimeToTimeFields(&(pInformation->LastWriteTime),&(pDirInfo->LastWriteTime));
		//�޸�ʱ�� 
		RtlTimeToTimeFields(&(pInformation->ChangeTime),&(pDirInfo->ChangeTime));

		//�ۼ���Ҫ�Ļ������ֽ���
		dwBytesReted += sizeof(DIRECTORY_INFO);

		//�������������˳�
		if( !pInformation->NextEntryOffset ) 
			goto exit;

		//��һ���ļ���Ϣ
		pNext = (PUCHAR)pInformation;
		pNext += pInformation->NextEntryOffset;
		pInformation = (PFILE_DIRECTORY_INFORMATION)(pNext);
		//�������������һ��ָ��
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
* ������:	EventCompletion
*
* ����:		[IN] pDirName	-�μ�DDK��������̵�����˵��
*
* ��������:	�ļ��������������
*
* ����ֵ:	����NTSTATUS��Ϣ���μ�DDK�е�˵��
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
* ������:	IxIoGetRelatedDeviceObject
*
* ����:		[IN] FileObject	-�μ�DDK��IoGetRelatedDeviceObject������˵��
*
* ��������:	�Զ���Ļ�ȡ����ļ�ϵͳ�豸ָ��ĺ���
*
* ����ֵ:	�����ļ��豸�Ķ���ָ��
*
=========================================================================*/
PDEVICE_OBJECT
IxIoGetRelatedDeviceObject( IN PFILE_OBJECT FileObject )
{
	PDEVICE_OBJECT devObject;

	if ( FileObject->Vpb != NULL &&
		 FileObject->Vpb->DeviceObject != NULL && 
		( !(FileObject->Flags & FO_DIRECT_DEVICE_OPEN) ) )	/* ��ֱ�Ӵ� */
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
* ������:	IoGetAttachedDevice
*
* ����:		PDEVICE_OBJECT		[IN]	DeviceObject
*
* ��������:	����ReactOS
*
* ����ֵ:	PDEVICE_OBJECT
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
* ������:	IxDeleteFile
*
* ����:		LPCWSTR				[IN]	FileFullPath
*			BOOLEAN				[IN]	bForce
*
* ��������:	ɾ���ļ�
*
* ����ֵ:	NTSTATUS
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

	/* �����ļ�����Ϊһ�� */
	Status = StripFileAttributes( FileHandle );
	if (!NT_SUCCESS(Status))
	{
		ZwClose(FileHandle);

		return Status;
	}

	/* ɾ���ļ� */
	Status = DeleteFileByHandle( FileHandle );

	ZwClose(FileHandle);

#ifdef _CHECKED
	DbgPrint( "ixer:DeleteFile() end:0x%08x", Status );
#endif

	return Status;
}

/*========================================================================
*
* ������:	DeleteFileByHandle
*
* ����:		HANDLE				[IN]	FileHandle
*
* ��������:	ɾ���ļ�
*
* ����ֵ:	NTSTATUS
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

	/* ȡ��ײ���豸���� */
	while ( (*(PULONG)((ULONG)(DeviceObject->DeviceObjectExtension) + 0x18)) != 0 )
	{
		DeviceObject = (PDEVICE_OBJECT)( *(PULONG)( (ULONG)(DeviceObject->DeviceObjectExtension) + 0x18 ));
	}

#ifdef _CHECKED
	DbgPrint( "ixer:Base deviceObject:\t0x%08x", DeviceObject );
#endif

	/* ���� IRP */
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
* ������:	StripFileAttributes
*
* ����:		HANDLE				[IN]	FileHandle
*
* ��������:	�����ļ�����
*
* ����ֵ:	NTSTATUS
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
	/* �Ƿ�ֱ�Ӵ� /
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

	/* ȡ��ײ���豸���� */
	while ( (*(PULONG)((ULONG)(DeviceObject->DeviceObjectExtension) + 0x18)) != 0 )
	{
		DeviceObject = (PDEVICE_OBJECT)(*(PULONG)((ULONG)(DeviceObject->DeviceObjectExtension) + 0x18));
	}

#ifdef _CHECKED
	DbgPrint( "$ixer:\tDeviceObject:\t0x%08x", DeviceObject );
#endif

	/* ���� IRP */
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

	//�޸��ļ�����
	FileInformation.FileAttributes = FILE_ATTRIBUTE_NORMAL;

	//��乹��irp��
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

	/* �������� */
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