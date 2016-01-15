#include "StdAfx.h"
#include "rootkit.h"
#include "WindowsVersion.h" 


WIN_VER_DETAIL g_WindowsVersion; 
ULONG g_SYSTEM_PROCESS_ID; 
DWORD g_EPROCESS_ExitTime;
DWORD g_EPROCESS_Flink;
DWORD g_EPROCESS_UniqueProcessId;
DWORD g_EPROCESS_ImageFileName;
DWORD g_EPROCESS_ThreadListHead;
DWORD g_EPROCESS_ObjectTable;
DWORD g_EPROCESS_Peb;
DWORD g_EPROCESS_InheritedFromUniqueProcessId;
DWORD g_EPROCESS_ActiveThreads;
DWORD g_EPROCESS_JobLinks;
DWORD g_Size_EPROCESS;

DWORD g_ETHREAD_UniqueThread;
DWORD g_ETHREAD_CLIENTID_PID;
DWORD g_ETHREAD_CLIENTID_TID; 
DWORD g_ETHREAD_StartAddress;
DWORD g_ETHREAD_ThreadListEntry;
DWORD g_ETHREAD_ThreadProcess;
DWORD g_ETHREAD_CreateTime;
DWORD g_ETHREAD_ExitTime;
DWORD g_ETHREAD_State;
DWORD g_ETHREAD_Priority;
DWORD g_ETHREAD_ContextSwitches;

DWORD g_PEB_ProcessParameters;
DWORD g_PEB_Ldr;

DWORD g_LDRDATA_InLoadOrderModuleList;
DWORD g_LDRDATA_BaseAddress;
DWORD g_LDRDATA_EntryPoint;
DWORD g_LDRDATA_SizeOfImage; 
DWORD g_LDRDATA_FullDllName;
DWORD g_LDRDATA_BaseDllName;

DWORD g_PROCESS_PARAMETERS_ImagePathName;

DWORD g_EJOB_ActiveProcesses;
DWORD g_EJOB_ProcessListHead;


/************************************************************************/
/* 获取操作系统版本 写入全局变量 g_WindowsVersion
	返回：
			STATUS_SUCCESS	表示成功；
			STATUS_UNSUCCESSFUL	表示失败；	                                                                 */
/************************************************************************/
NTSTATUS GetWindowsVersion()
{
	OSVERSIONINFOEX   osverinfo;   
	osverinfo.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);   
	if(!GetVersionEx((OSVERSIONINFO*)&osverinfo))   
	{    
		g_WindowsVersion= WINDOWS_VERSION_NONE;
		return STATUS_UNSUCCESSFUL;
	} 
	
	//5.0 = 2k
    if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 0) 
	{
        g_WindowsVersion= WINDOWS_VERSION_2K; 
    } 
	//5.1 = xp
	else if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 1) 
	{
        g_WindowsVersion= WINDOWS_VERSION_XP;
    } 
	//5.2 = 2k3
	else if (osverinfo.dwMajorVersion == 5 && osverinfo.dwMinorVersion == 2) 
	{
        if (osverinfo.wServicePackMajor==0) 
		{ 
            g_WindowsVersion= WINDOWS_VERSION_2K3;
        } 
		else 
		{
            g_WindowsVersion= WINDOWS_VERSION_2K3_SP1_SP2;
        }
    } 
	//6.0 = vista
	else if (osverinfo.dwMajorVersion == 6 /*&& osverinfo.dwMinorVersion == 0*/) 
	{
        g_WindowsVersion= WINDOWS_VERSION_VISTA;
    }
	//7.0 = win7
	else if (osverinfo.dwMajorVersion == 7 /*&& osverinfo.dwMinorVersion == 0*/) 
	{
        g_WindowsVersion= WINDOWS_VERSION_7;
    }
	
	return STATUS_SUCCESS; 
}

/************************************************************************/
/* 初始化内核变量的偏移量
	返回：
			STATUS_SUCCESS	表示成功；
			STATUS_UNSUCCESSFUL	表示失败；                                                                     */
/************************************************************************/
NTSTATUS InitOffsetbyWindowsVersion()
{    
	NTSTATUS ntStatus=STATUS_UNSUCCESSFUL;

	//根据不同的操作系统版本设置不同的内核变量的偏移量
	switch(g_WindowsVersion)
	{
	case WINDOWS_VERSION_2K:
		#define WIN_2K
		g_SYSTEM_PROCESS_ID=8;

		g_EPROCESS_ExitTime=0x90;
		g_EPROCESS_Flink=0xa0;
		g_EPROCESS_UniqueProcessId=0x9c;
		g_EPROCESS_ImageFileName=0x1fc;
		g_EPROCESS_ThreadListHead=0x270;
		g_EPROCESS_ObjectTable=0x128;
		g_EPROCESS_Peb=0x1b0;
		g_EPROCESS_InheritedFromUniqueProcessId=0x1c8;
		g_EPROCESS_JobLinks=0x220;
		g_Size_EPROCESS=0x288;

		g_ETHREAD_UniqueThread=0x1e4;
		g_ETHREAD_StartAddress=0x230;
		g_ETHREAD_ThreadListEntry=0x240;
		g_ETHREAD_ThreadProcess=0x22c;
 		g_ETHREAD_CreateTime=0x1b0;
 		g_ETHREAD_ExitTime=0x1b8;
		g_ETHREAD_State=0x02d;
		g_ETHREAD_Priority=0x033; 
		g_ETHREAD_ContextSwitches=0x04c;
		g_ETHREAD_CLIENTID_PID=0x1ec;//??
		g_ETHREAD_CLIENTID_TID=0x1f0; //??

		g_PEB_ProcessParameters=0x10;
		g_PEB_Ldr=0x0c;
		g_LDRDATA_InLoadOrderModuleList=0x0c;
		g_LDRDATA_BaseAddress=0x24;
		g_LDRDATA_EntryPoint=0x28;
		g_LDRDATA_SizeOfImage=0x2c; 
		g_LDRDATA_FullDllName=0x30;
		g_LDRDATA_BaseDllName=0x38;
		g_PROCESS_PARAMETERS_ImagePathName=0x3c;

		g_EJOB_ActiveProcesses=0x80;
		g_EJOB_ProcessListHead=0x18; 

		//ntStatus=STATUS_SUCCESS;
		break;
	//////////////////////////////////////////////////////////////////////////
	case WINDOWS_VERSION_XP:
		#define WIN_XP
		g_SYSTEM_PROCESS_ID=4;

		g_EPROCESS_ExitTime=0x78;
		g_EPROCESS_Flink=0x88 ;
		g_EPROCESS_UniqueProcessId=0x84;
		g_EPROCESS_ImageFileName=0x174;
		g_EPROCESS_ThreadListHead=0x190;
		g_EPROCESS_ObjectTable=0xC4;
		g_EPROCESS_Peb=0x1b0;
		g_EPROCESS_InheritedFromUniqueProcessId=0x14c;
		g_EPROCESS_ActiveThreads=0x1a0;
		g_EPROCESS_JobLinks=0x184;
		g_Size_EPROCESS=0x260;

		g_ETHREAD_UniqueThread=0x1f0;
		g_ETHREAD_StartAddress=0x224;
		g_ETHREAD_ThreadListEntry=0x22c;
		g_ETHREAD_ThreadProcess=0x220;
		g_ETHREAD_CreateTime=0x1c0;
		g_ETHREAD_ExitTime=0x1c8;
		g_ETHREAD_State=0x02d;
		g_ETHREAD_Priority=0x033;
		g_ETHREAD_ContextSwitches=0x04c;
		g_ETHREAD_CLIENTID_PID=0x1ec;
		g_ETHREAD_CLIENTID_TID=0x1f0; 

		g_PEB_ProcessParameters=0x10;
		g_PEB_Ldr=0x0c;
		g_LDRDATA_InLoadOrderModuleList=0x0c;
		g_LDRDATA_BaseAddress=0x24;
		g_LDRDATA_EntryPoint=0x28;
		g_LDRDATA_SizeOfImage=0x2c; 
		g_LDRDATA_FullDllName=0x30;
		g_LDRDATA_BaseDllName=0x38;
		g_PROCESS_PARAMETERS_ImagePathName=0x3c;

		g_EJOB_ActiveProcesses=0x80;
		g_EJOB_ProcessListHead=0x18;

		ntStatus=STATUS_SUCCESS;
		break;
	//////////////////////////////////////////////////////////////////////////
	case WINDOWS_VERSION_2K3_SP1_SP2: 
	//////////////////////////////////////////////////////////////////////////
	case WINDOWS_VERSION_2K3:
		#define WIN_2K3
		g_SYSTEM_PROCESS_ID=4;
		
		g_EPROCESS_ExitTime=0x78;
		g_EPROCESS_Flink=0x88 ;
		g_EPROCESS_UniqueProcessId=0x84;
		g_EPROCESS_ImageFileName=0x154;
		g_EPROCESS_ThreadListHead=0x170;
		g_EPROCESS_ObjectTable=0xC4;
		g_EPROCESS_Peb=0x190;
		g_EPROCESS_InheritedFromUniqueProcessId=0x128;
		g_EPROCESS_ActiveThreads=0x180;
		g_EPROCESS_JobLinks=0x164;
		g_Size_EPROCESS=0x278;
		
		g_ETHREAD_UniqueThread=0x1f8;
		g_ETHREAD_StartAddress=0x22c;
		g_ETHREAD_ThreadListEntry=0x234;
		g_ETHREAD_ThreadProcess=0x228;
		g_ETHREAD_CreateTime=0x1c8;
		g_ETHREAD_ExitTime=0x1d0;
		g_ETHREAD_State=0x02d;
		g_ETHREAD_Priority=0x033;
		g_ETHREAD_ContextSwitches=0x04c;
		g_ETHREAD_CLIENTID_PID=0x1ec;//??
		g_ETHREAD_CLIENTID_TID=0x1f0; //??
		
		g_PEB_ProcessParameters=0x10;
		g_PEB_Ldr=0x0c;
		g_LDRDATA_InLoadOrderModuleList=0x0c;
		g_LDRDATA_BaseAddress=0x24;
		g_LDRDATA_EntryPoint=0x28;
		g_LDRDATA_SizeOfImage=0x2c; 
		g_LDRDATA_FullDllName=0x30;
		g_LDRDATA_BaseDllName=0x38;
		g_PROCESS_PARAMETERS_ImagePathName=0x38;
		
		g_EJOB_ActiveProcesses=0x80;
		g_EJOB_ProcessListHead=0x18;
		
		ntStatus=STATUS_SUCCESS; 
		break;
		//////////////////////////////////////////////////////////////////////////
	case WINDOWS_VERSION_VISTA:
		#define WIN_VISTA
		//ntStatus=STATUS_SUCCESS;
		break; 
	}

	return ntStatus;
}