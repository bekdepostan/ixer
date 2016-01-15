#ifndef	___API___
#define ___API___

#include "rootkit.h"


//////////////////////////////////////////////////////////////////////////
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef enum _DEBUG_CONTROL_CODE{
		DebugGetTraceInformation = 1,
		DebugSetInternalBreakpoint = 2,
		DebugSetSpeicalCall = 3,
		DebugClearSpeicalCalls = 4,
		DebugQuerySpecialCalls = 5,
		DebugDbgBreakPoint = 6,
		
		//��ȡKdVersionBlock
		DebugSysGetVersion = 7,
		
		/*
		���ں˿ռ俽�����û��ռ�,���ߴ��û��ռ俽�����û��ռ�
		�����ܴ��û��ռ俽�����ں˿ռ�
		*/
		DebugCopyMemoryChunks_0 = 8,
		
		/*
		���û��ռ俽�����ں˿ռ�,���ߴ��û��ռ俽�����û��ռ�
		�����ܴ��ں˿ռ俽�����û��ռ�
		*/
		DebugCopyMemoryChunks_1 = 9,
		
		/*
		�������ַ�������û��ռ�,����д���ں˿ռ�
		*/
		DebugCopyPhyMemory_0 = 10,
		
		/*
		���û��ռ俽���������ַ,���ܶ�ȡ�ں˿ռ�  
		*/
		DebugCopyPhyMemory_1 = 11,
		
		//��д��������ؿ��ƿ�
		DebugSysReadControlSpace = 12,
		DebugSysWriteControlSpace = 13,
		
		//��д�˿�
		DebugSysReadIoSpace = 14,
		DebugSysWriteIoSpace = 15,
		
		//�ֱ���� RDMSD@4 �� WRMSR@12
		DebugSysReadMsr = 16,
		DebugSysWriteMsr = 17,
		
		//��д��������
		DebugSysReadBusData = 18,
		DebugSysWriteBusData = 19,
		
		DebugSysCheckLowMemory = 20,
		
		//�ֱ���� _KdEnableDebugger@0 �� _KdDisableDebugger@0
		DebugSysEnableDebugger = 21,
		DebugSysDisableDebugger = 22,
		
		//��ȡ������һЩ������صı���
		DebugSysGetAutoEnableOnEvent = 23,
		DebugSysSetAutoEnableOnEvent = 24,
		DebugSysGetPitchDebugger = 25,
		DebugSysSetDbgPrintBufferSize = 26,
		DebugSysGetIgnoreUmExceptions = 27,
		DebugSysSetIgnoreUmExceptions = 28
}DEBUG_CONTROL_CODE;

typedef struct _MemoryChunk
{
	PVOID pPhysicalAddress;
	PVOID pUnknown;
	PVOID pBuffer;
	DWORD dwBufferSize;
}MemoryChunk; 

typedef struct MEMORY_CHUNKS
{
	PVOID Address; 
	ULONG pData; 
	ULONG Length;
}MEMORY_CHUNKS; 



typedef enum _THREAD_INFORMATION_CLASS {
    ThreadBasicInformation,
		ThreadTimes,
		ThreadPriority,
		ThreadBasePriority,
		ThreadAffinityMask,
		ThreadImpersonationToken,
		ThreadDescriptorTableEntry,
		ThreadEnableAlignmentFaultFixup,
		ThreadEventPair,
		ThreadQuerySetWin32StartAddress,
		ThreadZeroTlsCell,
		ThreadPerformanceCount,
		ThreadAmILastThread,
		ThreadIdealProcessor,
		ThreadPriorityBoost,
		ThreadSetTlsArrayAddress,
		ThreadIsIoPending,
		ThreadHideFromDebugger
} THREAD_INFORMATION_CLASS, *PTHREAD_INFORMATION_CLASS;



// typedef NTSTATUS (__stdcall *NTSYSTEMDEBUGCONTROL)(
// 												   IN DEBUG_CONTROL_CODE ControlCode,
// 												   IN PVOID InputBuffer,
// 												   IN ULONG InputBufferLength,
// 												   OUT PVOID OutputBuffer,
// 												   IN ULONG OutputBufferLength,
// 												   OUT PULONG ReturnLength
//                            );
typedef NTSTATUS ( __stdcall *NTSYSTEMDEBUGCONTROL ) ( IN int nCmd,
													  IN PVOID pInBuf,
													  IN ULONG nInLen, 
													  OUT PVOID pOutBuf,
													  IN ULONG nOutLen,
														  OUT PULONG nRetLen OPTIONAL);

 

typedef DWORD (_stdcall *ZWQUERYSYSTEMINFORMATION)
( 
	IN DWORD SystemInformationClass, 
	IN OUT PVOID SystemInformation, 
	IN ULONG SystemInformationLength, 
	OUT PULONG ReturnLength OPTIONAL 
	); 
// 
// NTSYSAPI 
// NTSTATUS
// NTAPI
// ZwAllocateVirtualMemory( 
// 						IN HANDLE               ProcessHandle,
// 						IN OUT PVOID            *BaseAddress,
// 						IN ULONG                ZeroBits,
// 						IN OUT PULONG           RegionSize,
// 						IN ULONG                AllocationType,
// 						IN ULONG                Protect );
typedef NTSTATUS (*ZWALLOCATEVIRTUALMEMORY)(
											IN HANDLE               ProcessHandle,
											IN OUT PVOID            *BaseAddress,
											IN ULONG                ZeroBits,
											IN OUT PULONG           RegionSize,
											IN ULONG                AllocationType,
											IN ULONG                Protect );



////////////////////////////////////////////////////////////////////////// 
//ȱ��psapi.hͷ���Լ��������� 
typedef    BOOL (__stdcall *ENUMPROCESSES)( OUT DWORD *, IN DWORD, OUT DWORD * ); 
typedef DWORD (__stdcall *GETMODULEFILENAMEEX)( IN HANDLE, IN HMODULE, OUT LPTSTR, IN DWORD ); 
typedef BOOL (__stdcall *ENUMPROCESSMODULES)( IN HANDLE, OUT HMODULE*, IN DWORD, OUT LPDWORD ); 


typedef NTSTATUS (*RTLUNICODESTRINGTOANSISTRING)(          
												 PANSI_STRING DestinationString,
												 PUNICODE_STRING SourceString,
												 BOOLEAN AllocateDestinationString);

typedef void (*RTLFREEANSISTRING)( PANSI_STRING AnsiString );

typedef void (*RTLINITANSISTRING)(	  IN OUT PANSI_STRING  DestinationString,  IN char *  SourceString  );

typedef DWORD (_stdcall *ZWUNLOADDRIVER)( PUNICODE_STRING RegPath);
typedef HANDLE ( _stdcall *OPENTHREAD)( DWORD Access, BOOL bInherit, DWORD dwThreadID); 
typedef DWORD (_stdcall *NTUNMAPVIEWOFSECTION)( HANDLE hProcess, PVOID Address);

 

typedef NTSTATUS (_stdcall *ZWQUERYINFORMATIONPROCESS)
( 
	IN HANDLE ProcessHandle, 
	IN DWORD ProcessInformationClass, 
	OUT PVOID ProcessInformation, 
	IN ULONG ProcessInformationLength, 
	OUT PULONG ReturnLength OPTIONAL 
); 

typedef NTSTATUS (_stdcall *ZWQUERYINFORMATIONTHREAD)
( 
						 IN HANDLE               ThreadHandle,
						 IN DWORD				ThreadInformationClass,
						 OUT PVOID               ThreadInformation,
						 IN ULONG                ThreadInformationLength,
						 OUT PULONG              ReturnLength OPTIONAL );



 

typedef NTSTATUS 
(WINAPI *NTQUERYINFORMATIONPROCESS)(
									HANDLE ProcessHandle,
									ULONG ProcessInformationClass,
									PVOID ProcessInformation,
									ULONG ProcessInformationLength,
									PULONG ReturnLength
									);

 
//////////////////////////////////////////////////////////////////////////
BOOL GetAPIFunction();


//////////////////////////////////////////////////////////////////////////
#endif