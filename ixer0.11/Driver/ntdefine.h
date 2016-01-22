/*===================================================================
 * Filename ntdefine.h
 * 
 * Author: kcrazy
 * Email:  thekcrazy@gmail.com
 *
 * Description: 
 *
 * Date:   2007-3-12 Original from kcrazy
 *         
 * Version: 1.0
 ==================================================================*/

#ifndef _NTDEF_H_
#define _NTDEF_H_


/* 头文件 ================================================= */
#include <ntddk.h>
//#include "global.h"


/* 结构定义 ================================================ */

#define MAX_PATH 260

#define NUMBER_HASH_BUCKETS 37

typedef struct _OBJECT_DIRECTORY
{
    struct _OBJECT_DIRECTORY_ENTRY *HashBuckets[NUMBER_HASH_BUCKETS];
    struct _EX_PUSH_LOCK *Lock;
    struct _DEVICE_MAP *DeviceMap;
    ULONG SessionId;
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;

//
// Object Directory Structures
//
typedef struct _OBJECT_DIRECTORY_ENTRY
{
    struct _OBJECT_DIRECTORY_ENTRY *ChainLink;
    PVOID Object;
} OBJECT_DIRECTORY_ENTRY, *POBJECT_DIRECTORY_ENTRY;

//
// Device Map
//
typedef struct _DEVICE_MAP
{
    POBJECT_DIRECTORY   DosDevicesDirectory;
    POBJECT_DIRECTORY   GlobalDosDevicesDirectory;
    ULONG               ReferenceCount;
    ULONG               DriveMap;
    UCHAR               DriveType[32];
} DEVICE_MAP, *PDEVICE_MAP;

typedef struct _QUERY_DIRECTORY
{
	ULONG Length;
	PSTRING FileName;
	FILE_INFORMATION_CLASS FileInformationClass;
	ULONG POINTER_ALIGNMENT FileIndex;
} QUERY_DIRECTORY, *PQUERY_DIRECTORY;
/*
typedef struct _FILE_DIRECTORY_INFORMATION {
    ULONG           NextEntryOffset;
    ULONG           FileIndex;
    LARGE_INTEGER   CreationTime;
    LARGE_INTEGER   LastAccessTime;
    LARGE_INTEGER   LastWriteTime;
    LARGE_INTEGER   ChangeTime;
    LARGE_INTEGER   EndOfFile;
    LARGE_INTEGER   AllocationSize;
    ULONG           FileAttributes;
    ULONG           FileNameLength;
    WCHAR           FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;
*/
/*
* PURPOSE: Header for every object managed by the object manager
* 来自 XP SP2 符号文件
*/

typedef struct _OBJECT_HEADER
{
	ULONG		PointerCount;
	union
	{
		ULONG		HandleCount;
		ULONG		NextToFree;
	};
	POBJECT_TYPE		Type;
	UCHAR		NameInfoOffset;
	UCHAR		HandleInfoOffset;
	UCHAR		QuotaInfoOffset;
	UCHAR		Flags;
	union
	{
		ULONG	ObjectCreateInfo;
		ULONG	QuotaBlockCharged;
	};
	ULONG	SecurityDescriptor;
} OBJECT_HEADER, *POBJECT_HEADER;

typedef struct _OBJECT_HEADER_NAME_INFO
{
	ULONG			Directory;
	UNICODE_STRING	Name;
	ULONG			QueryReferences;
} OBJECT_HEADER_NAME_INFO, *POBJECT_HEADER_NAME_INFO;


typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation,					// 0	Y		N
	SystemProcessorInformation,				// 1	Y		N
	SystemPerformanceInformation,			// 2	Y		N
	SystemTimeOfDayInformation,				// 3	Y		N
	SystemNotImplemented1,					// 4	Y		N
	SystemProcessesAndThreadsInformation,	// 5	Y		N
	SystemCallCounts,						// 6	Y		N
	SystemConfigurationInformation,			// 7	Y		N
	SystemProcessorTimes,					// 8	Y		N
	SystemGlobalFlag,						// 9	Y		Y
	SystemNotImplemented2,					// 10	Y		N
	SystemModuleInformation,				// 11	Y		N
	SystemLockInformation,					// 12	Y		N
	SystemNotImplemented3,					// 13	Y		N
	SystemNotImplemented4,					// 14	Y		N
	SystemNotImplemented5,					// 15	Y		N
	SystemHandleInformation,				// 16	Y		N
	SystemObjectInformation,				// 17	Y		N
	SystemPagefileInformation,				// 18	Y		N
	SystemInstructionEmulationCounts,		// 19	Y		N
	SystemInvalidInfoClass1,				// 20
	SystemCacheInformation,					// 21	Y		Y
	SystemPoolTagInformation,				// 22	Y		N
	SystemProcessorStatistics,				// 23	Y		N
	SystemDpcInformation,					// 24	Y		Y
	SystemNotImplemented6,					// 25	Y		N
	SystemLoadImage,						// 26	N		Y
	SystemUnloadImage,						// 27	N		Y
	SystemTimeAdjustment,					// 28	Y		Y
	SystemNotImplemented7,					// 29	Y		N
	SystemNotImplemented8,					// 30	Y		N
	SystemNotImplemented9,					// 31	Y		N
	SystemCrashDumpInformation,				// 32	Y		N
	SystemExceptionInformation,				// 33	Y		N
	SystemCrashDumpStateInformation,		// 34	Y		Y/N
	SystemKernelDebuggerInformation,		// 35	Y		N
	SystemContextSwitchInformation,			// 36	Y		N
	SystemRegistryQuotaInformation,			// 37	Y		Y
	SystemLoadAndCallImage,					// 38	N		Y
	SystemPrioritySeparation,				// 39	N		Y
	SystemNotImplemented10,					// 40	Y		N
	SystemNotImplemented11,					// 41	Y		N
	SystemInvalidInfoClass2,				// 42
	SystemInvalidInfoClass3,				// 43
	SystemTimeZoneInformation,				// 44	Y		N
	SystemLookasideInformation,				// 45	Y		N
	SystemSetTimeSlipEvent,					// 46	N		Y
	SystemCreateSession,					// 47	N		Y
	SystemDeleteSession,					// 48	N		Y
	SystemInvalidInfoClass4,				// 49
	SystemRangeStartInformation,			// 50	Y		N
	SystemVerifierInformation,				// 51	Y		Y
	SystemAddVerifier,						// 52	N		Y
	SystemSessionProcessesInformation		// 53	Y		N
} SYSTEM_INFORMATION_CLASS;
/*
typedef struct _SYSTEM_MODULE_INFORMATION {//Information Class 11
    ULONG Reserved [2];
    PVOID Base;
    ULONG Size;
    ULONG Flags;
    USHORT Index;
    USHORT Unknown;
    USHORT LoadCount;
    USHORT ModuleNameOffset;
    CHAR ImageName [256 ];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;
*/
/*
typedef struct
{
    ULONG   dwNumberOfModules;
    SYSTEM_MODULE_INFORMATION   smi;
} MODULES, *PMODULES;
*/
//typedef NTSTATUS (*PSPTERMINATETHREADBYPOINTER)( PETHREAD, NTSTATUS, BOOLEAN );

//
// Loader Data Table. Used to track DLLs loaded into an
// image.
//

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
    struct _ACTIVATION_CONTEXT * EntryPointActivationContext;

    PVOID PatchInformation;

} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef NTSTATUS (NTAPI *NTPROC)();
typedef NTPROC* PNTPROC;

typedef struct _SYSTEM_SERVICE_TABLE
{
	PNTPROC ServiceTable; // array of entry points
	PULONG CounterTable; // array of usage counters
	ULONG ServiceLimit; // number of table entries
	PCHAR ArgumentTable; // array of byte counts
}
SYSTEM_SERVICE_TABLE,
*PSYSTEM_SERVICE_TABLE,
**PPSYSTEM_SERVICE_TABLE;

typedef struct _SERVICE_DESCRIPTOR_TABLE
{
	SYSTEM_SERVICE_TABLE ntoskrnl; // ntoskrnl.exe (native api)
	SYSTEM_SERVICE_TABLE win32k; // win32k.sys (gdi/user support)
	SYSTEM_SERVICE_TABLE Table3; // not used
	SYSTEM_SERVICE_TABLE Table4; // not used
}
SYSTEM_DESCRIPTOR_TABLE,
*PSYSTEM_DESCRIPTOR_TABLE,
**PPSYSTEM_DESCRIPTOR_TABLE;

typedef struct _KPROCESS *PKPROCESS;


typedef struct _CONTROL_AREA
{
	ULONG		Segment;
	LIST_ENTRY	DereferenceList;
	ULONG		NumberOfSectionReferences;
	ULONG		NumberOfPfnReferences;
	ULONG		NumberOfMappedViews;
	USHORT		NumberOfSubsections;
	USHORT		FlushInProgressCount;
	ULONG		NumberOfUserReferences;
	ULONG		u;
	PVOID		FilePointer;
	ULONG		WaitingForDeletion;
	USHORT		ModifiedWriteCount;
	USHORT		NumberOfSystemCacheViews;
} CONTROL_AREA, *PCONTROL_AREA;

typedef struct _SEGMENT
{
	PCONTROL_AREA	ControlArea;
	/* 后边省略 */
} SEGMENT, *PSEGMENT;

typedef struct _SECTION_OBJECT
{
	PVOID		StartingVa;
	PVOID		EndingVa;
	PVOID		Parent;
	PVOID		LeftChild;
	PVOID		RightChild;
	PSEGMENT	Segment;
} SECTION_OBJECT, *PSECTION_OBJECT;


/* 函数声明 ================================================ */

/*
extern PUSHORT NtBuildNumber;

extern POBJECT_TYPE *IoDriverObjectType;

extern PSYSTEM_DESCRIPTOR_TABLE KeServiceDescriptorTable;



extern
VOID
NTAPI
KeAttachProcess(PKPROCESS Process);

extern
VOID
NTAPI
KeDetachProcess (VOID);


/* 函数声明 /
extern
NTSTATUS PsLookupProcessByProcessId(
     IN ULONG ulProcId, 
     OUT PEPROCESS *pEProcess
	);

extern
LPSTR
STDCALL
PsGetProcessImageFileName(PEPROCESS Process);

extern
NTKERNELAPI 
NTSTATUS 
ObOpenObjectByName( 
    IN POBJECT_ATTRIBUTES ObjectAttributes, 
    IN POBJECT_TYPE ObjectType, 
    IN KPROCESSOR_MODE AccessMode, 
    IN OUT PACCESS_STATE PassedAccessState OPTIONAL, 
    IN ACCESS_MASK DesiredAccess OPTIONAL, 
    IN OUT PVOID ParseContext OPTIONAL, 
    OUT PHANDLE Handle 
	);

extern
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

extern 
NTKERNELAPI 
NTSTATUS 
ObReferenceObjectByName( 
	IN PUNICODE_STRING ObjectName, 
	IN ULONG Attributes, 
	IN PACCESS_STATE PassedAccessState OPTIONAL, 
	IN ACCESS_MASK DesiredAccess OPTIONAL, 
	IN POBJECT_TYPE ObjectType, 
	IN KPROCESSOR_MODE AccessMode, 
	IN OUT PVOID ParseContext OPTIONAL, 
	OUT PVOID *Object 
	);

extern
NTKERNELAPI 
VOID KiDispatchInterrupt(VOID);

extern
NTSTATUS
ObCloseHandle (
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    );

extern
NTSYSAPI
NTSTATUS
STDCALL
ZwUnloadDriver( IN PUNICODE_STRING DriverServiceName );

extern
NTKERNELAPI
NTSTATUS
STDCALL
ObQueryNameString(
	PVOID Object,
	POBJECT_NAME_INFORMATION ObjectNameInfo,
	ULONG Length,
	PULONG ReturnLength
    );

extern
NTSYSAPI
NTSTATUS
STDCALL
ZwDeleteValueKey(
	IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  ValueName
	);

extern
int swprintf(wchar_t *,const wchar_t *,...);

extern
int sprintf(char *,const char *,...);
*/

#endif