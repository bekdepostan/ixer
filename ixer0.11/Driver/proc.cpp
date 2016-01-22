//////////////////////////////////////////////////////////////////////////
// Process.c

#include "precomp.h"
#include "func.h"
#include "WinOSVersion.h"
#include "ASM/LDasm.h"
#include "dll.h"
#include "proc.h"



//获取进程列表
NTSTATUS GetProcessList()
{
	NTSTATUS status=STATUS_SUCCESS;
	if (KeGetCurrentIrql() != PASSIVE_LEVEL)
		return STATUS_UNSUCCESSFUL;

	//清空记录
	RtlZeroMemory( Eproc, sizeof(ULONG)*1024 );
	RtlZeroMemory( hideEproc, sizeof(ULONG)*128 );

	//ZwQuerySysInfo枚举进程
	status=EnumProcessByZwQuerySysInfo();
	//PspCidTable检测隐藏进程
	status=CheckHiddenProcByPspCidTable();
	//ActiveProcessList检测隐藏进程
	status=CheckHiddenProcByActiveProcListHead();

	return status;
}

//ZwQuerySystemInformation枚举进程
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

	//获取大小
	status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, NULL, 0, &dwRetSize);
	//申请内存
	pBuftmp = ExAllocatePool(NonPagedPool, dwRetSize);		//dwRetSize 需要的大小
	if (pBuftmp != NULL)
	{
		//再次执行,将枚举结果放到指定的内存区域
		status = ZwQuerySystemInformation(SystemProcessesAndThreadsInformation, pBuftmp, dwRetSize, NULL);
		if ( NT_SUCCESS(status) )
		{
			/*内联汇编，先获取Idle进程的EPROCESS地址并存储*/
			__asm
			{
				push eax
				mov	 eax, fs:[0x20]		//得到KPRCB的地址
				add	 eax, 0xc			//得到指向IdleThread的地址
				mov  eax, [eax]			//得到IdleThread的地址
				add	 eax, 0x34			//_KAPC_STATE结构地址
				add  eax, 0x10			//指向_KPROCESS的地址
				mov  eax, [eax]			//取得_KPROCESS的地址
				mov  idleAddr, eax		//保存到idleAddr变量
				pop  eax
			}
			//添加存储idle进程项
			StoreOrCheckProcess( TRUE, (ULONG)idleAddr );

			pSysProcInfo = (PSYSTEM_PROCESS_INFORMATION)pBuftmp;
			//循环遍历
			while (TRUE)
			{
				pEproc=NULL;
				if ( PsLookupProcessByProcessId( (HANDLE)pSysProcInfo->ProcessId, &pEproc) == STATUS_SUCCESS)
					//存储、检测进程
					StoreOrCheckProcess(TRUE, (ULONG)pEproc);

				//ptagProc->NextEntryOffset==0 即遍历到了链表尾部
				if (pSysProcInfo->NextEntryOffset == 0)
					break;

				//下一个结构
				pSysProcInfo = (PSYSTEM_PROCESS_INFORMATION)( (ULONG)pSysProcInfo + pSysProcInfo->NextEntryOffset);
			}
		}
	}

	ExFreePool(pBuftmp);
	return status;
}

/*========================================================================
*
* 函数名:	CheckHideProcByPspCidTable
*
* 参数:		无
*
* 功能描述:	检测隐藏进程通过PspCidTable
*
* 返回值:	NTSTATUS
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

	GetPspCidTableAddrByLookupPid(&pCidTableAddr);	// 特征码定位PsLookupProcessByProcessId->PspCidTable

	pCidHandleTable = (PHANDLE_TABLE)( *(PULONG)pCidTableAddr );

	ExEnumHandleTable( pCidHandleTable, IxEnumHandleCallBack, NULL, &hLastHandle );

	return status;
}

/*========================================================================
*
* 函数名:	IxEnumHandleCallBack
*
* 参数:		PHANDLE_TABLE_ENTRY		[IN] -HandleTableEntry,
*			HANDLE					[IN] -Handle
*			PVOID					[IN] -EnumParameter
*
* 功能描述:	EnumHandleProcedure
*
* 返回值:	BOOLEAN
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

						// 收集eprocess信息
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
* 函数名:	GetPspCidTableAddrByLookupPid
*
* 参数:		无
*
* 功能描述:	通过搜索PsLookupProcessByProcessId函数，获取PspCidTable的地址
*
* 返回值:	BOOLEAN
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

	/* 遍历 ActiveProcessList */
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
* 函数名:	StoreOrCheckProcess
*
* 参数:		BOOLEAN		[IN] storeOrCheck - TRUE存储，FALSE检测
*			ULONG		[IN] ulEproc	  - 进程EPROCESS对象
*
* 功能描述:	存储或检测进程
*
* 返回值:	TRUE正常，FALSE为隐藏
*
=========================================================================*/
BOOLEAN StoreOrCheckProcess( BOOLEAN	storeOrCheck, ULONG ulEproc )
{
	ULONG	i=0;
	ULONG	h=0;

	/* 句柄表为空，进程已退出 */
	if ( *(PULONG)(ulEproc + g_EPROCESS_ObjectTable) == 0 )
		return FALSE;

	//存储
	if (storeOrCheck)
	{
		for (i=0; i<1024; i++)
		{
			//找Eproc数组索引i指向的地址为空的地方存储
			if (Eproc[i] == 0)
			{
				Eproc[i]=ulEproc;	//存储枚举到的EPROCESS
				return TRUE;
			}
		}
	}
	else	//检测
	{
		for (i=0; i<1024; i++)
		{
			//已存则返回
			if (Eproc[i] == ulEproc)
				return TRUE;
		}

		//在Eproc数组中没找到，即隐藏:
// 		for (i=0; i<1024; i++)
// 		{
		//找Eproc索引i指向的地址为空的地方存储
// 			if (Eproc[i] == 0)
// 			{
		//添加存储检测到的EPROCESS(隐藏进程)
//				Eproc[i]=ulEproc;

			//在枚举的Eproc数组中没找到即隐藏，则:
			for (h=0; h<128; h++)
			{
				//已检测到，则返回
				if (hideEproc[h] == ulEproc)
					return FALSE;
			}
				//另记录隐藏进程的EPROCESS
				for ( h=0; h<128; h++ )
				{
					//找hideEproc索引h指向的地址为空的地址存储
					if ( hideEproc[h] == 0 )
					{
						//另记录检测到的隐藏进程EPROCESS
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
*	Win2k以上用ZwQueryInformationProcess->ProcessImageFileName
* 函数名:	GetImageFullPathByEprocess
*
* 参数:		PEPROCESS		[IN]  pEPROCESS		- 进程EPROCESS对象
*			PANSI_STRING	[OUT] imageFullPath	- 镜像完整路径
*
* 功能描述:	获取镜像完整路径通过EPROCESS
*
* 返回值:	NTSTATUS
*
* 思路:
*			1. 利用ZwQueryInformationProcess的0x1b号功能获得进程的NT路径
*
*			2. 再利用ZwOpenFile打开NT路径，获得进程的句柄
*
*			3. 再利用ObReferenceObjectByHandle获得内核文件对象(FileObj)
*
*			4. 获得驱动器盘符("C:\")，RtlVolumeDeviceToDosName(FileObj->DeviceObject, &DosName);
*
*			5. 拼接路径，RtlAppendUnicodeStringToString(&ImageFullPath, &FileObj->FileName);
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

	//"镜像路径"内存清零
	RtlZeroMemory( pszimageFullPath, sizeof(PCHAR) );

	__try
	{
		if( IsValidAddr( (ULONG)pEPROCESS ) != VALID_PAGE )
		{
			dprintf("pEPROCESS=0x%08X is a invalid Page!\n", pEPROCESS);
			return STATUS_UNSUCCESSFUL;
		}

		//引用EPROCESS对象
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
		//访问Eprocess对象引发的异常，如idle等进程的拒绝访问
		if (pszimageFullPath[0] == 0)
			strcpy(pszimageFullPath, (PCSZ)( (ULONG)pEPROCESS + g_EPROCESS_ImageFileName ));

		return STATUS_UNSUCCESSFUL;
	}

	//查询长度
	status=ZwQueryInformationProcess(hProc, ProcessImageFileName, NULL, 0, &rtnLen);
	if (status != STATUS_INFO_LENGTH_MISMATCH  ||  rtnLen == 0)
		goto Ret;

	//根据长度分配内存
	pBuffer=ExAllocatePool(NonPagedPool, rtnLen);
	if (pBuffer == NULL)
	{
		status=STATUS_INSUFFICIENT_RESOURCES;
		goto Ret;
	}

	//查询进程路径
	status=ZwQueryInformationProcess(hProc, ProcessImageFileName, pBuffer, rtnLen, &rtnLen);
	if ( !NT_SUCCESS(status) )
		goto Ret;

	//初始化对象属性为内核句柄
	InitializeObjectAttributes(&objAttributes, pBuffer, OBJ_KERNEL_HANDLE, NULL, NULL);

	//打开对象得到文件句柄
	status=ZwOpenFile(&fileHandle, 0, &objAttributes, &ioStatusBlock, (ULONG)0, (ULONG)0);
	if ( !NT_SUCCESS(status) )
		goto Ret;

	//引用对象通过句柄
	status=ObReferenceObjectByHandle(fileHandle, 0, NULL, KernelMode, &fileObj, NULL);
	if ( !NT_SUCCESS(status) )
		goto Ret;

	if (fileObj->DeviceObject && fileObj->FileName.Buffer)
	{
		//卷设备对象转为驱动器名，即盘符 "\Device\HarddiskVolume1"->'C:\'
		RtlVolumeDeviceToDosName(fileObj->DeviceObject, &dosName);

		//获取长度
		imagePath.MaximumLength = dosName.Length + fileObj->FileName.Length;
		//申请内存
		imagePath.Buffer = ExAllocatePool(NonPagedPool, imagePath.MaximumLength);

		RtlCopyUnicodeString(&imagePath, &dosName);
		//盘符再附加上文件对象dos路径字符串 "C:\" + "\WINDOWS\explorer.exe"
		RtlAppendUnicodeStringToString(&imagePath, &fileObj->FileName);

		//转Ansi字符串
		RtlUnicodeStringToAnsiString(&pAnsiPath, &imagePath, TRUE);
		//填充"镜像路径"
		strcpy(pszimageFullPath, pAnsiPath.Buffer);		//拷贝映像名

		//释放字符串资源
		RtlFreeUnicodeString(&imagePath);
		RtlFreeUnicodeString(&dosName);
		RtlFreeAnsiString(&pAnsiPath);
	}

Ret:
	//如"镜像路径"字符串数组为空，则用EPROCESS结构中的"映像名"做为路径(Idle、System进程)
	if (pszimageFullPath[0] == 0)
		strcpy(pszimageFullPath, (PCSZ)( (ULONG)pEPROCESS + g_EPROCESS_ImageFileName ));
//		dprintf("[process.c]GetImageFullPathByEprocess(0x%08X, %s)\n", pEPROCESS, pszimageFullPath);

	/*释放资源*/
	if (hProc)
		ZwClose(hProc);					//关闭句柄

	if (pBuffer)
		ExFreePool(pBuffer);			//释放内存

	if (fileObj)
		ObDereferenceObject(fileObj);	//解除文件对象引用

	if (pEPROCESS)
		ObDereferenceObject(pEPROCESS);	//解除Eprocess对象引用

	return status;
}

//检测地址有效性
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
* 函数名:	ProcessKiller
*
* 参数:		PEPROCESS	[IN] pEproc - 被结束进程EPROCESS对象
*
* 功能描述:	结束进程
*
* 返回值:	NTSTATUS
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
			//动态调用PspTerminateThreadByPointer结束线程
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
* 函数名:	GetNextProcessThread
*
* 参数:		PEPROCESS	[IN] Process - 要枚举线程的进程
*			PETHREAD	[IN] Thread	 - 当前线程
*
* 功能描述:	获取进程下一个线程
*
* 返回值:	PETHREAD
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
		/* 从 Thread 参数开始 */
		ThreadListEntry = (PLIST_ENTRY)( (ULONG)pEthread + g_ETHREAD_ThreadListEntry );
		// Entry = pEthread->ThreadListEntry.Flink;
		Entry = ThreadListEntry->Flink;
	}
	else
	{
		/* 从头开始 */
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
		/* 成功引用线程对象 */
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