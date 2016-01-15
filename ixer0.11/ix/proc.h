//////////////////////////////////////////////////////////////////////////
// Process.h

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "func.h"
#include "common.h"

#define  VALID_PAGE		1
#define  INVALID_PAGE	0
#define  PDEINVALID		2
#define  PTEINVALID		3

/* 全局变量 ================================================ */
ULONG Eproc[1024];							//进程EPROCESS数组
ULONG hideEproc[128];						//隐藏进程EPROCESS数组


PDRIVER_OBJECT g_IxDrvObject;

typedef struct _SYSTEM_THREAD {
	LARGE_INTEGER           KernelTime;
	LARGE_INTEGER           UserTime;
	LARGE_INTEGER           CreateTime;
	ULONG                   WaitTime;
	PVOID                   StartAddress;
	CLIENT_ID               ClientId;
	KPRIORITY               Priority;
	LONG                    BasePriority;
	ULONG                   ContextSwitchCount;
	ULONG                   State;
	KWAIT_REASON            WaitReason;
} SYSTEM_THREAD, *PSYSTEM_THREAD;

//系统进程
typedef struct _SYSTEM_PROCESS_INFORMATION {
	ULONG                   NextEntryOffset;
	ULONG                   NumberOfThreads;
	LARGE_INTEGER           Reserved[3];
	LARGE_INTEGER           CreateTime;
	LARGE_INTEGER           UserTime;
	LARGE_INTEGER           KernelTime;
	UNICODE_STRING          ImageName;
	KPRIORITY               BasePriority;
	HANDLE                  ProcessId;
	HANDLE                  InheritedFromProcessId;
	ULONG                   HandleCount;
	ULONG                   Reserved2[2];
	ULONG                   PrivatePageCount;
	VM_COUNTERS             VirtualMemoryCounters;
	IO_COUNTERS             IoCounters;
	SYSTEM_THREAD           Threads[0];
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;



PLIST_ENTRY PsActiveProcessHead;

//原型声明
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation (
						  IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
						  OUT PVOID                   SystemInformation,
						  IN ULONG                    Length,
						  OUT PULONG                  ReturnLength);
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess (
						   IN HANDLE           ProcessHandle,
						   IN PROCESSINFOCLASS ProcessInformationClass,
						   OUT PVOID           ProcessInformation,
						   IN ULONG            ProcessInformationLength,
						   OUT PULONG          ReturnLength OPTIONAL);


NTSTATUS CheckHiddenProcByPspCidTable();

NTSTATUS
GetPspCidTableAddrByLookupPid( PULONG pPspCidTableAddr );

BOOLEAN IxEnumHandleCallBack( IN PHANDLE_TABLE_ENTRY HandleTableEntry,
							 IN HANDLE Handle,
							 IN PVOID EnumParameter );
NTSTATUS
CheckHiddenProcByActiveProcListHead();

PETHREAD
GetNextProcessThread( PEPROCESS pEproc, PETHREAD pEthread );

NTSTATUS	KillProcByPspTerminateThreadByPointer( PEPROCESS pEproc );
ULONG		IsValidAddr( ULONG uAddr );
NTSTATUS	GetImageFullPathByEprocess( PEPROCESS pEPROCESS, PCHAR pszimageFullPath );
NTSTATUS	GetProcessList();
BOOLEAN		StoreOrCheckProcess( BOOLEAN	storeOrCheck, ULONG ulEproc );
NTSTATUS	EnumProcessByZwQuerySysInfo();
NTSTATUS	GetProcessList();
//////////////////////////////////////////////////////////////////////////
#endif