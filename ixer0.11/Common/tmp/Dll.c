//////////////////////////////////////////////////////////////////////////
// Dll.c

#include "precomp.h"
#include "WindowsVersion.h"
#include "Function.h"
#include "Dll.h"



//获取Dll列表
PDLL_ITEM GetDllList(ULONG ulPid, PULONG dllNum, PDLL_ITEM pDllItem)
{
	NTSTATUS status=STATUS_SUCCESS;
	ULONG i=0;

	KdPrint(("进程Pid:%d \t Dll计数:%d\n", ulPid, *dllNum));
	if (dllNum)
	{
		//按dll个数申请内存
		pDllItem=(DLL_ITEM *)ExAllocatePoolWithTag(PagedPool, sizeof(DLL_ITEM)*(*dllNum)*2, 'dll');
		if (pDllItem)
		{
			//内存清零
			RtlZeroMemory(pDllItem, sizeof(DLL_ITEM)*(*dllNum)*2);
			//枚举Dll
			status=EnumDllByZwQueryVirtualMem(FALSE, ulPid, dllNum, pDllItem);
		}
	}

	return pDllItem;
}

//ZwQueryVirtualMemory枚举Dll
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

	/*获取函数地址通过SSDT表[服务号]*/
	systemCallTable = (ULONG*)KeServiceDescriptorTable.KiServiceTable;
	ZwQueryVirtualMemory=(MyZwQueryVirtualMemory)( systemCallTable[0xB2] );	//index = GetFunctionIndex("NtReadVirtualMemory");
	if (!ZwQueryVirtualMemory)
		KdPrint(("Get \"ZwQueryVirtualMemory\" address failed!\n"));

	/*获取目标进程EPROCESS，并附加(切换到目标进程上下文)*/
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
	/*打开进程获取句柄*/
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

	/*切换线程为内核模式*/
	SetCurrentThreadPreviousMode(KernelMode);

	for (index; index < 0x80000000; index += 0x1000)
	{
		//查询内存基本信息
		status=ZwQueryVirtualMemory( hProcess,
									 (PULONG)index,
									 MemoryBasicInformation,
									 &mem_info,
									 sizeof(mem_info),
									 NULL );
		if ( status >= 0 )
		{
			//判断"内存区段"是否是"模块"类型
			if (mem_info.Type == MEM_IMAGE)
			{
				//判断模块基址范围
				if ( (DWORD)mem_info.AllocationBase == index )
				{
					//若只是获取dll个数
					if (getNum)
					{
						(*dllNum)++;
						continue;
					}

					//查询镜像模块区段名
					status=ZwQueryVirtualMemory( hProcess,
												 (PULONG)index,
												 MemorySectionName,
												 mem_secName,
												 512,
												 &retLen );
					if ( status >= 0 )
					{
						//转换有卷、设备名前缀的路径为dos路径
						ConvertVolumeDeviceToDosPath(mem_secName->SectionFileName.Buffer, dosPath);

						//存储枚举到的Dll信息
						StoreOrCheckDll(TRUE, index, dosPath, dllNum, pDllItem);
					}
				}
			}
		}
	}

	/*释放资源*/
	if (mem_secName)
		ExFreePool(mem_secName);

	//关闭handle
	if (hProcess)
		ZwClose(hProcess);

	// *tip:必须在进程context中关闭句柄，不然蓝屏！
	KeUnstackDetachProcess(pApcStatus);
	if (pApcStatus)
		ExFreePool(pApcStatus);

	status=STATUS_SUCCESS;
	return status;
}

/*========================================================================
*
* 函数名:	SetCurrentThreadPreviousMode
*
* 参数:		KPROCESSOR_MODE		[IN] PreviousMode - KernelMode内核模式，UserMode用户模式
*
* 功能描述:	设置当前线程的模式
*
* 返回值:	KPROCESSOR_MODE
*
=========================================================================*/
KPROCESSOR_MODE SetCurrentThreadPreviousMode(KPROCESSOR_MODE PreviousMode)
{
	PETHREAD pThread=PsGetCurrentThread();
	KPROCESSOR_MODE m_RetMode;
	ULONG m_PrevMode;

	//KThread->PreviousMode偏移量
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
* 函数名:	ConvertDeviceName
*
* 参数:		LPWSTR		[IN] FileName		- 要转换的
*			LPWSTR		[OUT] OutFileName	- DOS文件路径
*
* 功能描述:	将形如 "\Device\HarddiskVolume1\xxxx的内核文件路径转化为 DOS文件路径(C:\xxx\xxx..)
*
* 返回值:	NTSTATUS
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
	//初始化对象属性
	InitializeObjectAttributes( &objAttributes, &uniName, OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, 0, 0 );

	//打开对象得到文件句柄
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

	//引用对象通过句柄
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

	//获取盘符，VolumeName.Buffer 返回如:字符串"C:"、"D:"、..
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
* 函数名:	StoreOrCheckDll
*
* 参数:		BOOLEAN		[IN] storeOrCheck - TRUE存储，FALSE检测
*			PDLL_ITEM	[IN] pDllItem	  - 指向Dll结构体指针
*			ULONG		[IN] index		  - 模块基址
*			PWCHAR		[IN] dosPath	  - 指向镜像路径的指针
*			PULONG		[IN] dllNum		  - 指向dll计数的指针
*
* 功能描述:	存储或检测Dll模块
*
* 返回值:	TRUE正常，FALSE为隐藏
*
=========================================================================*/
BOOLEAN StoreOrCheckDll(BOOLEAN	storeOrCheck, ULONG baseAddr, PWCHAR dosPath, PULONG dllNum, PDLL_ITEM pDllItem)
{
	ULONG	i=0;
	ULONG	h=0;

	//存储
	if (storeOrCheck)
	{
		for (i=0; i<(*dllNum)*2; i++)
		{
			//找Eproc数组索引i指向的地址为空的地方存储
			if ( pDllItem[i].ulBaseAddr == 0)
			{
				//模块基址
				pDllItem[i].ulBaseAddr=baseAddr;
				//镜像路径
				wcscpy(pDllItem[i].pwszPath, dosPath);

				KdPrint(("StoreOrCheckDll(%08X, %ws)\n", pDllItem[i].ulBaseAddr, pDllItem[i].pwszPath));
				return TRUE;
			}
		}
	}
	else	//检测
	{
		for (i=0; i<(*dllNum)*2; i++)
		{
			//已存则返回
			if (baseAddr == pDllItem[i].ulBaseAddr)
				return TRUE;
		}

		for (i=0; i<(*dllNum)*2; i++)
		{
			if (pDllItem[i].ulBaseAddr == 0)
			{
				//模块基址
				pDllItem[i].ulBaseAddr=baseAddr;
				//镜像路径
				wcscpy(pDllItem[i].pwszPath, dosPath);

				for (h=0; h<(*dllNum)*2; h++)
				{
					if (pDllItem[i].ulBaseAddr == 0)
					{
						//模块基址
						pDllItem[h].ulBaseAddr=baseAddr;
						//镜像路径
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
//功    能：根据进程ID获取进程句柄
//参    数: 进程ID
//返 回 值：进程句柄
////////////////////////////////
HANDLE MyOpenProcess( ULONG ulPid )
{
	NTSTATUS status;
	PEPROCESS pEprocess = NULL;
	HANDLE hProcess = NULL;
	UNICODE_STRING Unicode;

	status = PsLookupProcessByProcessId( (HANDLE)ulPid, &pEprocess );
	if ( NT_SUCCESS(status) )	//判断进程号是否存在
	{
		RtlInitUnicodeString(&Unicode, L"PsProcessType");
		//获取系统导出函数的地址
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
				//减少指针计数
				ObfDereferenceObject(pEprocess);
				return hProcess;
			}
		}

		ObfDereferenceObject(pEprocess);
	}

	return 0;
}
*/