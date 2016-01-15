//////////////////////////////////////////////////////////////////////////
// Process.c

#include "precomp.h"
#include "func.h"
#include "WinOSVersion.h"
#include "ASM/LDasm.h"
#include "dll.h"
#include "proc.h"



//��ȡ�����б�
NTSTATUS GetProcessList()
{
	NTSTATUS status=STATUS_SUCCESS;
	ULONG		i;
	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_UNSUCCESSFUL;

	//��ռ�¼
	RtlZeroMemory( Eproc, sizeof(ULONG)*1024 );
	RtlZeroMemory( hideEproc, sizeof(ULONG)*128 );

	//ZwQuerySysInfoö�ٽ���
	status=EnumProcessByZwQuerySysInfo();
	//PspCidTable������ؽ���
	status=CheckHiddenProcByPspCidTable();
	//ActiveProcessList������ؽ���
	status=CheckHiddenProcByActiveProcListHead();

	return status;
}

//ZwQuerySystemInformationö�ٽ���
NTSTATUS EnumProcessByZwQuerySysInfo()
{
	PVOID	pBuftmp = NULL;
	ULONG	 dwRetSize = 0;
	NTSTATUS status = STATUS_SUCCESS;
	PSYSTEM_PROCESS_INFORMATION	pSysProcInfo = NULL;
	PEPROCESS	idleAddr;
	PEPROCESS	pEproc;

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_UNSUCCESSFUL;

	//��ȡ��С
	status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, NULL, 0, &dwRetSize);
	//�����ڴ�
	pBuftmp = ExAllocatePool(NonPagedPool, dwRetSize);		//dwRetSize ��Ҫ�Ĵ�С
	if (pBuftmp != NULL)
	{
		//�ٴ�ִ��,��ö�ٽ���ŵ�ָ�����ڴ�����
		status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, pBuftmp, dwRetSize, NULL);
		if ( NT_SUCCESS(status) )
		{
			/*������࣬�Ȼ�ȡIdle���̵�EPROCESS��ַ���洢*/
			__asm
			{
				push eax
				mov	 eax, fs:[0x20]		//�õ�KPRCB�ĵ�ַ
				add	 eax, 0xc			//�õ�ָ��IdleThread�ĵ�ַ
				mov  eax, [eax]			//�õ�IdleThread�ĵ�ַ
				add	 eax, 0x34			//_KAPC_STATE�ṹ��ַ
				add  eax, 0x10			//ָ��_KPROCESS�ĵ�ַ
				mov  eax, [eax]			//ȡ��_KPROCESS�ĵ�ַ
				mov  idleAddr, eax		//���浽idleAddr����
				pop  eax
			}
			//��Ӵ洢idle������
			StoreOrCheckProcess( TRUE, (ULONG)idleAddr );

			pSysProcInfo = (PSYSTEM_PROCESS_INFORMATION)pBuftmp;
			//ѭ������
			while (TRUE)
			{
				pEproc=NULL;
				if ( PsLookupProcessByProcessId( (HANDLE)pSysProcInfo->ProcessId, &pEproc) == STATUS_SUCCESS)
					//�洢��������
					StoreOrCheckProcess(TRUE, (ULONG)pEproc);

				//ptagProc->NextEntryOffset==0 ��������������β��
				if (pSysProcInfo->NextEntryOffset == 0)
					break;

				//��һ���ṹ
				pSysProcInfo = (PSYSTEM_PROCESS_INFORMATION)( (ULONG)pSysProcInfo + pSysProcInfo->NextEntryOffset);
			}
		}
	}

	ExFreePool(pBuftmp);
	return status;
}

/*========================================================================
*
* ������:	CheckHideProcByPspCidTable
*
* ����:		��
*
* ��������:	������ؽ���ͨ��PspCidTable
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS CheckHiddenProcByPspCidTable()
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG		pCidTableAddr = 0;
	PHANDLE_TABLE	pCidHandleTable = NULL;
	HANDLE		hLastHandle;

	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_UNSUCCESSFUL;

	GetPspCidTableAddrByLookupPid(&pCidTableAddr);	// �����붨λPsLookupProcessByProcessId->PspCidTable

	pCidHandleTable = (PHANDLE_TABLE)( *(PULONG)pCidTableAddr );

	ExEnumHandleTable( pCidHandleTable, IxEnumHandleCallBack, NULL, &hLastHandle );

	return status;
}

/*========================================================================
*
* ������:	IxEnumHandleCallBack
*
* ����:		PHANDLE_TABLE_ENTRY		[IN] -HandleTableEntry,
*			HANDLE					[IN] -Handle
*			PVOID					[IN] -EnumParameter
*
* ��������:	EnumHandleProcedure
*
* ����ֵ:	BOOLEAN
*
=========================================================================*/
BOOLEAN IxEnumHandleCallBack( IN PHANDLE_TABLE_ENTRY HandleTableEntry,
							 IN HANDLE Handle,
							 IN PVOID EnumParameter )
{
	NTSTATUS	status;
	HANDLE		Pid;
	PEPROCESS	pEprocess;
	ULONG		uTableCount;
	ULONG		uTablePage = 0;


	if( EnumParameter == HandleTableEntry )
	{
		return TRUE;
	}
	else
	{
		for( uTableCount = 0; uTableCount < 0x1000; uTableCount++ )
		{
			if( HandleTableEntry->Object )
			{
				// Pid
				Pid = (HANDLE)( (1024 * uTablePage) + (uTableCount << 2) );
				if ( Pid > (HANDLE)4 )
				{
					// eprocess
					status = PsLookupProcessByProcessId( Pid, &pEprocess );
					if(NT_SUCCESS(status))
					{
						KdPrint(( "Pid: %4d \t ProcessName: %-16s\n",
										Pid, PsGetProcessImageFileName( (PEPROCESS)pEprocess) ));

						// �ռ�eprocess��Ϣ
						StoreOrCheckProcess( FALSE, (ULONG)pEprocess );

						ObDereferenceObject(pEprocess);
					}
				}
				else if ( Pid == (HANDLE)0 || Pid == (HANDLE)4 )
				{
					KdPrint(( "Pid: %4d \tProcessName: Idle or System\n", Pid ));
				}

			} // end if
		} // end for

		uTablePage++;
		return TRUE;
	}
}

/*========================================================================
*
* ������:	GetPspCidTableAddrByLookupPid
*
* ����:		��
*
* ��������:	ͨ������PsLookupProcessByProcessId��������ȡPspCidTable�ĵ�ַ
*
* ����ֵ:	BOOLEAN
*
=========================================================================*/
NTSTATUS
GetPspCidTableAddrByLookupPid( PULONG pPspCidTableAddr )
{
	NTSTATUS		status = STATUS_NOT_FOUND;
	PUCHAR		cPtr;
	unsigned char * pOpcode;
	ULONG			Length;
	ULONG			pCidHandlePtr=0;


	for ( cPtr = (PUCHAR)PsLookupProcessByProcessId;
		cPtr < (PUCHAR)PsLookupProcessByProcessId + PAGE_SIZE;
		cPtr += Length )
	{
		Length = SizeOfCode( cPtr, &pOpcode );	//credit to LDasm.c by Ms-Rem
		if ( !Length )
			break;

		if ( *(PUSHORT)cPtr == 0x35FF && *(pOpcode + 6) == 0xE8 )
		{
			pCidHandlePtr = *(PULONG)(pOpcode + 2);

			KdPrint(( "[func]GetPspCidTableAddrByLookupPid: PspCidTable Addr: 0x%08x\n", pCidHandlePtr ));
			*pPspCidTableAddr = pCidHandlePtr;

			status = STATUS_SUCCESS;
			break;
		}
	}

	return status;
}

//PsActiveProcessList
NTSTATUS CheckHiddenProcByActiveProcListHead()
{
	NTSTATUS status=STATUS_SUCCESS;
	PLIST_ENTRY	ListEnty;


	if (PsActiveProcessHead == NULL)
	{
		KdPrint(("[proc]CheckHiddenProcByPsActiveProcessHead->g_PsActiveProcessHead==NULL\n" ));
		return STATUS_UNSUCCESSFUL;
	}

	/* ���� ActiveProcessList */
	ListEnty = PsActiveProcessHead->Flink;

	while (ListEnty != PsActiveProcessHead)
	{
		if (g_EPROCESS_Flags != 0)
		{
			if (( *(ULONG *)( (ULONG)ListEnty - g_EPROCESS_ActiveProcessLinks + g_EPROCESS_Flags ) & 0x4 ) == 0)
			{
				KdPrint(( "[$ixer]ActiveProcList->Eproc: 0x%08X ProcessName: %-16s\n",
					(ULONG)ListEnty - g_EPROCESS_ActiveProcessLinks,
					PsGetProcessImageFileName( (PEPROCESS)( (ULONG)ListEnty - g_EPROCESS_ActiveProcessLinks)) ));

				StoreOrCheckProcess(FALSE, (ULONG)((ULONG)ListEnty - g_EPROCESS_ActiveProcessLinks) );
			}
		}
		else
		{
			KdPrint(( "[$ixer]ActiveProcList->Flags=0 Eproc: 0x%08X ProcessName: %-16s\n",
				(ULONG)ListEnty - g_EPROCESS_ActiveProcessLinks,
				PsGetProcessImageFileName( (PEPROCESS)( (ULONG)ListEnty - g_EPROCESS_ActiveProcessLinks)) ));

			StoreOrCheckProcess(FALSE, (ULONG)((ULONG)ListEnty - g_EPROCESS_ActiveProcessLinks) );
		}

		ListEnty = ListEnty->Flink;
	}

	return status;
}

/*========================================================================
*
* ������:	StoreOrCheckProcess
*
* ����:		BOOLEAN		[IN] storeOrCheck - TRUE�洢��FALSE���
*			ULONG		[IN] ulEproc	  - ����EPROCESS����
*
* ��������:	�洢�������
*
* ����ֵ:	TRUE������FALSEΪ����
*
=========================================================================*/
BOOLEAN StoreOrCheckProcess( BOOLEAN	storeOrCheck, ULONG ulEproc )
{
	ULONG	i=0;
	ULONG	h=0;

	/* �����Ϊ�գ��������˳� */
	if ( *(PULONG)(ulEproc + g_EPROCESS_ObjectTable) == 0 )
		return FALSE;

	//�洢
	if (storeOrCheck)
	{
		for (i=0; i<1024; i++)
		{
			//��Eproc��������iָ��ĵ�ַΪ�յĵط��洢
			if (Eproc[i] == 0)
			{
				Eproc[i]=ulEproc;	//�洢ö�ٵ���EPROCESS
				return TRUE;
			}
		}
	}
	else	//���
	{
		for (i=0; i<1024; i++)
		{
			//�Ѵ��򷵻�
			if (Eproc[i] == ulEproc)
				return TRUE;
		}

		//��Eproc������û�ҵ���������:
// 		for (i=0; i<1024; i++)
// 		{
		//��Eproc����iָ��ĵ�ַΪ�յĵط��洢
// 			if (Eproc[i] == 0)
// 			{
		//��Ӵ洢��⵽��EPROCESS(���ؽ���)
//				Eproc[i]=ulEproc;

			//��ö�ٵ�Eproc������û�ҵ������أ���:
			for (h=0; h<128; h++)
			{
				//�Ѽ�⵽���򷵻�
				if (hideEproc[h] == ulEproc)
					return FALSE;
			}
				//���¼���ؽ��̵�EPROCESS
				for ( h=0; h<128; h++ )
				{
					//��hideEproc����hָ��ĵ�ַΪ�յĵ�ַ�洢
					if ( hideEproc[h] == 0 )
					{
						//���¼��⵽�����ؽ���EPROCESS
						hideEproc[h]=ulEproc;
						return FALSE;
					}
				}
//			}
//		}
	}

	return FALSE;
}

/*========================================================================
*	Win2k������ZwQueryInformationProcess->ProcessImageFileName
* ������:	GetImageFullPathByEprocess
*
* ����:		PEPROCESS		[IN]  pEPROCESS		- ����EPROCESS����
*			PANSI_STRING	[OUT] imageFullPath	- ��������·��
*
* ��������:	��ȡ��������·��ͨ��EPROCESS
*
* ����ֵ:	NTSTATUS
*
* ˼·:
*			1. ����ZwQueryInformationProcess��0x1b�Ź��ܻ�ý��̵�NT·��
*
*			2. ������ZwOpenFile��NT·������ý��̵ľ��
*
*			3. ������ObReferenceObjectByHandle����ں��ļ�����(FileObj)
*
*			4. ����������̷�("C:\")��RtlVolumeDeviceToDosName(FileObj->DeviceObject, &DosName);
*
*			5. ƴ��·����RtlAppendUnicodeStringToString(&ImageFullPath, &FileObj->FileName);
*
=========================================================================*/
NTSTATUS GetImageFullPathByEprocess( PEPROCESS pEPROCESS, PCHAR pszimageFullPath )
{
	NTSTATUS	status=STATUS_SUCCESS;
	ULONG		rtnLen=0;
	PVOID		pBuffer;

	HANDLE				hProc=NULL;
	OBJECT_ATTRIBUTES	objAttributes = {0};
	IO_STATUS_BLOCK		ioStatusBlock;

	UNICODE_STRING		imagePath = {0};
	UNICODE_STRING		dosName = {0};
	ANSI_STRING			pAnsiPath;

	HANDLE				fileHandle = NULL;
	PFILE_OBJECT		fileObj = NULL;

	//"����·��"�ڴ�����
	RtlZeroMemory( pszimageFullPath, sizeof(PCHAR) );

	__try
	{
		if( IsValidAddr( (ULONG)pEPROCESS ) != VALID_PAGE )
		{
			dprintf("pEPROCESS=0x%08X is a invalid Page!\n", pEPROCESS);
			return STATUS_UNSUCCESSFUL;
		}

		//����EPROCESS����
		ObReferenceObject(pEPROCESS);
		status=ObOpenObjectByPointer(pEPROCESS,			// Object
									OBJ_KERNEL_HANDLE,  // HandleAttributes
									NULL,               // PassedAccessState OPTIONAL
									GENERIC_READ,       // DesiredAccess
									*PsProcessType,     // ObjectType
									KernelMode,         // AccessMode
									&hProc);
		if ( !NT_SUCCESS(status) )
		{
			dprintf("[process.c]ObOpenObjectByPointer() Error! \nstatus=%d, pEPROCESS=0x%08X\n",
				status, pEPROCESS);

			goto Ret;
		}
	}__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//����Eprocess�����������쳣����idle�Ƚ��̵ľܾ�����
		if (pszimageFullPath[0] == 0)
			strcpy(pszimageFullPath, (PCSZ)( (ULONG)pEPROCESS + g_EPROCESS_ImageFileName ));

		return STATUS_UNSUCCESSFUL;
	}

	//��ѯ����
	status=ZwQueryInformationProcess(hProc, ProcessImageFileName, NULL, 0, &rtnLen);
	if (status != STATUS_INFO_LENGTH_MISMATCH  ||  rtnLen == 0)
		goto Ret;

	//���ݳ��ȷ����ڴ�
	pBuffer=ExAllocatePool(NonPagedPool, rtnLen);
	if (pBuffer == NULL)
	{
		status=STATUS_INSUFFICIENT_RESOURCES;
		goto Ret;
	}

	//��ѯ����·��
	status=ZwQueryInformationProcess(hProc, ProcessImageFileName, pBuffer, rtnLen, &rtnLen);
	if ( !NT_SUCCESS(status) )
		goto Ret;

	//��ʼ����������Ϊ�ں˾��
	InitializeObjectAttributes(&objAttributes, pBuffer, OBJ_KERNEL_HANDLE, NULL, NULL);

	//�򿪶���õ��ļ����
	status=ZwOpenFile(&fileHandle, 0, &objAttributes, &ioStatusBlock, (ULONG)0, (ULONG)0);
	if ( !NT_SUCCESS(status) )
		goto Ret;

	//���ö���ͨ�����
	status=ObReferenceObjectByHandle(fileHandle, 0, NULL, KernelMode, &fileObj, NULL);
	if ( !NT_SUCCESS(status) )
		goto Ret;

	if (fileObj->DeviceObject && fileObj->FileName.Buffer)
	{
		//���豸����תΪ�������������̷� "\Device\HarddiskVolume1"->'C:\'
		RtlVolumeDeviceToDosName(fileObj->DeviceObject, &dosName);

		//��ȡ����
		imagePath.MaximumLength = dosName.Length + fileObj->FileName.Length;
		//�����ڴ�
		imagePath.Buffer = ExAllocatePool(NonPagedPool, imagePath.MaximumLength);

		RtlCopyUnicodeString(&imagePath, &dosName);
		//�̷��ٸ������ļ�����dos·���ַ��� "C:\" + "\WINDOWS\explorer.exe"
		RtlAppendUnicodeStringToString(&imagePath, &fileObj->FileName);

		//תAnsi�ַ���
		RtlUnicodeStringToAnsiString(&pAnsiPath, &imagePath, TRUE);
		//���"����·��"
		strcpy(pszimageFullPath, pAnsiPath.Buffer);		//����ӳ����

		//�ͷ��ַ�����Դ
		RtlFreeUnicodeString(&imagePath);
		RtlFreeUnicodeString(&dosName);
		RtlFreeAnsiString(&pAnsiPath);
	}

Ret:
	//��"����·��"�ַ�������Ϊ�գ�����EPROCESS�ṹ�е�"ӳ����"��Ϊ·��(Idle��System����)
	if (pszimageFullPath[0] == 0)
		strcpy(pszimageFullPath, (PCSZ)( (ULONG)pEPROCESS + g_EPROCESS_ImageFileName ));
//		dprintf("[process.c]GetImageFullPathByEprocess(0x%08X, %s)\n", pEPROCESS, pszimageFullPath);

	/*�ͷ���Դ*/
	if (hProc)
		ZwClose(hProc);					//�رվ��

	if (pBuffer)
		ExFreePool(pBuffer);			//�ͷ��ڴ�

	if (fileObj)
		ObDereferenceObject(fileObj);	//����ļ���������

	if (pEPROCESS)
		ObDereferenceObject(pEPROCESS);	//���Eprocess��������

	return status;
}

//����ַ��Ч��
ULONG IsValidAddr( ULONG uAddr )
{
	ULONG uInfo;
	ULONG uCr4;
	ULONG uPdeAddr;
	ULONG uPteAddr;
	_asm
	{
		cli
			push eax

			_emit 0x0F
			_emit 0x20
			_emit 0xE0//mov eax,cr4

			mov [uCr4], eax
			pop eax
	}

	_asm sti

		uInfo = uCr4 & 0x20;
	if( uInfo != 0 )
	{
		uPdeAddr = (uAddr>>21)*8+0xC0600000;
	}
	else
		uPdeAddr = (uAddr>>22)*4+0xc0300000;
	if( (*(PULONG)uPdeAddr & 0x1) != 0 )
	{
		if( (*(PULONG)uPdeAddr & 0x80) != 0 )
		{
			return VALID_PAGE;
		}
		else
		{
			if( uInfo != 0 )
			{
				uPteAddr = (uAddr>>12)*8+0xc0000000;
			}
			else
			{
				uPteAddr = (uAddr>>12)*4+0xc0000000;
			}

			if( (*(PULONG)uPteAddr & 0x1) != 0 )
				return VALID_PAGE;
			else
				return PTEINVALID;
		}
	}
	else
		return PDEINVALID;
}

/*========================================================================
*
* ������:	ProcessKiller
*
* ����:		PEPROCESS	[IN] pEproc - ����������EPROCESS����
*
* ��������:	��������
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS KillProcByPspTerminateThreadByPointer( PEPROCESS pEproc )
{
	NTSTATUS	status;
	PETHREAD	pEthread;

	dprintf("$XTR:ProcessKiller(pEproc=0x%08x)\n", pEproc);
	__try
	{
		for (pEthread = GetNextProcessThread( pEproc, NULL );
			pEthread != NULL;
			pEthread = GetNextProcessThread( pEproc, pEthread ) )
		{
			status=STATUS_SUCCESS;
			//��̬����PspTerminateThreadByPointer�����߳�
			status = (*MyPspTerminateThreadByPointer)(pEthread, 0);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
	}

	return status;
}

/*========================================================================
*
* ������:	GetNextProcessThread
*
* ����:		PEPROCESS	[IN] Process - Ҫö���̵߳Ľ���
*			PETHREAD	[IN] Thread	 - ��ǰ�߳�
*
* ��������:	��ȡ������һ���߳�
*
* ����ֵ:	PETHREAD
*
=========================================================================*/
PETHREAD
GetNextProcessThread( PEPROCESS pEproc, PETHREAD pEthread )
{
	PETHREAD FoundThread=NULL;
	PLIST_ENTRY	ThreadListEntry;
	PLIST_ENTRY	Entry, ListHead;

	dprintf("$ixer:GetNextProcessThread(pEproc=0x%08x, pEthread=0x%08x)\n", pEproc, pEthread);
	if (pEthread  != NULL )
	{
		/* �� Thread ������ʼ */
		ThreadListEntry = (PLIST_ENTRY)( (ULONG)pEthread + g_ETHREAD_ThreadListEntry );
		// Entry = pEthread->ThreadListEntry.Flink;
		Entry = ThreadListEntry->Flink;
	}
	else
	{
		/* ��ͷ��ʼ */
		ThreadListEntry = (PLIST_ENTRY)( (ULONG)pEproc + g_ETHREAD_ThreadListHead);
		// Entry = pEproc->ThreadListHead.Flink;
		Entry = ThreadListEntry->Flink;
	}

	// ListHead = &pEproc->ThreadListHead;
	ListHead = (PLIST_ENTRY)( (ULONG)pEproc + g_ETHREAD_ThreadListHead);
	while (ListHead != Entry)
	{
		// FoundThread = CONTAINING_RECORD(Entry, ETHREAD, ThreadListEntry);
		FoundThread = (PETHREAD)( (ULONG)Entry - g_ETHREAD_ThreadListEntry );
		/* �ɹ������̶߳��� */
		if ( ObReferenceObject(FoundThread) )
			break;

		FoundThread = NULL;
		Entry = Entry->Flink;
	}

	if (FoundThread != NULL)
	{
		ObDereferenceObject( FoundThread );
	}

	return FoundThread;
}