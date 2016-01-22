/*===================================================================
 * Filename: common.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 驱动通用模块，公用的函数在这里定义
 *
 * Date:   2013-5-13 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#ifndef _COMMON_H_
#define _COMMON_H_



/* 头文件 =================================================*/

//#include <ntifs.h>
#include "ntdefine.h"


/* 函数声明 ===============================================*/
/*
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
*/
typedef struct _SYSTEM_SERVICE_TABLE_SHADOW
{
	PVOID *ServiceTable; // array of entry points
	ULONG *CounterTable; // array of usage counters
	ULONG ServiceLimit; // number of table entries
	PVOID *ArgumentTable; // array of byte counts
} SYSTEM_SERVICE_TABLE_SHADOW, *PSYSTEM_SERVICE_TABLE_SHADOW;

#define ObjectNameInformation  1 

typedef struct _OBJECT_TYPE_INFORMATION { 
	UNICODE_STRING          TypeName;
	ULONG                   TotalNumberOfHandles;
	ULONG                   TotalNumberOfObjects;
	WCHAR                   Unused1[8];
	ULONG                   HighWaterNumberOfHandles;
	ULONG                   HighWaterNumberOfObjects;
	WCHAR                   Unused2[8];
	ACCESS_MASK             InvalidAttributes;
	GENERIC_MAPPING         GenericMapping;
	ACCESS_MASK             ValidAttributes;
	BOOLEAN                 SecurityRequired;
	BOOLEAN                 MaintainHandleCount;
	USHORT                  MaintainTypeList;
	POOL_TYPE               PoolType;
	ULONG                   DefaultPagedPoolCharge;
	ULONG                   DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

typedef struct _OBJECT_BASIC_INFORMATION {
	ULONG Attributes;
	ACCESS_MASK GrantedAccess;
	ULONG HandleCount;
	ULONG PointerCount;
	ULONG PagedPoolCharge;
	ULONG NonPagedPoolCharge;
	ULONG Reserved[ 3 ];
	ULONG NameInfoSize;
	ULONG TypeInfoSize;
	ULONG SecurityDescriptorSize;
	LARGE_INTEGER CreationTime;
} OBJECT_BASIC_INFORMATION, *POBJECT_BASIC_INFORMATION;

typedef struct _SYSTEM_HANDLE_INFORMATION {
		ULONG ProcessId;
		UCHAR ObjectTypeNumber;
		UCHAR Flags;
		USHORT Handle;
		PVOID Object;
		ACCESS_MASK GrantedAccess;
} _SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLE_INformATION_EX {
		ULONG NumberOfHandles;
		_SYSTEM_HANDLE_INFORMATION Information[1];
} _SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

typedef struct _KOBJECT_NAME_INFORMATION { 
	UNICODE_STRING          Name;
	WCHAR                   NameBuffer[];
} KOBJECT_NAME_INFORMATION, *PKOBJECT_NAME_INFORMATION;

// typedef struct _OBJECT_HEADER
// {
// 	ULONG		PointerCount;
// 	union
// 	{
// 		ULONG		HandleCount;
// 		ULONG		NextToFree;
// 	};
// 	POBJECT_TYPE		Type;
// 	UCHAR		NameInfoOffset;
// 	UCHAR		HandleInfoOffset;
// 	UCHAR		QuotaInfoOffset;
// 	UCHAR		Flags;
// 	union
// 	{
// 		ULONG	ObjectCreateInfo;
// 		ULONG	QuotaBlockCharged;
// 	};
// 	ULONG	SecurityDescriptor;
// } OBJECT_HEADER, *POBJECT_HEADER;


NTKERNELAPI
HANDLE
PsGetThreadProcessId(
					 __in PETHREAD Thread
					 );
NTKERNELAPI
HANDLE
PsGetThreadId(
			  __in PETHREAD Thread
			  );
NTKERNELAPI
HANDLE
PsGetThreadSessionId(
			  __in PETHREAD Thread
			  );

//原型声明
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation (
						  IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
						  OUT PVOID                   SystemInformation,
						  IN ULONG                    Length,
						  OUT PULONG                  ReturnLength);

// PDEVICE_OBJECT
// STDCALL
// IoGetAttachedDevice( PDEVICE_OBJECT DeviceObject );

NTSTATUS 
IxIoCallDriver( PDEVICE_OBJECT DeviceObject,
          PIRP Irp );

NTSTATUS
IxCreateFile(
		PHANDLE	pFileHandle,
		LPCWSTR FileName,
		ACCESS_MASK DesiredAccess,
		ULONG ShareAccess
		);

PVOID
GetModuleBase(
    LPCSTR ModuleFileName
    );

BOOLEAN
IxReferenceObject( PVOID Object );



VOID
IxPrintDevObj( PVOID pDeviceObj );

BOOLEAN
IxGetObjName( PVOID pObj, PVOID Name, ULONG Size );

LPWSTR
FullPathToFileName( LPWSTR FullPath, ULONG Length );

VOID
IxDisableWriteProtect( VOID );

VOID
IxEnableWriteProtect( VOID );


PKDPC
GainExclusivity( VOID );

NTSTATUS
ReleaseExclusivity( PVOID pkdpc );

VOID
RaiseCPUIrqlAndWait( IN PKDPC Dpc,
					IN PVOID DeferredContext,
					IN PVOID SystemArgument1,
					IN PVOID SystemArgument2
					);

PVOID
GetInfoTable( SYSTEM_INFORMATION_CLASS SystemInformationClass );

VOID	IxExFreePool( PVOID pPool );

VOID	IxObDereferenceObject( PVOID pObject );

PVOID
CreateWriteMDL( IN	ULONG	MemAddress,
				IN	ULONG	MdlLength,
				IN	OUT	PMDL	*pMdlPointer );

VOID
FreeWriteMDL(IN	PMDL	pMdlPointer,IN	PVOID	pMappedPointer);

//////////////////////////////////////////////////////////////////////////
#endif