#include "precomp.h"
#include "proc.h"
#include "WinOSVersion.h"


typedef NTSTATUS (NTAPI * PFN_RtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);

extern PLIST_ENTRY PsActiveProcessHead;
NTSTATUS GetWindowsVersion()
{
    UNICODE_STRING ustrFuncName = { 0 }; 
    RTL_OSVERSIONINFOEXW osverinfo = { sizeof(osverinfo) }; 
    PFN_RtlGetVersion pfnRtlGetVersion = NULL; 
	g_WindowsVersion= WINDOWS_VERSION_NONE;
	
	//��ȡ RtlGetVersion �����ĵ�ַ
    RtlInitUnicodeString(&ustrFuncName, L"RtlGetVersion"); 
    pfnRtlGetVersion = MmGetSystemRoutineAddress(&ustrFuncName); 
	
	//�����ȡ�� RtlGetVersion �����ĵ�ַ ��ֱ�ӵ��øú���
    if (pfnRtlGetVersion)
    { 
        DbgPrint("[GetWindowsVersion] Using \"RtlGetVersion\"\n"); 
		if (!NT_SUCCESS(pfnRtlGetVersion((PRTL_OSVERSIONINFOW)&osverinfo))) 
			return STATUS_UNSUCCESSFUL;  
    } 
	//������� PsGetVersion ��������ȡ����ϵͳ�汾��Ϣ
    else 
    {
        DbgPrint("[GetWindowsVersion] Using \"PsGetVersion\"\n");
        if(!PsGetVersion(&osverinfo.dwMajorVersion, &osverinfo.dwMinorVersion, &osverinfo.dwBuildNumber, NULL)) 
			return STATUS_UNSUCCESSFUL; 
    }
	
	//��ӡ����ϵͳ�汾��Ϣ
	DbgPrint( "[GetWindowsVersion] OS Version: NT %d.%d.%d sp%d.%d\n",
					osverinfo.dwMajorVersion, osverinfo.dwMinorVersion, osverinfo.dwBuildNumber,
						osverinfo.wServicePackMajor, osverinfo.wServicePackMinor );

	//�������ϵͳ�汾�� ȫ�ֱ��� g_WindowsVersion
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
	else if (osverinfo.dwMajorVersion == 6 && osverinfo.dwMinorVersion == 0) 
	{
        g_WindowsVersion= WINDOWS_VERSION_VISTA;
    }
	//7.0 = win7
	else if (osverinfo.dwMajorVersion == 7 && osverinfo.dwMinorVersion == 0) 
	{
        g_WindowsVersion= WINDOWS_VERSION_7;
    }

	//�������ϵͳ��Build Number��ȫ�ֱ���g_WindowsBuildNumber
	g_WindowsBuildNumber=osverinfo.dwBuildNumber;
	
    return STATUS_SUCCESS;
}

NTSTATUS InitOffsetbyWindowsVersion()
{
	NTSTATUS ntStatus=STATUS_UNSUCCESSFUL;
	PEPROCESS	pEprocSysProc;

	pEprocSysProc = PsGetCurrentProcess();
	/*���ݲ�ͬ�Ĳ���ϵͳ�汾���ò�ͬ���ں˱�����ƫ����*/
	switch(g_WindowsVersion)
	{
	case WINDOWS_VERSION_2K:
		#define WIN_2K
		g_SYSTEM_PROCESS_ID=8;

		g_EPROCESS_ExitTime=0x90;
		g_EPROCESS_ActiveProcessLinks=0xa0;
		g_EPROCESS_UniqueProcessId=0x9c;
		g_EPROCESS_ImageFileName=0x1fc;
		g_EPROCESS_ThreadListHead=0x270;
		g_EPROCESS_ObjectTable=0x128;
		g_EPROCESS_Peb=0x1b0;
		g_EPROCESS_InheritedFromUniqueProcessId=0x1c8;
		g_EPROCESS_JobLinks=0x220;
		g_Size_EPROCESS=0x288;

		g_ETHREAD_UniqueThreadID=0x1e4;
		g_ETHREAD_Teb=0;					//BSoD
		g_ETHREAD_win32ThreadStartAddr=0x230;
		g_ETHREAD_ThreadListEntry=0x240;
		g_ETHREAD_ThreadProcess=0x22c;
//  	g_ETHREAD_CreateTime=0x1b0;
//  	g_ETHREAD_ExitTime=0x1b8;
		g_ETHREAD_State=0x02d;
		g_ETHREAD_Priority=0x033; 
		g_ETHREAD_ContextSwitches=0x04c;
		g_ETHREAD_ApcQueueable=0x15a;
		g_ETHREAD_Alertable=0x164;		//??

		//msghook
		g_ETHREAD_Win32Thread=0x130;
		g_ETHREAD_CidOffset=0x1EC;

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

	case WINDOWS_VERSION_XP:
		#define WIN_XP
		/*�������ƫ����*/
		g_SYSTEM_PROCESS_ID=4;							//system����id

		g_EPROCESS_ExitTime=0x78;						//�˳�ʱ��
		g_EPROCESS_ActiveProcessLinks=0x88;				//ActiveProcessLinks
		g_EPROCESS_UniqueProcessId=0x84;				//����PID
		g_EPROCESS_ImageFileName=0x174;					//ӳ���ļ���
		g_EPROCESS_ThreadListHead=0x190;				//ThreadListHead
		g_EPROCESS_ObjectTable=0xC4;					//ObjectTable
		g_EPROCESS_Peb=0x1b0;							//PEB
		g_EPROCESS_InheritedFromUniqueProcessId=0x14c;	//������PID
		g_EPROCESS_ActiveThreads=0x1a0;					//ActiveThreads
		g_EPROCESS_JobLinks=0x184;						//JobLinks
		g_EPROCESS_Flags=0x248;
		g_Size_EPROCESS=0x260;							//EPROCESS

		/*�߳����ƫ����*/
		g_ETHREAD_UniqueThreadID=0x1f0;					//�߳�CID
		g_ETHREAD_Teb=0x20;								//Teb
		g_ETHREAD_win32ThreadStartAddr=0x228;			//win32�߳����
		g_ETHREAD_ThreadListEntry=0x22c;				//ThreadListEntry
		g_ETHREAD_ThreadListHead=0x190;					//ThreadListHead
		g_ETHREAD_ThreadProcess=0x220;
// 		g_ETHREAD_CreateTime=0x1c0;
// 		g_ETHREAD_ExitTime=0x1c8;
		g_ETHREAD_State=0x02d;							//״̬
		g_ETHREAD_Priority=0x033;						//���ȼ�
		g_ETHREAD_ContextSwitches=0x04c;				//�л�����

		g_ETHREAD_ApcQueueable=0x166;
		g_ETHREAD_Alertable=0x164;
		//msghook
		g_ETHREAD_Win32Thread=0x130;
		g_ETHREAD_CidOffset=0x1EC;

		/*ģ�����ƫ����*/
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

	case WINDOWS_VERSION_2K3:
		#define WIN_2K3
		g_SYSTEM_PROCESS_ID=4;
		
		g_EPROCESS_ExitTime=0x78;
		g_EPROCESS_ActiveProcessLinks=0x88 ;
		g_EPROCESS_UniqueProcessId=0x84;
		g_EPROCESS_ImageFileName=0x154;
		g_EPROCESS_ThreadListHead=0x170;
		g_EPROCESS_ObjectTable=0xC4;
		g_EPROCESS_Peb=0x190;
		g_EPROCESS_InheritedFromUniqueProcessId=0x128;
		g_EPROCESS_ActiveThreads=0x180;
		g_EPROCESS_JobLinks=0x164;
		g_EPROCESS_Flags=0;					//bsod
		g_Size_EPROCESS=0x278;
		
		g_ETHREAD_Teb=0;					//BSoD
		g_ETHREAD_UniqueThreadID=0x1f8;
		g_ETHREAD_win32ThreadStartAddr=0x22c;
		g_ETHREAD_ThreadListEntry=0x234;
		g_ETHREAD_ThreadProcess=0x228;
// 		g_ETHREAD_CreateTime=0x1c8;
// 		g_ETHREAD_ExitTime=0x1d0;
		g_ETHREAD_State=0x02d;
		g_ETHREAD_Priority=0x033;
		g_ETHREAD_ContextSwitches=0x04c;
		g_ETHREAD_ApcQueueable=0x166;	//??
		g_ETHREAD_Alertable=0x164;		//??
		
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

	case WINDOWS_VERSION_2K3_SP1_SP2:
		#define WIN_2K3_SP1_SP2
		g_EPROCESS_ExitTime=0x88;
		g_EPROCESS_ActiveProcessLinks=0x98;
		g_EPROCESS_UniqueProcessId=0x94;

		ntStatus=STATUS_SUCCESS;
		break;

	case WINDOWS_VERSION_VISTA:
		#define WIN_VISTA
		g_EPROCESS_ActiveProcessLinks=0x0a0;
		g_EPROCESS_UniqueProcessId=0x09c;

		ntStatus=STATUS_SUCCESS;
		break;

	case WINDOWS_VERSION_7:
		#define WIN_7
		g_EPROCESS_ExitTime=0xa8;
		g_EPROCESS_ActiveProcessLinks=0xb8;
		g_EPROCESS_UniqueProcessId=0xb4;

		ntStatus=STATUS_SUCCESS;
		break;
	}

	PsActiveProcessHead = *(PVOID *)((PUCHAR)pEprocSysProc + g_EPROCESS_ActiveProcessLinks + 4);
	return ntStatus;
}