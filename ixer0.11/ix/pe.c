/*===================================================================
 * Filename: pe.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description:  PE�ļ�����
 *
 * Date:   2013-5-13 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "pe.h"


/*========================================================================
*
* ������:	GetDirectoryEntry
*
* ����:		PVOID	[IN]	ImageBase		- �ļ�����ַ
*			DWORD	[IN]	DirectoryIndex	- Ŀ¼�����
*			BOOLEAN	[IN]	bFile			- ��ȡ�����ļ����廹���ڴ�ӳ��
*											  TRUE	�ļ�
*											  FALSE	�ڴ�ӳ��
*
* ��������:	���� PE ͷ����PE Ŀ¼��ƫ��
*
* ����ֵ:	ULONG
*
=========================================================================*/
ULONG
GetDirectoryEntry( PVOID ImageBase, DWORD DirectoryIndex, BOOLEAN bFile )
{
	ULONG					Offset;
	PIMAGE_DOS_HEADER		pDosHeader;
	PIMAGE_NT_HEADERS		pNtHeaders;
	PIMAGE_EXPORT_DIRECTORY	pExportDirectory;

	#ifdef _CHECKED
		DbgPrint( "ixer:\tGetDirectoryEntry( 0x%08x, 0x%x, %d )", ImageBase, DirectoryIndex, bFile );
	#endif


	pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	pNtHeaders = (PIMAGE_NT_HEADERS)((LONG)ImageBase + pDosHeader->e_lfanew );

	if ( pNtHeaders->OptionalHeader.DataDirectory[DirectoryIndex].VirtualAddress == 0 )
	{
		#ifdef _CHECKED
			DbgPrint( "ixer:\tVirtualAddress == 0" );
		#endif

		return 0;
	}

	Offset = pNtHeaders->OptionalHeader.DataDirectory[DirectoryIndex].VirtualAddress;
	if (bFile)
	{
		Offset = RVAToFileOffset( ImageBase, Offset );
	}

	#ifdef _CHECKED
		DbgPrint( "ixer:\tGetDirectoryEntry() end:\t0x%x", Offset );
	#endif

	return Offset;
}


/*========================================================================
*
* ������:	RVAToFileOffset
*
* ����:		PVOID	[IN]	ImageBase	- �����ַ
*			ULONG	[IN]	Rva			- RVAֵ
*
* ��������:	RVA ת��Ϊ �ļ�ƫ��
*
* ����ֵ:	ULONG
*
=========================================================================*/
ULONG
RVAToFileOffset( PVOID ImageBase, ULONG Rva )
{
	PIMAGE_DOS_HEADER		pDosHeader;
	PIMAGE_NT_HEADERS		pNtHeaders;
	PIMAGE_SECTION_HEADER	pSectionHeader;
	ULONG					FileOffset;
	WORD					Index;

	#ifdef _CHECKED
		DbgPrint( "ixer:\tRVAToFileOffset( 0x%08x, 0x%x )", ImageBase, Rva );
	#endif

	FileOffset = Rva;

	pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	pNtHeaders = (PIMAGE_NT_HEADERS)((LONG)ImageBase + pDosHeader->e_lfanew );

	pSectionHeader = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeaders + sizeof(IMAGE_NT_HEADERS));

	/* �����ڱ����жԱ�ת�� */
	for ( Index = 0; Index < pNtHeaders->FileHeader.NumberOfSections; Index++, pSectionHeader++)
	{
		if (pSectionHeader->VirtualAddress <= Rva
			&& Rva <= pSectionHeader->VirtualAddress + pSectionHeader->SizeOfRawData)
		{
			FileOffset = Rva - pSectionHeader->VirtualAddress + pSectionHeader->PointerToRawData;
		}
	}

	#ifdef _CHECKED
		DbgPrint( "ixer:\tRVAToFileOffset() end:\t0x%x", FileOffset );
	#endif

	return FileOffset;
}


/*========================================================================
*
* ������:	FindProcAddressFromExport
*
* ����:		PVOID	[IN]	ImageBase	- �ļ�����ַ
*			PVOID	[IN]	Export		- �������ַ������ƫ�ƣ�
*			LPSTR	[IN]	ProcName	- ��������
*			BOOLEAN	[IN]	bFile		- ��ȡ�����ļ����廹���ڴ�ӳ��
*										  TRUE	�ļ�
*										  FALSE	�����ļ�
*
* ��������:	�� ������ ��Ѱ��ָ��������ƫ��
*
* ����ֵ:	ULONG
*
=========================================================================*/
ULONG
FindProcAddressFromExport( PVOID ImageBase, PVOID Export, LPSTR ProcName, BOOLEAN bFile )
{
	ULONG					Index;
	ULONG					NameOffset;
	ULONG					FunAddr;
	PULONG					AddressOfFunctions;
	PULONG					AddressOfNames;
	PUSHORT					AddressOfNameOrdinals;
	PIMAGE_EXPORT_DIRECTORY	pExportDirectory;


	#ifdef _CHECKED
		DbgPrint( "ixer:\tFindProcAddressFromExport( 0x%08x, 0x%08x, %s, %d )", ImageBase, Export, ProcName, bFile );
	#endif

	pExportDirectory = Export;
	FunAddr = 0;

	if (bFile)
	{
		AddressOfFunctions = (PULONG)((ULONG)ImageBase + RVAToFileOffset( ImageBase, pExportDirectory->AddressOfFunctions ));
		AddressOfNameOrdinals = (PUSHORT)((ULONG)ImageBase + RVAToFileOffset( ImageBase, pExportDirectory->AddressOfNameOrdinals ));
		AddressOfNames = (PULONG)((ULONG)ImageBase + RVAToFileOffset( ImageBase, pExportDirectory->AddressOfNames ));
	}
	else
	{
		AddressOfFunctions = (PULONG)((DWORD)ImageBase + pExportDirectory->AddressOfFunctions);
		AddressOfNameOrdinals = (PUSHORT)((ULONG)ImageBase + pExportDirectory->AddressOfNameOrdinals);
		AddressOfNames = (PULONG)((ULONG)ImageBase + pExportDirectory->AddressOfNames);
	}

	for( Index = 0; Index < pExportDirectory->NumberOfNames; Index++, AddressOfNames++, AddressOfNameOrdinals++ )
	{
		if (bFile)
		{
			NameOffset = RVAToFileOffset( ImageBase, *AddressOfNames );
		}
		else
		{
			NameOffset = *AddressOfNames;
		}

		/* �Ƚ����� */
		if (strcmp( ProcName, (PCHAR)((ULONG)ImageBase + NameOffset) ) == 0)
		{
			if (bFile)
			{
				FunAddr = RVAToFileOffset( ImageBase, (ULONG)AddressOfFunctions[*AddressOfNameOrdinals] );
			}
			else
			{
				FunAddr = (ULONG)AddressOfFunctions[*AddressOfNameOrdinals];
			}

			break;
		}
	}

	#ifdef _CHECKED
		DbgPrint( "ixer:\tFindProcAddressFromExport() end:\t0x%x", FunAddr );
	#endif

	return FunAddr;
}


/*========================================================================
*
* ������:	GetImageBase
*
* ����:		PVOID	[IN]	ImageBase	- �ļ�����ַ
*
* ��������:	��ȡԸ�����ػ�ַ
*
* ����ֵ:	ULONG
*
=========================================================================*/
ULONG
GetImageBase( PVOID ImageBase )
{
	PIMAGE_DOS_HEADER		pDosHeader;
	PIMAGE_NT_HEADERS		pNtHeaders;

	#ifdef _CHECKED
		DbgPrint( "ixer:\tGetImageBase( 0x%08x )", ImageBase );
	#endif

	pDosHeader = (PIMAGE_DOS_HEADER)ImageBase;
	pNtHeaders = (PIMAGE_NT_HEADERS)((LONG)ImageBase + pDosHeader->e_lfanew );

	#ifdef _CHECKED
		DbgPrint( "ixer:\tGetImageBase() end:\t0x%08x", pNtHeaders->OptionalHeader.ImageBase );
	#endif

	return pNtHeaders->OptionalHeader.ImageBase;
}