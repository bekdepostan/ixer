//////////////////////////////////////////////////////////////////////////
// Dll.c

#include "precomp.h"
#include "WindowsVersion.h"
#include "Function.h"
#include "Dll.h"



//��ȡDll�б�
PDLL_ITEM GetDllList(ULONG ulPid, PULONG dllNum, PDLL_ITEM pDllItem)
{
	NTSTATUS status=STATUS_SUCCESS;
	ULONG i=0;

	KdPrint(("����Pid:%d \t Dll����:%d\n", ulPid, *dllNum));
	if (dllNum)
	{
		//��dll���������ڴ�
		pDllItem=(DLL_ITEM *)ExAllocatePoolWithTag(PagedPool, sizeof(DLL_ITEM)*(*dllNum)*2, 'dll');
		if (pDllItem)
		{
			//�ڴ�����
			RtlZeroMemory(pDllItem, sizeof(DLL_ITEM)*(*dllNum)*2);
			//ö��Dll
			status=EnumDllByZwQueryVirtualMem(FALSE, ulPid, dllNum, pDllItem);
		}
	}

	return pDllItem;
}

//ZwQueryVirtualMemoryö��Dll
NTSTATUS EnumDllByZwQueryVirtualMem(BOOLEAN getNum, ULONG ulPid, PULONG dllNum, PDLL_ITEM pDllItem)
{
	NTSTATUS status=0;
	MEMORY_BASIC_INFORMATION mem_info = {0};
	PMEMORY_SECTION_NAME mem_secName;

	HANDLE hProcess;
	OBJECT_ATTRIBUTES obj;
	CLIENT_ID cid;
	PEPROCESS	pEproc;

	int			retLen;
	ULONG		index=0;
	PRKAPC_STATE pApcStatus;

	ULONG *systemCallTable;
	MyZwQueryVirtualMemory ZwQueryVirtualMemory;
	WCHAR dosPath[MAX_PATH]=L"";

	/*��ȡ������ַͨ��SSDT��[�����]*/
	systemCallTable = (ULONG*)KeServiceDescriptorTable.KiServiceTable;
	ZwQueryVirtualMemory=(MyZwQueryVirtualMemory)( systemCallTable[0xB2] );	//index = GetFunctionIndex("NtReadVirtualMemory");
	if (!ZwQueryVirtualMemory)
		KdPrint(("Get \"ZwQueryVirtualMemory\" address failed!\n"));

	/*��ȡĿ�����EPROCESS��������(�л���Ŀ�����������)*/
	status=PsLookupProcessByProcessId( (HANDLE)ulPid, &pEproc);
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("Can't get target process eprocess! ErrorCode= %08X\n", status));
		status = STATUS_UNSUCCESSFUL;
		return status;
	}
	pApcStatus=(PRKAPC_STATE)ExAllocatePoolWithTag(NonPagedPool, sizeof(KAPC_STATE), 'pApc');
	if (pApcStatus)
		KeStackAttachProcess(pEproc, pApcStatus);

	cid.UniqueProcess = (HANDLE)ulPid;
	cid.UniqueThread=NULL;
	InitializeObjectAttributes(&obj, NULL, 0, NULL, NULL);
	/*�򿪽��̻�ȡ���*/
	status=ZwOpenProcess(&hProcess, PAGE_READWRITE, &obj, &cid);
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("Can't get target process handle! Error Code= %08X\n", status));
		status = STATUS_UNSUCCESSFUL;
		return status;
	}

	mem_secName=(PMEMORY_SECTION_NAME)ExAllocatePoolWithTag(NonPagedPool, sizeof(MEMORY_SECTION_NAME)*300, 'Name');
	if (!mem_secName)
	{
		status=STATUS_INSUFFICIENT_RESOURCES;
		return status;
	}

	/*�л��߳�Ϊ�ں�ģʽ*/
	SetCurrentThreadPreviousMode(KernelMode);

	for (index; index < 0x80000000; index += 0x1000)
	{
		//��ѯ�ڴ������Ϣ
		status=ZwQueryVirtualMemory( hProcess,
									 (PULONG)index,
									 MemoryBasicInformation,
									 &mem_info,
									 sizeof(mem_info),
									 NULL );
		if ( status >= 0 )
		{
			//�ж�"�ڴ�����"�Ƿ���"ģ��"����
			if (mem_info.Type == MEM_IMAGE)
			{
				//�ж�ģ���ַ��Χ
				if ( (DWORD)mem_info.AllocationBase == index )
				{
					//��ֻ�ǻ�ȡdll����
					if (getNum)
					{
						(*dllNum)++;
						continue;
					}

					//��ѯ����ģ��������
					status=ZwQueryVirtualMemory( hProcess,
												 (PULONG)index,
												 MemorySectionName,
												 mem_secName,
												 512,
												 &retLen );
					if ( status >= 0 )
					{
						//ת���о��豸��ǰ׺��·��Ϊdos·��
						ConvertVolumeDeviceToDosPath(mem_secName->SectionFileName.Buffer, dosPath);

						//�洢ö�ٵ���Dll��Ϣ
						StoreOrCheckDll(TRUE, index, dosPath, dllNum, pDllItem);
					}
				}
			}
		}
	}

	/*�ͷ���Դ*/
	if (mem_secName)
		ExFreePool(mem_secName);

	//�ر�handle
	if (hProcess)
		ZwClose(hProcess);

	// *tip:�����ڽ���context�йرվ������Ȼ������
	KeUnstackDetachProcess(pApcStatus);
	if (pApcStatus)
		ExFreePool(pApcStatus);

	status=STATUS_SUCCESS;
	return status;
}

/*========================================================================
*
* ������:	SetCurrentThreadPreviousMode
*
* ����:		KPROCESSOR_MODE		[IN] PreviousMode - KernelMode�ں�ģʽ��UserMode�û�ģʽ
*
* ��������:	���õ�ǰ�̵߳�ģʽ
*
* ����ֵ:	KPROCESSOR_MODE
*
=========================================================================*/
KPROCESSOR_MODE SetCurrentThreadPreviousMode(KPROCESSOR_MODE PreviousMode)
{
	PETHREAD pThread=PsGetCurrentThread();
	KPROCESSOR_MODE m_RetMode;
	ULONG m_PrevMode;

	//KThread->PreviousModeƫ����
	if(g_WindowsVersion < 6)
		m_PrevMode=0x140;
	else
		m_PrevMode=0x13a;

	__asm
	{
		mov eax, pThread
		add eax, m_PrevMode
		mov bl, [eax]
		mov m_RetMode, bl
		mov bl, PreviousMode
		mov [eax], bl
	}

	return m_RetMode;
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

// 	VolumeName.Buffer = NULL;
// 	VolumeName.Length = 0;
// 	VolumeName.MaximumLength = 0;

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
*			PDLL_ITEM	[IN] pDllItem	  - ָ��Dll�ṹ��ָ��
*			ULONG		[IN] index		  - ģ���ַ
*			PWCHAR		[IN] dosPath	  - ָ����·����ָ��
*			PULONG		[IN] dllNum		  - ָ��dll������ָ��
*
* ��������:	�洢����Dllģ��
*
* ����ֵ:	TRUE������FALSEΪ����
*
=========================================================================*/
BOOLEAN StoreOrCheckDll(BOOLEAN	storeOrCheck, ULONG baseAddr, PWCHAR dosPath, PULONG dllNum, PDLL_ITEM pDllItem)
{
	ULONG	i=0;
	ULONG	h=0;

	//�洢
	if (storeOrCheck)
	{
		for (i=0; i<(*dllNum)*2; i++)
		{
			//��Eproc��������iָ��ĵ�ַΪ�յĵط��洢
			if ( pDllItem[i].ulBaseAddr == 0)
			{
				//ģ���ַ
				pDllItem[i].ulBaseAddr=baseAddr;
				//����·��
				wcscpy(pDllItem[i].pwszPath, dosPath);

				KdPrint(("StoreOrCheckDll(%08X, %ws)\n", pDllItem[i].ulBaseAddr, pDllItem[i].pwszPath));
				return TRUE;
			}
		}
	}
	else	//���
	{
		for (i=0; i<(*dllNum)*2; i++)
		{
			//�Ѵ��򷵻�
			if (baseAddr == pDllItem[i].ulBaseAddr)
				return TRUE;
		}

		for (i=0; i<(*dllNum)*2; i++)
		{
			if (pDllItem[i].ulBaseAddr == 0)
			{
				//ģ���ַ
				pDllItem[i].ulBaseAddr=baseAddr;
				//����·��
				wcscpy(pDllItem[i].pwszPath, dosPath);

				for (h=0; h<(*dllNum)*2; h++)
				{
					if (pDllItem[i].ulBaseAddr == 0)
					{
						//ģ���ַ
						pDllItem[h].ulBaseAddr=baseAddr;
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
/*
///////////////////////////////
//��    �ܣ����ݽ���ID��ȡ���̾��
//��    ��: ����ID
//�� �� ֵ�����̾��
////////////////////////////////
HANDLE MyOpenProcess( ULONG ulPid )
{
	NTSTATUS status;
	PEPROCESS pEprocess = NULL;
	HANDLE hProcess = NULL;
	UNICODE_STRING Unicode;

	status = PsLookupProcessByProcessId( (HANDLE)ulPid, &pEprocess );
	if ( NT_SUCCESS(status) )	//�жϽ��̺��Ƿ����
	{
		RtlInitUnicodeString(&Unicode, L"PsProcessType");
		//��ȡϵͳ���������ĵ�ַ
		PsProcessType = MmGetSystemRoutineAddress(&Unicode);
		if (PsProcessType)
		{
			status = ObOpenObjectByPointer( pEprocess,
											0,
											NULL,
											PROCESS_ALL_ACCESS,
											(PVOID) *PsProcessType,
											KernelMode,
											&hProcess );
			if ( NT_SUCCESS(status) )
			{
				//����ָ�����
				ObfDereferenceObject(pEprocess);
				return hProcess;
			}
		}

		ObfDereferenceObject(pEprocess);
	}

	return 0;
}
*/