//////////////////////////////////////////////////////////////////////////
// API.cpp

#include "StdAfx.h"
#include "API.h"


//函数指针
ZWQUERYSYSTEMINFORMATION		ZwQuerySystemInformation ;
NTSYSTEMDEBUGCONTROL			NtSystemDebugControl ;
ZWALLOCATEVIRTUALMEMORY   ZwAllocateVirtualMemory;
RTLUNICODESTRINGTOANSISTRING RtlUnicodeStringToAnsiString ;
RTLFREEANSISTRING RtlFreeAnsiString;
RTLINITANSISTRING RtlInitAnsiString;

ENUMPROCESSES            pEnumProcesses; 
GETMODULEFILENAMEEX        pGetModuleFileNameEx; 
ENUMPROCESSMODULES        pEnumProcessModules; 

ZWQUERYINFORMATIONPROCESS ZwQueryInformationProcess;
ZWQUERYINFORMATIONTHREAD ZwQueryInformationThread;
NTUNMAPVIEWOFSECTION NtUnmapViewOfSection; 
ZWUNLOADDRIVER ZwUnloadDriver;
OPENTHREAD Ring3OpenThread;

ZWOPENPROCESS ZwOpenProcess ;
ZWDUPLICATEOBJECT ZwDuplicateObject ; 
ZWPROTECTVIRTUALMEMORY ZwProtectVirtualMemory ;
ZWWRITEVIRTUALMEMORY ZwWriteVirtualMemory ;
ZWFREEVIRTUALMEMORY   ZwFreeVirtualMemory ;
ZWCLOSE   ZwClose ;

NTQUERYINFORMATIONPROCESS NtQueryInformationProcess;

//////////////////////////////////////////////////////////////////////////

BOOL GetAPIFunction()
{ 
	HMODULE hNTDLL =NULL; 
	HMODULE hPsDll = NULL;
	HMODULE hKernelDll =NULL;  

	//////////////////////////////////////////////////////////////////////////
	// 加载 ntdll.dll 模块 并获取该模块的函数地址
	//////////////////////////////////////////////////////////////////////////
	hNTDLL = LoadLibrary(TEXT("ntdll.dll") );  
	MyOutputDebugString("[GetAPIFunction] hNTDLL = 0x%08X \n",hNTDLL);  
	if(!hNTDLL)
	{
		MyOutputDebugString("[GetAPIFunction] LoadLibrary error = %d \n",GetLastError());   
		return FALSE;
	} 
	// 获取函数地址
	ZwQuerySystemInformation = (ZWQUERYSYSTEMINFORMATION)GetProcAddress(hNTDLL,"ZwQuerySystemInformation");
	MyOutputDebugString("[GetAPIFunction] ZwQuerySystemInformation = 0x%08X \n",ZwQuerySystemInformation);  
	
	NtSystemDebugControl = (NTSYSTEMDEBUGCONTROL)GetProcAddress(hNTDLL,"NtSystemDebugControl");
	MyOutputDebugString("[GetAPIFunction] NtSystemDebugControl = 0x%08X \n",NtSystemDebugControl);  
	
	ZwAllocateVirtualMemory=(ZWALLOCATEVIRTUALMEMORY) GetProcAddress (hNTDLL, "ZwAllocateVirtualMemory");
	MyOutputDebugString("[GetAPIFunction] ZwAllocateVirtualMemory = 0x%08X \n",ZwAllocateVirtualMemory);  
	
	RtlUnicodeStringToAnsiString=(RTLUNICODESTRINGTOANSISTRING) GetProcAddress(hNTDLL,"RtlUnicodeStringToAnsiString");
	MyOutputDebugString("[GetAPIFunction] RtlUnicodeStringToAnsiString = 0x%08X \n",RtlUnicodeStringToAnsiString);  
	
	RtlFreeAnsiString=(RTLFREEANSISTRING) GetProcAddress(hNTDLL,"RtlFreeAnsiString");
	MyOutputDebugString("[GetAPIFunction] RtlFreeAnsiString = 0x%08X \n",RtlFreeAnsiString);  
	
	RtlInitAnsiString=(RTLINITANSISTRING) GetProcAddress(hNTDLL,"RtlInitAnsiString");
	MyOutputDebugString("[GetAPIFunction] RtlInitAnsiString = 0x%08X \n",RtlInitAnsiString);  
	
	NtUnmapViewOfSection = (NTUNMAPVIEWOFSECTION)GetProcAddress(hNTDLL , "NtUnmapViewOfSection" );
	MyOutputDebugString("[GetAPIFunction] NtUnmapViewOfSection = 0x%08X \n",NtUnmapViewOfSection);  

	ZwUnloadDriver = (ZWUNLOADDRIVER)GetProcAddress(hNTDLL , "ZwUnloadDriver" );
	MyOutputDebugString("[GetAPIFunction] ZwUnloadDriver = 0x%08X \n",ZwUnloadDriver);  
	
	ZwOpenProcess = (ZWOPENPROCESS) GetProcAddress (hNTDLL, "ZwOpenProcess");
	MyOutputDebugString("[GetAPIFunction] ZwOpenProcess = 0x%08X \n",ZwOpenProcess);  

	ZwDuplicateObject =	(ZWDUPLICATEOBJECT) GetProcAddress (hNTDLL, "ZwDuplicateObject");
	MyOutputDebugString("[GetAPIFunction] ZwDuplicateObject = 0x%08X \n",ZwDuplicateObject);  

	ZwAllocateVirtualMemory =	(ZWALLOCATEVIRTUALMEMORY) GetProcAddress (hNTDLL, "ZwAllocateVirtualMemory");
	MyOutputDebugString("[GetAPIFunction] ZwAllocateVirtualMemory = 0x%08X \n",ZwAllocateVirtualMemory); 

	ZwProtectVirtualMemory = 		(ZWPROTECTVIRTUALMEMORY) GetProcAddress (hNTDLL, "ZwProtectVirtualMemory");
	MyOutputDebugString("[GetAPIFunction] ZwProtectVirtualMemory = 0x%08X \n",ZwProtectVirtualMemory); 

	ZwWriteVirtualMemory = 	(ZWWRITEVIRTUALMEMORY) GetProcAddress (hNTDLL, "ZwWriteVirtualMemory");
	MyOutputDebugString("[GetAPIFunction] ZwWriteVirtualMemory = 0x%08X \n",ZwWriteVirtualMemory); 

	ZwFreeVirtualMemory = (ZWFREEVIRTUALMEMORY) GetProcAddress (hNTDLL,   "ZwFreeVirtualMemory");
	MyOutputDebugString("[GetAPIFunction] ZwFreeVirtualMemory = 0x%08X \n",ZwFreeVirtualMemory); 

	ZwClose = 	(ZWCLOSE) GetProcAddress (hNTDLL, "ZwClose");	
	MyOutputDebugString("[GetAPIFunction] ZwClose = 0x%08X \n",ZwClose); 

	ZwQueryInformationProcess = (ZWQUERYINFORMATIONPROCESS)GetProcAddress( hNTDLL, "ZwQueryInformationProcess" );
	MyOutputDebugString("[GetAPIFunction] ZwQueryInformationProcess = 0x%08X \n",ZwQueryInformationProcess); 

	ZwQueryInformationThread = (ZWQUERYINFORMATIONTHREAD)GetProcAddress( hNTDLL, "ZwQueryInformationThread" );
	MyOutputDebugString("[GetAPIFunction] ZwQueryInformationThread = 0x%08X \n",ZwQueryInformationThread); 

	NtQueryInformationProcess = (NTQUERYINFORMATIONPROCESS)GetProcAddress(hNTDLL,"NtQueryInformationProcess");
	MyOutputDebugString("[GetAPIFunction] NtQueryInformationProcess = 0x%08X \n",NtQueryInformationProcess);



	//////////////////////////////////////////////////////////////////////////
	// 加载 Kernel32.dll 模块 并获取该模块的函数地址
	//////////////////////////////////////////////////////////////////////////
	hKernelDll = LoadLibrary(TEXT("Kernel32.dll") );  
	MyOutputDebugString("[GetAPIFunction] hKernelDll = 0x%08X \n",hKernelDll);  
	if(!hKernelDll)
	{
		MyOutputDebugString("[GetAPIFunction] LoadLibrary error = %d \n",GetLastError());   
		return FALSE;
	} 
	// 获取函数地址
	Ring3OpenThread = (OPENTHREAD)GetProcAddress(hKernelDll , "OpenThread" );
	MyOutputDebugString("[GetAPIFunction] Ring3OpenThread = 0x%08X \n",Ring3OpenThread);  



	//////////////////////////////////////////////////////////////////////////
	// 加载 PSAPI.dll 模块 并获取该模块的函数地址
	//////////////////////////////////////////////////////////////////////////
	hPsDll	 = LoadLibrary(TEXT("PSAPI.DLL"));
	MyOutputDebugString("[GetAPIFunction] hPsDll = 0x%08X \n",hPsDll);  
	if(!hPsDll)
	{
		MyOutputDebugString("[GetAPIFunction] LoadLibrary error = %d \n",GetLastError());   
		return FALSE;
	} 
	// 获取函数地址 
	pEnumProcesses  = 	(ENUMPROCESSES)GetProcAddress(hPsDll, "EnumProcesses");
	MyOutputDebugString("[GetAPIFunction] pEnumProcesses = 0x%08X \n",pEnumProcesses);  

	pEnumProcessModules  =	(ENUMPROCESSMODULES)GetProcAddress(hPsDll, "EnumProcessModules");
	MyOutputDebugString("[GetAPIFunction] pEnumProcessModules = 0x%08X \n",pEnumProcessModules);  

	pGetModuleFileNameEx  = 	(GETMODULEFILENAMEEX)GetProcAddress(hPsDll, "GetModuleFileNameExA");
	MyOutputDebugString("[GetAPIFunction] pGetModuleFileNameEx = 0x%08X \n",pGetModuleFileNameEx);  
	
	return TRUE;
}