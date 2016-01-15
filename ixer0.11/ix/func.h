//////////////////////////////////////////////////////////////////////////
//头文件
//////////////////////////////////////////////////////////////////////////
#ifndef  ___FUNCTION___
#define	 ___FUNCTION___


#include "ix.h" 

#define MAX_PATH	260


/*句柄表*/
typedef PVOID PHANDLE_TABLE_ENTRY_INFO;
typedef PVOID PHANDLE_TABLE;

typedef struct _HANDLE_TABLE_ENTRY 
{
	union 
	{
		PVOID                    Object;
		ULONG                    ObAttributes;
		PHANDLE_TABLE_ENTRY_INFO InfoTable;
		ULONG                    Value;
	};

	union 
	{
		union 
		{
			ACCESS_MASK GrantedAccess;

			struct 
			{
				USHORT GrantedAccessIndex;
				USHORT CreatorBackTraceIndex;
			};
		};

		LONG NextFreeTableEntry;
	};

} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

//
typedef BOOLEAN (*EX_ENUMERATE_HANDLE_ROUTINE)(
					IN PHANDLE_TABLE_ENTRY HandleTableEntry,
					IN HANDLE Handle,
					IN PVOID EnumParameter );
NTKERNELAPI
BOOLEAN
ExEnumHandleTable (
				   __in PHANDLE_TABLE HandleTable,
				   __in EX_ENUMERATE_HANDLE_ROUTINE EnumHandleProcedure,
				   __in PVOID EnumParameter,
				   __out_opt PHANDLE Handle );

NTKERNELAPI
VOID
KeInitializeApc (
				 PKAPC               Apc,
				 PKTHREAD            Thread,
				 UCHAR               StateIndex,
				 PKKERNEL_ROUTINE    KernelRoutine,
				 PKRUNDOWN_ROUTINE   RundownRoutine,
				 PKNORMAL_ROUTINE    NormalRoutine,
				 KPROCESSOR_MODE     ApcMode,
				 PVOID               NormalContext
				 );
NTKERNELAPI
BOOLEAN
KeInsertQueueApc (
				  IN PKAPC        Apc,
				  IN PVOID        SystemArgument1,
				  IN PVOID        SystemArgument2,
				  IN KPRIORITY    Increment
				  );

//PspTerminateThreadByPointer
typedef NTSTATUS (*PSPTERMINATETHREADBYPOINTER)( PETHREAD, NTSTATUS );
//
PSPTERMINATETHREADBYPOINTER MyPspTerminateThreadByPointer;
DWORD	g_PspTerminateThreadByPointer;	//PspTerminateThreadByPointer未导出函数地址
DWORD	g_PspExitThread;	//PspExitThread未导出函数地址

BOOLEAN GetPspTerminateThreadByPointer();
BOOL	GetPspExitThread();
ULONG	GetFunctionAddr( IN PCWSTR FunctionName);
ULONG	GetKiInsertQueueApc();
ULONG	GetKiAttachProcess();

NTSTATUS CharStringToWideChar(BOOLEAN boAnsiOrWide, char *srcAnsiStr, PWCHAR desWideChar);
///////////////////////////////////////
#endif