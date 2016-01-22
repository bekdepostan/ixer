//////////////////////////////////////////////////////////////////////////
// Dll.c

#include "precomp.h"
#include "ssdt.h"
#include "proc.h"
#include "WinOSVersion.h"
#include "func.h"
#include "Dll.h"



//获取Dll列表
PDLL_ITEM GetDllList(ULONG ulPid, PULONG numOfDll)
{
	NTSTATUS status=STATUS_SUCCESS;
	PDLL_ITEM pDllItem=NULL;
	ULONG i=0;

//	KdPrint(("进程Pid:%d \t Dll计数:%d\n", ulPid, *dllNum));
	//按dll个数申请内存
	pDllItem=(DLL_ITEM *)ExAllocatePoolWithTag(PagedPool, sizeof(DLL_ITEM)*MAX_Dll_NUM*2, 'dll');
	if (pDllItem)
	{
		//内存清零
		RtlZeroMemory(pDllItem, sizeof(DLL_ITEM)*MAX_Dll_NUM*2);
		//枚举Dll
		EnumDllByInLoadOrderLinks(ulPid, numOfDll, pDllItem);
		//检测dll
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

	/*获取目标进程EPROCESS*/
	status=PsLookupProcessByProcessId( (HANDLE)ulPid, &pEproc);
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(("Can't get target process eprocess! ErrorCode= %08X\n", status));
		return FALSE;
	}

	//EPROCESS为0或无效则返回
	if (pEproc == 0  ||  IsValidAddr( ( (ULONG)pEproc+g_EPROCESS_Peb )) != VALID_PAGE )
	{
		KdPrint(("Target process eprocess  =0 or invalid!\n"));
		return FALSE;
	}

	/*栈附加(切换到目标进程上下文)*/
		KeStackAttachProcess(pEproc, &pApcStatus);

	Peb = *(PULONG)( (ULONG)pEproc+g_EPROCESS_Peb );
	__try
	{
		//Peb是用户空间，可能会无法访问
		ProbeForRead( (PVOID)Peb, 4, 1);
		//刺探读！
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
// 					//dll计数
// 					(*numOfDll)++;

					KdPrint(("StoreOrCheckDll(%08X, %ws)\n", (ULONG)LdrEntry->DllBase, LdrEntry->FullDllName.Buffer));
					//存储枚举到的dll模块信息
					StoreOrCheckDll(TRUE, (ULONG)LdrEntry->DllBase, LdrEntry->SizeOfImage, LdrEntry->FullDllName.Buffer, pDllItem, numOfDll);
				}
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		KdPrint(("__except!\n"));

		//分离
		KeUnstackDetachProcess(&pApcStatus);
		return FALSE;
	}

	//分离
	KeUnstackDetachProcess(&pApcStatus);
	return TRUE;
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

	VolumeName.Buffer = NULL;
	VolumeName.Length = 0;
	VolumeName.MaximumLength = 0;

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
*			ULONG		[IN] index		  - 模块基址
*			ULONG		[IN] imageSize	  - 模块大小
*			PWCHAR		[IN] dosPath	  - 模块路径
*			PDLL_ITEM	[IN] pDllItem	  - 指向Dll结构体指针
*			PULONG		[IN] dllNum		  - dll计数
*
* 功能描述:	存储或检测Dll模块
*
* 返回值:	TRUE正常，FALSE为隐藏
*
=========================================================================*/
BOOLEAN StoreOrCheckDll(BOOLEAN	storeOrCheck, ULONG baseAddr, ULONG imageSize, PWCHAR dosPath, PDLL_ITEM pDllItem, PULONG numOfDll)
{
	ULONG	i=0;
	ULONG	h=0;

	//基址or大小都不能<=0
	if (baseAddr <= 0  ||  imageSize <= 0)
		return FALSE;

	//存储
	if (storeOrCheck)
	{
		//dll计数
		(*numOfDll)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			//找Eproc数组索引i指向的地址为空的地方存储
			if ( pDllItem[i].ulBaseAddr == 0)
			{
//				KdPrint(("StoreOrCheckDll(%08X, %ws)\n", pDllItem[i].ulBaseAddr, pDllItem[i].pwszPath));
				/*存储dll模块相关信息*/
				pDllItem[i].ulBaseAddr=baseAddr;		//基址
				pDllItem[i].ulImageSize=imageSize;		//模块大小
				wcscpy(pDllItem[i].pwszPath, dosPath);	//模块路径

				return TRUE;
			}
		}
	}
	else
	{
		//检测
		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			//已存则返回
			if (baseAddr == pDllItem[i].ulBaseAddr)
				return TRUE;
		}

		//发现隐藏，dll计数递增
		(*numOfDll)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			if (pDllItem[i].ulBaseAddr == 0)
			{
				/*加入到记录集*/
				//模块基址
				pDllItem[i].ulBaseAddr=baseAddr;
				//模块大小
				pDllItem[i].ulImageSize=imageSize;
				//模块路径
				wcscpy(pDllItem[i].pwszPath, dosPath);

				//指定存储隐藏dll
				for (h=0; h<MAX_Dll_NUM*2; h++)
				{
					if (pDllItem[i].ulBaseAddr == 0)
					{
						//模块基址
						pDllItem[h].ulBaseAddr=baseAddr;
						//模块大小
						pDllItem[h].ulImageSize=imageSize;
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