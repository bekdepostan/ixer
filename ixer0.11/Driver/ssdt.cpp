/*===================================================================
 * Filename ssdt.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: ϵͳ������ɱ�
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
* ������:	GetSSDTNumber
*
* ����:		
*
* ��������:	��ȡ�������
*
* ����ֵ:	ULONG
*
=========================================================================*/
ULONG
GetSSDTNumber( VOID )
{
	return KeServiceDescriptorTable->ntoskrnl.ServiceLimit;
}

/*========================================================================
*
* ������:	GetSSDTinfo
*
* ����:
* PSSDT_INFO	[IN] - pSsdtInfo	ָ�������������PSSDT_INFO����ָ��
*
* ��������:	��ȡSSDT��Ϣ
*
* ����ֵ:	VOID
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

	//ö��������Ϣ
	ptagDrvItem=GetKrnlModuleList(&ulDrvNums);
	//����ntoskrnl**.exeģ����Ϣ
	pKrnlInfo = &ptagDrvItem[0];
	//
	for (index = 0; index < KeServiceDescriptorTable->ntoskrnl.ServiceLimit; index++)
	{
		/* ��䵱ǰssdt������ַ */
		pSsdtInfo[index].currentAddr = (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable[index];

		/* ���Һ�����ַ����ģ�� */
		for (i=0; i<ulDrvNums; i++)
		{
			/* ����ssdt������ǰ���ڵ��ں�ģ�� */
			if ( CompareModule( (PVOID)pSsdtInfo[index].currentAddr, &ptagDrvItem[i]) )
			{
				RtlInitUnicodeString(&unico, ptagDrvItem[i].pwszDrvPath);
				RtlUnicodeStringToAnsiString(&AnsiString, &unico, TRUE);

				/* ��䵱ǰssdt��������ģ��·�� */
				strcpy( pSsdtInfo[index].imagePath, AnsiString.Buffer );
				RtlFreeAnsiString( &AnsiString );
				break;
			}
		}
	}

	/* 
	 * �� ntoskrnl**.exe�ں��ļ� �в���ԭʼ SSDT �� 
	 * ����� pSsdtInfo.Addr��
	 */
	GetSSDTAddrByPhyFile( pKrnlInfo, pSsdtInfo );

	/* 
	 * �� ntdll �в��Һ�������
	 * ����� pSsdtInfo
	 */
	GetSSDTNameByPhyFile( pSsdtInfo );
}

/*========================================================================
*
* ������:	CompareModule
*
* ����:
*			PVOID					[IN]	Base	 - ������ַ
*			PDRVMOD_ITEM			[IN]	pDrvItem - ָ��PDRVMOD_ITEM����ָ��
*
* ��������:	�ȽϺ����Ƿ��ڴ�ģ����
*
* ����ֵ:
*			BOOLEAN					[OUT]	TRUE	- �ڴ�ģ����
*									[OUT]	FALSE	- ���ڴ�ģ����
*
=========================================================================*/
BOOLEAN CompareModule( PVOID Base, PDRVMOD_ITEM pDrvItem )
{
	//ģ���ַ<=  Base  <= ģ���ַ+SizeOfImage
	if (pDrvItem->ulBaseAddr <= (ULONG)Base && (ULONG)Base <= pDrvItem->ulBaseAddr + pDrvItem->ulImageSize)
	{
		DbgPrint("CompareModule( ������ַ:0x%08x, ����ģ��:%ws )\n", Base, pDrvItem->pwszDrvName);
		return TRUE;
	}

	return FALSE;
}

/*========================================================================
*
* ������:	GetSSDTAddrByPhyFile
*
* ����:		PDRVMOD_ITEM	[IN]	- pDrvItem	ָ��PDRVMOD_ITEM����ָ�룬��������ntoskrnl**.exe�ں��ļ���Ϣ
*			PSSDT_INFO		[I/0]	- pSsdtInfo ָ��PSSDT_INFO����ָ�룬���ڱ��淵�ص�ssdt��Ϣ
*
* ��������:	�� �ں��ļ�ntoskrnl**.exe �в���ԭʼ SSDT ��,����� pSsdtInfo.Addr��
*
* ����ֵ:	VOID
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



	/* �ں�ntoskrnl**.exe ����ַ */
	KernelBase = pDrvItem->ulBaseAddr;
	/* �ں�ntoskrnl**.exe �ߴ� */
	KernelSize = pDrvItem->ulImageSize;

	if (pDrvItem == NULL)
	{
		DbgPrint( "$ixer:\tKernelInfo == NULL\n" );
		return;
	}

	if (!KernelBase || !KernelSize)
	{
		DbgPrint( "$ixer:\tntoskrnl**.exe ��ַor�ߴ�С��0\n" );
		return;
	}


	/* �ж� SSDT�� �Ƿ�����λ */
	if ( (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable < KernelBase
		|| (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable > KernelBase + KernelSize)
	{
		DbgPrint( "$ixer:\t ssdt����ntoskrnl**.exe��Χ!\n");
//		return;
	}

	/* ��ȡ�ں��ļ�·�� */
	swprintf( KernelPath, L"\\SystemRoot\\system32\\%s", pDrvItem->pwszDrvName );
	DbgPrint( "$ixer:\tKernelPath:\t%S", KernelPath );

	/* ���ں��ļ� */
	Status = IxCreateFile( &FileHandle, KernelPath, FILE_READ_ATTRIBUTES | SYNCHRONIZE, FILE_SHARE_READ );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwOpenFile failed��status:0x%08x\n", Status );
	}

	/* ��ȡ�ļ���Ϣ */
	Status = ZwQueryInformationFile( FileHandle, &ioStatus, &FileInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwQueryInformationFile failed��status:0x%08x\n", Status );

		ZwClose( FileHandle );
		return;
	}

	/* �ļ��ߴ���� */
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

	/* ��ͷ����ȡ�ļ� */
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
	/* �����ڴ��е� SSDT ƫ�ƣ���ȡ SSDT ���ļ�ƫ�� /
	FileServiceTableOffset = RVAToFileOffset( Buffer, (ULONG)KeServiceDescriptorTable->ntoskrnl.ServiceTable - KernelBase );
	FileServiceTable = (PULONG)((ULONG)Buffer + FileServiceTableOffset);
*/
	//��ȡԸ�����ػ�ַ
	ImageBase = GetImageBase( Buffer );
	if (ImageBase == 0)
	{
		DbgPrint( "$ixer:\tImageBase == 0" );

		ZwClose( FileHandle );

		return;
	}

	//�ڼ��ص�ntoskrnl**.exe�ڴ��в���ssdt��ԭʼ��ַ
	FileServiceTable = FindRealSSDTaddr( Buffer );
	if (FileServiceTable != NULL)
	{
		for (index = 0; index < KeServiceDescriptorTable->ntoskrnl.ServiceLimit; index++)
		{
			/* ���ԭʼssdt������ַ��Ч�� */
			if ( !IsValidAddr( ((ULONG)FileServiceTable[index] - ImageBase + KernelBase)) )
			{
				break;
			}

			/* ����ssdt������ԭʼ��ַ */
			pSsdtInfo[index].nativeAddr = (ULONG)FileServiceTable[index] - ImageBase + KernelBase;	//�ض�λ

		}
	}

	ExFreePool( Buffer );
	ZwClose( FileHandle );
}

/*��ȡssdt��ԭʼ��ַ*/
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

	/* �������ַ */
	pExportDirectory = (PVOID)((ULONG)BaseAddress + GetDirectoryEntry( BaseAddress, IMAGE_DIRECTORY_ENTRY_EXPORT, TRUE ));
	AddressOfFunctions = (PULONG)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, pExportDirectory->AddressOfFunctions ));
	AddressOfNameOrdinals = (PUSHORT)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, pExportDirectory->AddressOfNameOrdinals ));
	AddressOfNames = (PULONG)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, pExportDirectory->AddressOfNames ));

	/* ���������� */
	for( Index = 0; Index < pExportDirectory->NumberOfNames; Index++, AddressOfNames++, AddressOfNameOrdinals++ )
	{
		NameOffset = RVAToFileOffset( BaseAddress, *AddressOfNames );
		ProcName = (LPSTR)((ULONG)BaseAddress + NameOffset);

		/* �������� ZwReadFile ���� */
		if (strcmp( ProcName, "ZwReadFile" ) == 0)
		{
			FuncAddr = (PVOID)((ULONG)BaseAddress + RVAToFileOffset( BaseAddress, (ULONG)AddressOfFunctions[*AddressOfNameOrdinals] ));

			/* ������� */
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

			/* ������� */
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
* ������:	GetSSDTNameByPhyFile
*
* ����:		PSSDT_RECORD	[IN] - pSsdtInfo
*
* ��������:	�� ntdll �в��Һ�������,����� pSsdtInfo
*
* ����ֵ:	VOID
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



	/* �� ntdll �ļ� */
	Status = IxCreateFile( &FileHandle, L"\\SystemRoot\\system32\\ntdll.dll", FILE_READ_ATTRIBUTES | SYNCHRONIZE, FILE_SHARE_READ );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwOpenFile failed��status:0x%08x\n", Status );
	}

	/* ��ȡ�ļ���Ϣ */
	Status = ZwQueryInformationFile( FileHandle, &ioStatus, &FileInformation, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation );
	if (!NT_SUCCESS(Status))
	{
		DbgPrint( "$ixer:\tZwQueryInformationFile failed��status:0x%08x\n", Status );

		ZwClose( FileHandle );
		return;
	}

	/* �ļ��ߴ���� */
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

	/* ��ͷ��ȡ�ļ� */
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
		DbgPrint( "$ixer:\tZwReadFile failed��status:0x%08x\n", Status );

		ZwClose( FileHandle );
		return;
	}

	/* �������ַ */
	pExportDirectory = (PVOID)((ULONG)Buffer + GetDirectoryEntry( Buffer, IMAGE_DIRECTORY_ENTRY_EXPORT, TRUE ));
	AddressOfFunctions = (PULONG)((ULONG)Buffer + RVAToFileOffset( Buffer, pExportDirectory->AddressOfFunctions ));
	AddressOfNameOrdinals = (PUSHORT)((ULONG)Buffer + RVAToFileOffset( Buffer, pExportDirectory->AddressOfNameOrdinals ));
	AddressOfNames = (PULONG)((ULONG)Buffer + RVAToFileOffset( Buffer, pExportDirectory->AddressOfNames ));

	/* ���������� */
	for( Index = 0; Index < pExportDirectory->NumberOfNames; Index++, AddressOfNames++, AddressOfNameOrdinals++ )
	{
		NameOffset = RVAToFileOffset( Buffer, *AddressOfNames );
		funName = (LPSTR)((ULONG)Buffer + NameOffset);

		/* �������� Zw* ���� */
		if (funName[0] == 'Z' && funName[1] == 'w')
		{
			FuncAddr = (PVOID)((ULONG)Buffer + RVAToFileOffset( Buffer, (ULONG)AddressOfFunctions[*AddressOfNameOrdinals] ));

			/* ������� */
			ServerIndex = *(PULONG)((ULONG)FuncAddr + 1);

			/* ���ƺ������� */
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
//	����ʵ�֣��ָ�SSDT����ָ�������ķ������ĵ�ַ
//	���������pInfoΪSSDTINFO���͵�ָ�룬������Ҫ�ָ��ķ����������͵�ַ
//	�������������TRUE��ʾ�ָ��ɹ������򷵻�FALSEʧ��
//
///////////////////////////////////////////////////////////////////////////////////
BOOLEAN
RestoreSSDTItem(IN PSSDTINFO	pInfo)
{
	//KdPrint(("[RestoreSSDTItem] �ָ�����Ϊ%ld��������ַ!",pInfo->Index));

	if(!pInfo)
	{
		//KdPrint(("[RestoreSSDTItem] ����ָ����Ч!"));
		return FALSE;
	}

	if(0 != SetSSDTServiceAddr(pInfo->index, pInfo->funAddr))
	{
		//KdPrint(("[RestoreSSDTItem] �ָ�ָ����������ַ�ɹ�!"));
		return TRUE;
	}
	else
	{
		//KdPrint(("[RestoreSSDTItem] �ָ�ָ����������ַʧ��!"));
		return FALSE;
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣��޸�SSDT����ָ���ķ�������ַ,��������SSDT�������HOOK
//	���������IndexΪҪ�޸ĵķ�������SSDT���е�������
//			  NewServiceAddΪҪ�޸ĵķ��������µ�ַ��
//	�������������ָ���ı��޸ķ�������ԭʼ��ַ
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

	//KdPrint(("[SetSSDTServiceAddr] ����SSDT��ַ\n"));

	pmdlPointer=MmCreateMdl(NULL, KeServiceDescriptorTable->ntoskrnl.ServiceTable, KeServiceDescriptorTable->ntoskrnl.ServiceLimit*sizeof(ULONG));
	if(!pmdlPointer)
	{
		//KdPrint(("[SetSSDTServiceAddr] ����MDL��ʧ��!\n"));
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

	//KdPrint(("[SetSSDTServiceAddr] �޸�SSDT���еķ�������ַ�ɹ�!"));
	return OldFunServiceAddr;
}