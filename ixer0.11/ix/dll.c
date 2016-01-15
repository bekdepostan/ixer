//////////////////////////////////////////////////////////////////////////
// Dll.c

#include "precomp.h"
#include "ssdt.h"
#include "proc.h"
#include "WinOSVersion.h"
#include "func.h"
#include "Dll.h"



//��ȡDll�б�
PDLL_ITEM GetDllList(ULONG ulPid, PULONG numOfDll)
{
	NTSTATUS status=STATUS_SUCCESS;
	PDLL_ITEM pDllItem=NULL;
	ULONG i=0;

//	KdPrint(("����Pid:%d \t Dll����:%d\n", ulPid, *dllNum));
	//��dll���������ڴ�
	pDllItem=(DLL_ITEM *)ExAllocatePoolWithTag(PagedPool, sizeof(DLL_ITEM)*MAX_Dll_NUM*2, 'dll');
	if (pDllItem)
	{
		//�ڴ�����
		RtlZeroMemory(pDllItem, sizeof(DLL_ITEM)*MAX_Dll_NUM*2);
		//ö��Dll
		EnumDllByInLoadOrderLinks(ulPid, numOfDll, pDllItem);
		//���dll
//		status=CheckDllByZwQueryVirtualMem(ulPid, numOfDll, pDllItem);
	}

	return pDllItem;
}

BOOLEAN EnumDllByInLoadOrderLinks(ULONG ulPid, PULONG numOfDll, PDLL_ITEM pDllItem)
{
	PLDR_DATA_TABLE_ENTRY LdrEntry=NULL;
	PLIST_ENTRY	LdrHeader=NULL, LdrNext=NULL;
	NTSTATUS status=0;
	PEPROCESS	pEproc;
	ULONG		Peb, Ldr;
	KAPC_STATE	pApcStatus;

	/*��ȡĿ�����EPROCESS*/
	status=PsLookupProcessByProcessId( (HANDLE)ulPid, &pEproc);
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("Can't get target process eprocess! ErrorCode= %08X\n", status));
		return FALSE;
	}

	//EPROCESSΪ0����Ч�򷵻�
	if (pEproc == 0  ||  IsValidAddr( ( (ULONG)pEproc+g_EPROCESS_Peb )) != VALID_PAGE )
	{
		KdPrint(("Target process eprocess  =0 or invalid!\n"));
		return FALSE;
	}

	/*ջ����(�л���Ŀ�����������)*/
		KeStackAttachProcess(pEproc, &pApcStatus);

	Peb = *(PULONG)( (ULONG)pEproc+g_EPROCESS_Peb );
	__try
	{
		//Peb���û��ռ䣬���ܻ��޷�����
		ProbeForRead( (PVOID)Peb, 4, 1);
		//��̽����
		ProbeForRead( (PVOID)(Peb+g_PEB_Ldr), 4, 1);
		if (0 != Peb  &&  *(PULONG)(Peb+g_PEB_Ldr) != 0)
		{
			Ldr=*(PULONG)( Peb+g_PEB_Ldr );
			LdrHeader=(PLIST_ENTRY)( Ldr+g_LDRDATA_InLoadOrderModuleList );

			for (LdrNext=LdrHeader->Flink; LdrNext != LdrHeader; LdrNext=LdrNext->Flink)
			{
				LdrEntry= CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
				if (LdrEntry)
				{
// 					//dll����
// 					(*numOfDll)++;

					KdPrint(("StoreOrCheckDll(%08X, %ws)\n", (ULONG)LdrEntry->DllBase, LdrEntry->FullDllName.Buffer));
					//�洢ö�ٵ���dllģ����Ϣ
					StoreOrCheckDll(TRUE, (ULONG)LdrEntry->DllBase, LdrEntry->SizeOfImage, LdrEntry->FullDllName.Buffer, pDllItem, numOfDll);
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("__except!\n"));

		//����
		KeUnstackDetachProcess(&pApcStatus);
		return FALSE;
	}

	//����
	KeUnstackDetachProcess(&pApcStatus);
	return TRUE;
}

/*========================================================================
*
* ������:	ConvertDeviceName
*
* ����:		LPWSTR		[IN] FileName		- Ҫת����
*			LPWSTR		[OUT] OutFileName	- DOS�ļ�·��
*
* ��������:	������ "\Device\HarddiskVolume1\xxxx���ں��ļ�·��ת��Ϊ DOS�ļ�·��(C:\xxx\xxx..)
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS ConvertVolumeDeviceToDosPath(LPWSTR FileName , LPWSTR OutFileName)
{
	OBJECT_ATTRIBUTES	objAttributes = {0};
	UNICODE_STRING	uniName;
	HANDLE	FileHandle;
	IO_STATUS_BLOCK	ioStatusBlock;

	NTSTATUS  status;
	PFILE_OBJECT FileObject;

	UNICODE_STRING	VolumeName = {0};
	UNICODE_STRING	FanalName;
	BOOL	WillFreeVolumeName=TRUE ;

	VolumeName.Buffer = NULL;
	VolumeName.Length = 0;
	VolumeName.MaximumLength = 0;

	RtlInitUnicodeString(&uniName, FileName);
	//��ʼ����������
	InitializeObjectAttributes( &objAttributes, &uniName, OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, 0, 0 );

	//�򿪶���õ��ļ����
	status = IoCreateFile( &FileHandle,
							FILE_READ_ATTRIBUTES, 
							&objAttributes,
							&ioStatusBlock,
							0,
							FILE_ATTRIBUTE_NORMAL,
							FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
							FILE_OPEN,
							FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
							0,
							0,
							CreateFileTypeNone,
							0,
							IO_NO_PARAMETER_CHECKING );
	if ( !NT_SUCCESS(status) )
	{
		return status; 
	}

	//���ö���ͨ�����
	status=ObReferenceObjectByHandle( FileHandle, 
									  GENERIC_READ, 
									  *IoFileObjectType, 
									  KernelMode, 
									  &FileObject, 
									  NULL );
	if ( !NT_SUCCESS(status) ||
		!FileObject ||
		!FileObject->FileName.Length ||
		!FileObject->FileName.Buffer )
	{
		ZwClose(FileHandle);
		return STATUS_INVALID_PARAMETER;
	}

	//��ȡ�̷���VolumeName.Buffer ������:�ַ���"C:"��"D:"��..
	status = RtlVolumeDeviceToDosName(FileObject->DeviceObject , &VolumeName);
	if ( !NT_SUCCESS(status) )
	{
		RtlInitUnicodeString( &VolumeName , L"\\" );
		WillFreeVolumeName=FALSE;
	}

	if (FileObject->FileName.Length + VolumeName.Length >= MAX_PATH) //0x800 * 2
	{
		ObDereferenceObject(FileObject);
		ZwClose(FileHandle);

		if (WillFreeVolumeName && MmIsAddressValid(VolumeName.Buffer))
		{
			ExFreePool(VolumeName.Buffer);
		}
		return STATUS_INVALID_PARAMETER;
	}

	RtlZeroMemory((PVOID)OutFileName , MAX_PATH); //0x800 * 2
	RtlInitUnicodeString(&FanalName , OutFileName);
	FanalName.MaximumLength = MAX_PATH;			//0x800 * 2

	//"C:" += "\WINDOWS\NOTEPAD.EXE" (Assume)
	if ( !NT_SUCCESS( RtlAppendUnicodeStringToString(&FanalName, &VolumeName) ) )
	{
		ObDereferenceObject(FileObject);
		ZwClose(FileHandle);

		if (WillFreeVolumeName && MmIsAddressValid(VolumeName.Buffer))
		{
			ExFreePool(VolumeName.Buffer);
		}
		return STATUS_INVALID_PARAMETER;      
	}

	if ( !NT_SUCCESS( RtlAppendUnicodeStringToString(&FanalName, &FileObject->FileName) ) )
	{
		ObDereferenceObject(FileObject);
		ZwClose(FileHandle);

		if (WillFreeVolumeName && MmIsAddressValid(VolumeName.Buffer))
		{
			ExFreePool(VolumeName.Buffer);
		}
		return STATUS_INVALID_PARAMETER;
	}

	ObDereferenceObject(FileObject);
	ZwClose(FileHandle);
	if (WillFreeVolumeName && MmIsAddressValid(VolumeName.Buffer))
	{
		ExFreePool(VolumeName.Buffer);
	}

		return STATUS_SUCCESS;
}

/*========================================================================
*
* ������:	StoreOrCheckDll
*
* ����:		BOOLEAN		[IN] storeOrCheck - TRUE�洢��FALSE���
*			ULONG		[IN] index		  - ģ���ַ
*			ULONG		[IN] imageSize	  - ģ���С
*			PWCHAR		[IN] dosPath	  - ģ��·��
*			PDLL_ITEM	[IN] pDllItem	  - ָ��Dll�ṹ��ָ��
*			PULONG		[IN] dllNum		  - dll����
*
* ��������:	�洢����Dllģ��
*
* ����ֵ:	TRUE������FALSEΪ����
*
=========================================================================*/
BOOLEAN StoreOrCheckDll(BOOLEAN	storeOrCheck, ULONG baseAddr, ULONG imageSize, PWCHAR dosPath, PDLL_ITEM pDllItem, PULONG numOfDll)
{
	ULONG	i=0;
	ULONG	h=0;

	//��ַor��С������<=0
	if (baseAddr <= 0  ||  imageSize <= 0)
		return FALSE;

	//�洢
	if (storeOrCheck)
	{
		//dll����
		(*numOfDll)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			//��Eproc��������iָ��ĵ�ַΪ�յĵط��洢
			if ( pDllItem[i].ulBaseAddr == 0)
			{
//				KdPrint(("StoreOrCheckDll(%08X, %ws)\n", pDllItem[i].ulBaseAddr, pDllItem[i].pwszPath));
				/*�洢dllģ�������Ϣ*/
				pDllItem[i].ulBaseAddr=baseAddr;		//��ַ
				pDllItem[i].ulImageSize=imageSize;		//ģ���С
				wcscpy(pDllItem[i].pwszPath, dosPath);	//ģ��·��

				return TRUE;
			}
		}
	}
	else
	{
		//���
		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			//�Ѵ��򷵻�
			if (baseAddr == pDllItem[i].ulBaseAddr)
				return TRUE;
		}

		//�������أ�dll��������
		(*numOfDll)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			if (pDllItem[i].ulBaseAddr == 0)
			{
				/*���뵽��¼��*/
				//ģ���ַ
				pDllItem[i].ulBaseAddr=baseAddr;
				//ģ���С
				pDllItem[i].ulImageSize=imageSize;
				//ģ��·��
				wcscpy(pDllItem[i].pwszPath, dosPath);

				//ָ���洢����dll
				for (h=0; h<MAX_Dll_NUM*2; h++)
				{
					if (pDllItem[i].ulBaseAddr == 0)
					{
						//ģ���ַ
						pDllItem[h].ulBaseAddr=baseAddr;
						//ģ���С
						pDllItem[h].ulImageSize=imageSize;
						//����·��
						wcscpy(pDllItem[h].pwszPath, dosPath);
						return FALSE;
					}
				}
			}
		}
	}

	return TRUE;
}