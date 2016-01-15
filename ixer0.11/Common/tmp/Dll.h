//////////////////////////////////////////////////////////////////////////
// Dll.h

#ifndef _DLL_H_
#define _DLL_H_

#include "..\Common\DataStruct.h"



#define MEM_IMAGE		0x1000000
#define PROCESS_QUERY_INFORMATION          (0x0400) 

/*全局变量*/
ULONG numOfDll; //Dll个数


// typedef struct _DLL_ITEM
// {
// 	WCHAR	pwszPath[MAX_PATH];
// 	ULONG	ulBaseAddr;
// //	ULONG	ulSize;
// }DLL_ITEM, *PDLL_ITEM;

typedef enum _MEMORY_INFORMATION_CLASS 
{
	MemoryBasicInformation,
	MemoryWorkingSetList,
	MemorySectionName
}MEMORY_INFORMATION_CLASS;

typedef struct _MEMORY_BASIC_INFORMATION {
	PVOID BaseAddress;
	PVOID AllocationBase;
	DWORD AllocationProtect;
	SIZE_T RegionSize;
	DWORD State;
	DWORD Protect;
	DWORD Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

typedef struct
{
	UNICODE_STRING SectionFileName;
	WCHAR       NameBuffer[ANYSIZE_ARRAY];
} MEMORY_SECTION_NAME, *PMEMORY_SECTION_NAME;

#pragma pack(1)
typedef struct ServiceDescriptorEntry
{
	DWORD *KiServiceTable;
	DWORD *CounterBaseTable;
	DWORD *nSystemCalls;
	DWORD *KiArgumentTable;
}SDE, *PSDE;
#pragma pack()

typedef struct ServiceDescriptorTable
{
	SDE ServiceDescriptor[4];
} SDT;

__declspec(dllimport) SDE KeServiceDescriptorTable;

NTKERNELAPI
UCHAR *
PsGetProcessImageFileName(
						  __in PEPROCESS Process
						  );
typedef NTSTATUS
(NTAPI *MyZwQueryVirtualMemory)(
								IN HANDLE ProcessHandle,
								IN PVOID BaseAddress,
								IN MEMORY_INFORMATION_CLASS MemoryInformationClass,
								OUT PVOID MemoryInformation,
								IN ULONG MemoryInformationLength,
								OUT PULONG ReturnLength OPTIONAL
								);


BOOLEAN StoreOrCheckDll(BOOLEAN	storeOrCheck, ULONG baseAddr, PWCHAR dosPath, PULONG dllNum, PDLL_ITEM pDllItem);
NTSTATUS ConvertVolumeDeviceToDosPath(LPWSTR FileName , LPWSTR OutFileName);
KPROCESSOR_MODE SetCurrentThreadPreviousMode(KPROCESSOR_MODE PreviousMode);
NTSTATUS EnumDllByZwQueryVirtualMem(BOOLEAN getNum, ULONG ulPid, PULONG dllNum, PDLL_ITEM pDllItem);
PDLL_ITEM GetDllList(ULONG ulPid, PULONG dllNum, PDLL_ITEM pDllItem);

//HANDLE MyOpenProcess( ULONG ulPid );
//////////////////////////////////////////////////////////////////////////
#endif