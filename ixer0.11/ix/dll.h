//////////////////////////////////////////////////////////////////////////
// Dll.h

#ifndef _DLL_H_
#define _DLL_H_

#include "..\Common\Surface.h"



#define MEM_IMAGE		0x1000000
#define PROCESS_QUERY_INFORMATION          (0x0400) 


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
/*
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
*/

//
// Loader Data Table. Used to track DLLs loaded into an
// image.
//
/*
typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union {
		LIST_ENTRY HashLinks;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union {
		struct {
			ULONG TimeDateStamp;
		};
		struct {
			PVOID LoadedImports;
		};
	};
//	struct _ACTIVATION_CONTEXT * EntryPointActivationContext;

	PVOID EntryPointActivationContext;
	PVOID PatchInformation;

} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;
*/

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


BOOLEAN StoreOrCheckDll(BOOLEAN	storeOrCheck, ULONG baseAddr, ULONG imageSize, PWCHAR dosPath, PDLL_ITEM pDllItem, PULONG numOfDll);

NTSTATUS ConvertVolumeDeviceToDosPath(LPWSTR FileName , LPWSTR OutFileName);

BOOLEAN EnumDllByInLoadOrderLinks(ULONG ulPid, PULONG numOfDll, PDLL_ITEM pDllItem);

PDLL_ITEM GetDllList(ULONG ulPid, PULONG numOfDll);


//////////////////////////////////////////////////////////////////////////
#endif