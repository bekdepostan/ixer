/*===================================================================
 * Filename ssdt.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 系统服务分派表
 *
 * Date:   2013-5-12 Original from Chinghoi
 *
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "proc.h"
#include "drvMod.h"
#include "ssdt.h"
#include "common.h"
#include "pe.h"


/*========================================================================
*
* 函数名:	GetSSDTNumber
*
* 参数:		
*
* 功能描述:	获取服务个数
*
* 返回值:	ULONG
*
=========================================================================*/
ULONG
GetSSDTNumber( VOID )
{
	return KeServiceDescriptorTable->ntoskrnl.ServiceLimit;
}

/*========================================================================
*
* 函数名:	GetSSDTinfo
*
* 参数:
* PSSDT_INFO	[IN] - pSsdtInfo	指向输出缓冲区的PSSDT_INFO类型指针
*
* 功能描述:	获取SSDT信息
*
* 返回值:	VOID
*
=========================================================================*/
VOID
GetSSDTinfo( PSSDT_INFO pSsdtInfo )
{
	ULONG					index;
	ANSI_STRING				AnsiString;
	UNICODE_STRING			unico;
	ULONG					i;
	ULONG				ulDrvNums=0;
	PDRVMOD_ITEM		ptagDrvItem;
	PDRVMOD_ITEM		pKrnlInfo;

	//枚举驱动信息
	ptagDrvItem=GetKrnlModuleList(&ulDrvNums);
	//保存ntoskrnl**.exe模块信息
	pKrnlInfo = &ptagDrvItem[0];
	//
	for (index = 0; index < KeServiceDescriptorTable->ntoskrnl.ServiceLimit; index++)
	{
		/* 填充当前ssdt函数地址 */
		pSsdtInfo[index].currentAddr = (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable[index];

		/* 查找函数地址所在模块 */
		for (i=0; i<ulDrvNums; i++)
		{
			/* 查找ssdt函数当前所在的内核模块 */
			if ( CompareModule( (PVOID)pSsdtInfo[index].currentAddr, &ptagDrvItem[i]) )
			{
				RtlInitUnicodeString(&unico, ptagDrvItem[i].pwszDrvPath);
				RtlUnicodeStringToAnsiString(&AnsiString, &unico, TRUE);

				/* 填充当前ssdt函数所在模块路径 */
				strcpy( pSsdtInfo[index].imagePath, AnsiString.Buffer );
				RtlFreeAnsiString( &AnsiString );
				break;
			}
		}
	}

	/* 
	 * 从 ntoskrnl**.exe内核文件 中查找原始 SSDT 表 
	 * 并填充 pSsdtInfo.Addr域
	 */
	GetSSDTAddrByPhyFile( pKrnlInfo, pSsdtInfo );

	/* 
	 * 从 ntdll 中查找函数名称
	 * 并填充 pSsdtInfo
	 */
	GetSSDTNameByPhyFile( pSsdtInfo );
}

/*========================================================================
*
* 函数名:	CompareModule
*
* 参数:
*			PVOID					[IN]	Base	 - 函数地址
*			PDRVMOD_ITEM			[IN]	pDrvItem - 指向PDRVMOD_ITEM类型指针
*
* 功能描述:	比较函数是否在此模块内
*
* 返回值:
*			BOOLEAN					[OUT]	TRUE	- 在此模块中
*									[OUT]	FALSE	- 不在此模块中
*
=========================================================================*/
BOOLEAN CompareModule( PVOID Base, PDRVMOD_ITEM pDrvItem )
{
	//模块基址<=  Base  <= 模块基址+SizeOfImage
	if (pDrvItem->ulBaseAddr <= (ULONG)Base && (ULONG)Base <= pDrvItem->ulBaseAddr + pDrvItem->ulImageSize)
	{
		DbgPrint("CompareModule( 函数地址:0x%08x, 所属模块:%ws )\n", Base, pDrvItem->pwszDrvName);
		return TRUE;
	}

	return FALSE;
}

/*========================================================================
*
* 函数名:	GetSSDTAddrByPhyFile
*
* 参数:		PDRVMOD_ITEM	[IN]	- pDrvItem	指向PDRVMOD_ITEM类型指针，用于描述ntoskrnl**.exe内核文件信息
*			PSSDT_INFO		[I/0]	- pSsdtInfo 指向PSSDT_INFO类型指针，用于保存返回的ssdt信息
*
* 功能描述:	从 内核文件ntoskrnl**.exe 中查找原始 SSDT 表,并填充 pSsdtInfo.Addr域
*
* 返回值:	VOID
*
=========================================================================*/
VOID
GetSSDTAddrByPhyFile( PDRVMOD_ITEM pDrvItem, PSSDT_INFO pSsdtInfo )
{
	NTSTATUS					Status;
	HANDLE						FileHandle;
	ULONG						KernelBase;
	ULONG						KernelSize;
	ULONG						FileSize;
	PULONG						FileServiceTable;
	PVOID						Buffer;
	IO_STATUS_BLOCK				ioStatus;
	FILE_STANDARD_INFORMATION	FileInformation;
	WCHAR						KernelName[256] = L"\0";
	WCHAR						KernelPath[256] = L"\0";
	LARGE_INTEGER				byteOffset;
	ULONG						ImageBase;
	ULONG						index;



	/* 内核ntoskrnl**.exe 基地址 */
	KernelBase = pDrvItem->ulBaseAddr;
	/* 内核ntoskrnl**.exe 尺寸 */
	KernelSize = pDrvItem->ulImageSize;

	if (pDrvItem == NULL)
	{
		DbgPrint( "$ixer:\tKernelInfo == NULL\n" );
		return;
	}

	if (!KernelBase || !KernelSize)
	{
		DbgPrint( "$ixer:\tntoskrnl**.exe 基址or尺寸小于0\n" );
		return;
	}


	/* 判断 SSDT表 是否被人移位 */
	if ( (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable < KernelBase
		|| (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable > KernelBase + KernelSize)
	{
		DbgPrint( "$ixer:\t ssdt表不在ntoskrnl**.exe范围!\n");
//		return;
	}

	/* 获取内核文件路径 */
	swprintf( KernelPath, L"\\SystemRoot\\system32\\%s", pDrvItem->pwszDrvName );
	DbgPrint( "$ixer:\tKernelPath:\t%S", KernelPath );

	/* 打开内核文件 */
	Status = IxCreateFile( &FileHandle, KernelPath, FILE_READ_ATTRIBUTES | SYNCHRONIZE, FILE_SHARE_READ );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwOpenFile failed！status:0x%08x\n", Status );
	}

	/* 获取文件信息 */
	Status = ZwQueryInformationFile( FileHandle, &ioStatus, &FileInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwQueryInformationFile failed！status:0x%08x\n", Status );

		ZwClose( FileHandle );
		return;
	}

	/* 文件尺寸过大 */
	if (FileInformation.EndOfFile.HighPart != 0 )
	{
		DbgPrint( "$ixer:\tFile Size Too High\n" );

		ZwClose( FileHandle );
		return;
	}

	FileSize = FileInformation.EndOfFile.LowPart;

	Buffer = ExAllocatePool( NonPagedPool, FileSize );
	if (Buffer == NULL)
	{
		ZwClose( FileHandle );
		return;
	}

	/* 从头部读取文件 */
	byteOffset.LowPart = 0;
	byteOffset.HighPart = 0;

	Status = ZwReadFile(
						FileHandle, 
						NULL,
						NULL,
						NULL,
						&ioStatus,
						Buffer,
						FileSize,
						&byteOffset,
						NULL );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwReadFile:\t0x%08x", Status );

		ZwClose( FileHandle );
		return;
	}

/*
	/* 根据内存中的 SSDT 偏移，获取 SSDT 的文件偏移 /
	FileServiceTableOffset = RVAToFileOffset( Buffer, (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable - KernelBase );
	FileServiceTable = (PULONG)((ULONG)Buffer + FileServiceTableOffset);
*/
	//获取愿望加载基址
	ImageBase = GetImageBase( Buffer );
	if (ImageBase == 0)
	{
		DbgPrint( "$ixer:\tImageBase == 0" );

		ZwClose( FileHandle );

		return;
	}

	//在加载的ntoskrnl**.exe内存中查找ssdt表原始地址
	FileServiceTable = FindRealSSDTaddr( Buffer );
	if (FileServiceTable != NULL)
	{
		for (index = 0; index < KeServiceDescriptorTable->ntoskrnl.ServiceLimit; index++)
		{
			/* 检测原始ssdt函数地址有效性 */
			if ( !IsValidAddr( ((ULONG)FileServiceTable[index] - ImageBase + KernelBase)) )
			{
				break;
			}

			/* 保存ssdt服务函数原始地址 */
			pSsdtInfo[index].nativeAddr = (ULONG)FileServiceTable[index] - ImageBase + KernelBase;	//重定位

		}
	}

	ExFreePool( Buffer );
	ZwClose( FileHandle );
}

/*获取ssdt表原始地址*/
PVOID FindRealSSDTaddr( PVOID BaseAddress )
{
	PUCHAR						realaddr;
	PIMAGE_EXPORT_DIRECTORY		pExportDirectory;
	PULONG						AddressOfFunctions;
	PULONG						AddressOfNames;
	PUSHORT						AddressOfNameOrdinals;
	ULONG						Index;
	LPSTR						ProcName;
	ULONG						NameOffset;
	PVOID						FuncAddr;
	ULONG						ServerIndex1;
	ULONG						ServerIndex2;
	ULONG						ServerAddr1;
	ULONG						ServerAddr2;
	ULONG						ImageBase;
	ULONG						FoundNum;
	PULONG						systemtable;

	realaddr = BaseAddress;
	FoundNum = 0;

	ImageBase = GetImageBase( BaseAddress );
	if (ImageBase == 0)
	{
		DbgPrint( "$ixer:\tImageBase == 0" );
		return NULL;
	}

	/* 导出表地址 */
	pExportDirectory = (PVOID)((ULONG)BaseAddress + GetDirectoryEntry( BaseAddress, IMAGE_DIRECTORY_ENTRY_EXPORT, TRUE ));
	AddressOfFunctions = (PULONG)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, pExportDirectory->AddressOfFunctions ));
	AddressOfNameOrdinals = (PUSHORT)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, pExportDirectory->AddressOfNameOrdinals ));
	AddressOfNames = (PULONG)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, pExportDirectory->AddressOfNames ));

	/* 分析导出表 */
	for( Index = 0; Index < pExportDirectory->NumberOfNames; Index++, AddressOfNames++, AddressOfNameOrdinals++ )
	{
		NameOffset = RVAToFileOffset( BaseAddress, *AddressOfNames );
		ProcName = (LPSTR)((ULONG)BaseAddress + NameOffset);

		/* 查找所有 ZwReadFile 函数 */
		if (strcmp( ProcName, "ZwReadFile" ) == 0)
		{
			FuncAddr = (PVOID)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, (ULONG)AddressOfFunctions[*AddressOfNameOrdinals] ));

			/* 函数序号 */
			ServerIndex1 = *(PULONG)((ULONG)FuncAddr + 1);

			DbgPrint( "ixer:\tZwReadFile:\t0x%x", ServerIndex1 );
			FoundNum++;
		}
		else if (strcmp( ProcName, "NtReadFile" ) == 0)
		{
			ServerAddr1 = (ULONG)ImageBase + (ULONG)AddressOfFunctions[*AddressOfNameOrdinals];

			DbgPrint( "ixer:\tNtReadFile:\t0x%08x", ServerAddr1 );
			FoundNum++;
		}
		else if (strcmp( ProcName, "ZwClose" ) == 0)
		{
			FuncAddr = (PVOID)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, (ULONG)AddressOfFunctions[*AddressOfNameOrdinals] ));

			/* 函数序号 */
			ServerIndex2 = *(PULONG)((ULONG)FuncAddr + 1);

			DbgPrint( "ixer:\tZwClose:\t0x%x", ServerIndex2 );
			FoundNum++;
		}
		else if (strcmp( ProcName, "NtClose" ) == 0)
		{
			ServerAddr2 = (ULONG)ImageBase + (ULONG)AddressOfFunctions[*AddressOfNameOrdinals];

			DbgPrint( "ixer:\tNtClose:\t0x%08x", ServerAddr2 );
			FoundNum++;
		}
	}

	if (FoundNum != 4)
	{
		DbgPrint( "$ixer:\tFoundNum:\t%d", FoundNum );
		return NULL;
	}

	while (TRUE)
	{
		if (IsValidAddr( *realaddr ))
		{
			systemtable = (PULONG)realaddr;
			if (IsValidAddr( systemtable[ServerIndex1] ) && IsValidAddr( systemtable[ServerIndex2] ))
			{
				if ((systemtable[ServerIndex1] == ServerAddr1) && (systemtable[ServerIndex2] == ServerAddr2))
				{
					DbgPrint("ssdt RealAddr:0x%08x\n",realaddr);
					break;
				}
			}

		}

		realaddr++;
	}

	return realaddr;
}

/*========================================================================
*
* 函数名:	GetSSDTNameByPhyFile
*
* 参数:		PSSDT_RECORD	[IN] - pSsdtInfo
*
* 功能描述:	从 ntdll 中查找函数名称,并填充 pSsdtInfo
*
* 返回值:	VOID
*
=========================================================================*/
VOID
GetSSDTNameByPhyFile( PSSDT_INFO pSsdtInfo )
{
	NTSTATUS					Status;
	HANDLE						FileHandle;
	ULONG						FileSize;
	PVOID						Buffer;
	IO_STATUS_BLOCK				ioStatus;
	FILE_STANDARD_INFORMATION	FileInformation;
	LARGE_INTEGER				byteOffset;
	PIMAGE_EXPORT_DIRECTORY		pExportDirectory;
	PULONG						AddressOfFunctions;
	PULONG						AddressOfNames;
	PUSHORT						AddressOfNameOrdinals;
	ULONG						Index;
	ULONG						NameOffset;
	LPSTR						funName;
	PVOID						FuncAddr;
	ULONG						ServerIndex;



	/* 打开 ntdll 文件 */
	Status = IxCreateFile( &FileHandle, L"\\SystemRoot\\system32\\ntdll.dll", FILE_READ_ATTRIBUTES | SYNCHRONIZE, FILE_SHARE_READ );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwOpenFile failed！status:0x%08x\n", Status );
	}

	/* 获取文件信息 */
	Status = ZwQueryInformationFile( FileHandle, &ioStatus, &FileInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwQueryInformationFile failed！status:0x%08x\n", Status );

		ZwClose( FileHandle );
		return;
	}

	/* 文件尺寸过大 */
	if (FileInformation.EndOfFile.HighPart != 0 )
	{
		DbgPrint( "$ixer:\tFile Size Too High" );

		ZwClose( FileHandle );
		return;
	}

	FileSize = FileInformation.EndOfFile.LowPart;

	Buffer = ExAllocatePool( PagedPool, FileSize );
	if (Buffer == NULL)
	{
		DbgPrint( "$ixer:\tExAllocatePool() == NULL" );

		ZwClose( FileHandle );
		return;
	}

	/* 从头读取文件 */
	byteOffset.LowPart = 0;
	byteOffset.HighPart = 0;

	Status = ZwReadFile(
					FileHandle, 
					NULL,
					NULL,
					NULL,
					&ioStatus,
					Buffer,
					FileSize,
					&byteOffset,
					NULL );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwReadFile failed！status:0x%08x\n", Status );

		ZwClose( FileHandle );
		return;
	}

	/* 导出表地址 */
	pExportDirectory = (PVOID)((ULONG)Buffer + GetDirectoryEntry( Buffer, IMAGE_DIRECTORY_ENTRY_EXPORT, TRUE ));
	AddressOfFunctions = (PULONG)((ULONG)Buffer + RVAToFileOffset( Buffer, pExportDirectory->AddressOfFunctions ));
	AddressOfNameOrdinals = (PUSHORT)((ULONG)Buffer + RVAToFileOffset( Buffer, pExportDirectory->AddressOfNameOrdinals ));
	AddressOfNames = (PULONG)((ULONG)Buffer + RVAToFileOffset( Buffer, pExportDirectory->AddressOfNames ));

	/* 分析导出表 */
	for( Index = 0; Index < pExportDirectory->NumberOfNames; Index++, AddressOfNames++, AddressOfNameOrdinals++ )
	{
		NameOffset = RVAToFileOffset( Buffer, *AddressOfNames );
		funName = (LPSTR)((ULONG)Buffer + NameOffset);

		/* 查找所有 Zw* 函数 */
		if (funName[0] == 'Z' && funName[1] == 'w')
		{
			FuncAddr = (PVOID)((ULONG)Buffer + RVAToFileOffset( Buffer, (ULONG)AddressOfFunctions[*AddressOfNameOrdinals] ));

			/* 函数序号 */
			ServerIndex = *(PULONG)((ULONG)FuncAddr + 1);

			/* 复制函数名称 */
			funName[0] = 'N';
			funName[1] = 't';
			strcpy(pSsdtInfo[ServerIndex].funName, funName );
		}
	}

	ExFreePool( Buffer );
	ZwClose( FileHandle );
}

///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：恢复SSDT表中指定序数的服务函数的地址
//	输入参数：pInfo为SSDTINFO类型的指针，里面是要恢复的服务函数序数和地址
//	输出参数：返回TRUE表示恢复成功，否则返回FALSE失败
//
///////////////////////////////////////////////////////////////////////////////////
BOOLEAN
RestoreSSDTItem(IN PSSDTINFO	pInfo)
{
	//KdPrint(("[RestoreSSDTItem] 恢复序数为%ld服务函数地址!",pInfo->Index));

	if(!pInfo)
	{
		//KdPrint(("[RestoreSSDTItem] 输入指针无效!"));
		return FALSE;
	}

	if(0 != SetSSDTServiceAddr(pInfo->index, pInfo->funAddr))
	{
		//KdPrint(("[RestoreSSDTItem] 恢复指定服务函数地址成功!"));
		return TRUE;
	}
	else
	{
		//KdPrint(("[RestoreSSDTItem] 恢复指定服务函数地址失败!"));
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：修改SSDT表中指定的服务函数地址,可以用于SSDT表服务函数HOOK
//	输入参数：Index为要修改的服务函数在SSDT表中的序数；
//			  NewServiceAdd为要修改的服务函数的新地址；
//	输出参数：返回指定的被修改服务函数的原始地址
//
///////////////////////////////////////////////////////////////////////////////////
ULONG
SetSSDTServiceAddr(IN unsigned int Index,IN ULONG	NewServiceAdd)
{
	PMDL	pmdlPointer=NULL;
	KIRQL	oldirql;
	PVOID	*MappedPointer=NULL;
	PULONG	EnableAdd=NULL;
	ULONG	OldFunServiceAddr=0;

	//KdPrint(("[SetSSDTServiceAddr] 设置SSDT地址\n"));

	pmdlPointer=MmCreateMdl(NULL, KeServiceDescriptorTable->ntoskrnl.ServiceTable, KeServiceDescriptorTable->ntoskrnl.ServiceLimit*sizeof(ULONG));
	if(!pmdlPointer)
	{
		//KdPrint(("[SetSSDTServiceAddr] 创建MDL块失败!\n"));
		return OldFunServiceAddr;
	}
	MmBuildMdlForNonPagedPool(pmdlPointer);
	pmdlPointer->MdlFlags = pmdlPointer->MdlFlags | MDL_MAPPED_TO_SYSTEM_VA;
	MappedPointer= MmMapLockedPages(pmdlPointer, KernelMode);
	EnableAdd=(PULONG)MappedPointer;
	KeRaiseIrql(DISPATCH_LEVEL, &oldirql);

	OldFunServiceAddr=EnableAdd[Index];
	EnableAdd[Index]=NewServiceAdd;

	KeLowerIrql(oldirql);
	MmUnmapLockedPages(MappedPointer,pmdlPointer);
	IoFreeMdl(pmdlPointer);

	//KdPrint(("[SetSSDTServiceAddr] 修改SSDT表中的服务函数地址成功!"));
	return OldFunServiceAddr;
}