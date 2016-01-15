//////////////////////////////////////////////////////////////////////////
//头文件
//////////////////////////////////////////////////////////////////////////
#ifndef  ___WindowsVersion___
#define  ___WindowsVersion___


#include "../Common/Surface.h"

//操作系统标识 枚举类型
typedef enum WIN_VER_DETAIL
{
		WINDOWS_VERSION_NONE,       // 0
		WINDOWS_VERSION_2K,
		WINDOWS_VERSION_XP,
		WINDOWS_VERSION_2K3,
		WINDOWS_VERSION_2K3_SP1_SP2,
		WINDOWS_VERSION_VISTA,
		WINDOWS_VERSION_7
} WIN_VER_DETAIL;

WIN_VER_DETAIL g_WindowsVersion;
DWORD g_WindowsBuildNumber;


ULONG	g_SYSTEM_PROCESS_ID;

DWORD	g_EPROCESS_ExitTime;
DWORD	g_EPROCESS_ActiveProcessLinks;
DWORD	g_EPROCESS_UniqueProcessId;
DWORD	g_EPROCESS_ImageFileName;
DWORD	g_EPROCESS_ThreadListHead;
DWORD	g_EPROCESS_ObjectTable;
DWORD	g_EPROCESS_Peb;
DWORD	g_EPROCESS_InheritedFromUniqueProcessId;
DWORD	g_EPROCESS_ActiveThreads;
DWORD	g_EPROCESS_JobLinks;
DWORD	g_EPROCESS_Flags;
DWORD g_Size_EPROCESS;

DWORD	g_ETHREAD_UniqueThreadID;
DWORD	g_ETHREAD_Teb;
DWORD	g_ETHREAD_win32ThreadStartAddr;
DWORD	g_ETHREAD_ThreadListEntry;
DWORD	g_ETHREAD_ThreadListHead;
DWORD	g_ETHREAD_ThreadProcess;
DWORD	g_ETHREAD_CreateTime;
DWORD	g_ETHREAD_ExitTime;
DWORD	g_ETHREAD_State;
DWORD	g_ETHREAD_Priority;
DWORD	g_ETHREAD_ContextSwitches;
DWORD	g_ETHREAD_ApcQueueable;
DWORD	g_ETHREAD_Alertable;
//msghook
DWORD	g_ETHREAD_Win32Thread;
DWORD	g_ETHREAD_CidOffset;

DWORD	g_PEB_ProcessParameters;
DWORD	g_PEB_Ldr;

DWORD	g_LDRDATA_InLoadOrderModuleList;
DWORD	g_LDRDATA_BaseAddress;
DWORD	g_LDRDATA_EntryPoint;
DWORD	g_LDRDATA_SizeOfImage; 
DWORD	g_LDRDATA_FullDllName;
DWORD	g_LDRDATA_BaseDllName;

DWORD	g_PROCESS_PARAMETERS_ImagePathName;

DWORD	g_EJOB_ActiveProcesses;
DWORD	g_EJOB_ProcessListHead;


NTSTATUS GetWindowsVersion();
NTSTATUS InitOffsetbyWindowsVersion();

#endif