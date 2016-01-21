//////////////////////////////////////////////////////////////////////////
// rootkit.h

#ifndef ___ROOTKIT___
#define ___ROOTKIT___



#ifdef __cplusplus
extern "C" {
#endif
////////////////////////////////////////////////////////////////////////

//类型定义
typedef long NTSTATUS, *PNTSTATUS;

typedef unsigned long DWORD;
typedef DWORD * PDWORD;
typedef unsigned long ULONG;
typedef unsigned long ULONG_PTR;
typedef ULONG *PULONG;
typedef unsigned short WORD;
typedef unsigned char BYTE; 
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef void *PVOID;
typedef int BOOL;
typedef BYTE BOOLEAN;

#define OBJ_CASE_INSENSITIVE			0x00000040L
#define PAGE_READONLY					0x02
#define PAGE_READWRITE					0x04
#define DEF_KERNEL_BASE					0x80400000L
#define PROT_MEMBASE					0x80000000

//状态定义
#define NT_SUCCESS(Status)			    ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS					0x00000000//成功
#define STATUS_UNSUCCESSFUL				0xC0000001//失败
#define STATUS_NOT_IMPLEMENTED          0xC0000002//未执行
#define STATUS_INFO_LENGTH_MISMATCH     0xC0000004//长度不匹配
#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER		0xC000000D//非法参数
#endif
#define STATUS_ACCESS_DENIED			0xC0000022//访问拒绝
#define STATUS_BUFFER_TOO_SMALL			0xC0000023//缓冲区太小
#define STATUS_IO_DEVICE_ERROR          ((NTSTATUS) 0xC0000185)//IO设备错误

#define OBJ_KERNEL_HANDLE				0x00000200//内核句柄对象

///////////////////////////////////////////////////
//
// Define the create disposition values
//
#define FILE_SUPERSEDE 0x00000000
#define FILE_OPEN 0x00000001
#define FILE_CREATE 0x00000002
#define FILE_OPEN_IF 0x00000003
#define FILE_OVERWRITE 0x00000004
#define FILE_OVERWRITE_IF 0x00000005
#define FILE_MAXIMUM_DISPOSITION 0x00000005
//
// Define the create/open option flags
//
#define FILE_DIRECTORY_FILE 0x00000001
#define FILE_WRITE_THROUGH 0x00000002
#define FILE_SEQUENTIAL_ONLY 0x00000004
#define FILE_NO_INTERMEDIATE_BUFFERING 0x00000008

#define FILE_ATTRIBUTE_READONLY 0x00000001  
#define FILE_ATTRIBUTE_HIDDEN 0x00000002  
#define FILE_ATTRIBUTE_SYSTEM 0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY 0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE 0x00000020  
#define FILE_ATTRIBUTE_INROM 0x00000040 
//#define FILE_ATTRIBUTE_ENCRYPTED 0x00000040  
#define FILE_ATTRIBUTE_NORMAL 0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE 0x00000200 
#define MODULE_ATTR_NOT_TRUSTED 0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT 0x00000400  
#define MODULE_ATTR_NODEBUG 0x00000400 
#define FILE_ATTRIBUTE_COMPRESSED 0x00000800  
#define FILE_ATTRIBUTE_OFFLINE 0x00001000  
#define FILE_ATTRIBUTE_ROMSTATICREF 0x00001000 
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED 0x00002000  
#define FILE_ATTRIBUTE_ROMMODULE 0x00002000 
///////////////////////////////////////////////////

#ifndef  LOWORD
#define LOWORD(l)           ((unsigned short)(unsigned int)(l))
#endif

#ifndef HIWORD
#define HIWORD(l)           ((unsigned short)((((unsigned int)(l)) >> 16) & 0xFFFF))
#endif

// 定义ioctl相关的，用于R3和R0间的通信
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG) (((WORD) (a)) | ((DWORD) ((WORD) (b))) << 16))
#endif

#define MY_DEVICE_TYPE		 0x0000AA71   // 这地方可以自己改
#define DRIVER_IO(code)	 CTL_CODE (MY_DEVICE_TYPE, code, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                      结构体声明                           +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --      
//                                                                           --    --        
//                                                                              --    

#ifndef  ANSI_STRING
typedef struct _STRING {
  USHORT  Length;
  USHORT  MaximumLength;
  PCHAR  Buffer;
} ANSI_STRING, *PANSI_STRING;
#endif

// #ifndef  UNICODE_STRING
// typedef struct _UNICODE_STRING {
//   USHORT  Length;
//   USHORT  MaximumLength;
//   PWSTR  Buffer;
// } UNICODE_STRING, *PUNICODE_STRING;
// #endif

/* SSDT */
#pragma pack(1)//1字节对齐
typedef struct ServiceDescriptorEntry {
	unsigned int	*ServiceTableBase;
	unsigned int	*ServiceCounterTableBase; 
	unsigned int	NumberOfServices;
	unsigned char   *ParamTableBase;
} ServiceDescriptorTableEntry_t, *PServiceDescriptorTableEntry_t;

typedef struct ServiceDescriptorShadowEntry {
	unsigned int	*Win32kTableBase;
	unsigned int	*Win32kCounterTableBase;
	unsigned int	NumberofWin32kServices;
	unsigned char   *Win32kParamTableBase;
} ServiceDescriptorTableShadowEntry_t, *PServiceDescriptorTableShadowEntry_t;
#pragma pack()//字节对齐停止

// __declspec(dllimport)  ServiceDescriptorTableEntry_t    KeServiceDescriptorTable;
// PServiceDescriptorTableShadowEntry_t					KeServiceDescriptorTableShadow;

// OBJECT_ATTRIBUTES 对象属性
typedef struct _OBJECT_ATTRIBUTES {
	DWORD                         Length; 
	HANDLE                        RootDirectory;
	PUNICODE_STRING               ObjectName;
	DWORD                         Attributes;
	PSECURITY_DESCRIPTOR          SecurityDescriptor;
	PSECURITY_QUALITY_OF_SERVICE  SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;



// CLIENT_ID
#ifndef  CLIENT_ID
typedef struct _CLIENT_ID {
	HANDLE     UniqueProcess;
	HANDLE     UniqueThread;
} CLIENT_ID, *PCLIENT_ID;
#endif

/*
struct _SYSTEM_THREADS
{
	LARGE_INTEGER		KernelTime;
	LARGE_INTEGER		UserTime;
	LARGE_INTEGER		CreateTime;
	ULONG				WaitTime;
	PVOID				StartAddress;
	CLIENT_ID			ClientIs;
	KPRIORITY			Priority;
	KPRIORITY			BasePriority;
	ULONG				ContextSwitchCount;
	ULONG				ThreadState;
	KWAIT_REASON		WaitReason;
};

struct _SYSTEM_PROCESSES
{
	ULONG				NextEntryDelta;
	ULONG				ThreadCount;
	ULONG				Reserved[6];
	LARGE_INTEGER		CreateTime;
	LARGE_INTEGER		UserTime;
	LARGE_INTEGER		KernelTime;
	UNICODE_STRING		ProcessName;
	KPRIORITY			BasePriority;
	ULONG				ProcessId;
	ULONG				InheritedFromProcessId;
	ULONG				HandleCount;
	ULONG				Reserved2[2];
	VM_COUNTERS			VmCounters;
	IO_COUNTERS			IoCounters; //windows 2000 only
	struct _SYSTEM_THREADS		Threads[1];
};
*/
typedef LONG KPRIORITY;
typedef struct _VM_COUNTERS {
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
} VM_COUNTERS;
typedef VM_COUNTERS *PVM_COUNTERS;
//线程信息结构如下：
typedef struct _SYSTEM_THREADS{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    ULONG ThreadState;
    ULONG WaitReason;
    ULONG Reversed;//add by achillis
} SYSTEM_THREAD_INFORMATION,*PSYSTEM_THREADS;
//进程信息
typedef struct _SYSTEM_PROCESSES {
	ULONG NextEntryDelta;
	ULONG ThreadCount;
	LARGE_INTEGER Reserved1[3];
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ProcessName;
	KPRIORITY BasePriority;
	ULONG ProcessId;
	ULONG InheritedFromProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR PageDirectoryBase;
	VM_COUNTERS VmCounters;
	ULONG PrivatePageCount;// add by achillis
	IO_COUNTERS IoCounters;
	struct _SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, *PSYSTEM_PROCESSES;


// PROCESS_BASIC_INFORMATION
//#ifdef  PROCESS_BASIC_INFORMATION
//#undef  PROCESS_BASIC_INFORMATION
typedef struct _PROCESS_BASIC_INFORMATION {
	NTSTATUS		ExitStatus;
	ULONG			PebBaseAddress;
	ULONG_PTR		AffinityMask;
	LONG			BasePriority;
	ULONG_PTR		UniqueProcessId;
	ULONG_PTR		InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;
//#endif

typedef struct _THREAD_BASIC_INFORMATION {
    NTSTATUS    ExitStatus;
    PVOID       TebBaseAddress;
    ULONG       UniqueProcessId;
    ULONG       UniqueThreadId;
    KAFFINITY   AffinityMask;
    KPRIORITY   BasePriority;
    ULONG       DiffProcessPriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;

// SYSTEM_HANDLE_INFORMATION
typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
     USHORT UniqueProcessId;
     USHORT CreatorBackTraceIndex;
     UCHAR ObjectTypeIndex;
     UCHAR HandleAttributes;
     USHORT HandleValue;   // 句柄
     PVOID Object;         // 若HANDLE类型为线程,则它是ETHREAD结构
     ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

// typedef struct _SYSTEM_HANDLE_INFORMATION {
//      ULONG NumberOfHandles;
//      SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
// } SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;
// SystemHandleInformation
typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG       ProcessId;
    UCHAR       ObjectTypeNumber;
    UCHAR       Flags;
    USHORT      Handle;
    PVOID       Object;
    ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

/*
// SYSTEM_MODULE_INFORMATION 系统模块信息
typedef struct _SYSTEM_MODULE_INFORMATION {
	ULONG	Reserved[2];
	PVOID	Base;
	ULONG	Size;
	ULONG	Flags;
	USHORT  Index;
	USHORT  Unknown;
	USHORT  LoadCount;
	USHORT  ModuleNameOffset;
	CHAR    ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;


typedef struct {
    ULONG   dwNumberOfModules;
    SYSTEM_MODULE_INFORMATION   smi;
} MODULES, *PMODULES;
*/

// SYSTEM_BASIC_INFORMATION 系统基本信息
typedef struct _SYSTEM_BASIC_INFORMATION {
	ULONG Unknown;                  //Always contains zero
	ULONG MaximumIncrement;         //一个时钟的计量单位
	ULONG PhysicalPageSize;         //一个内存页的大小
	ULONG NumberOfPhysicalPages;    //系统管理着多少个页
	ULONG LowestPhysicalPage;       //低端内存页
	ULONG HighestPhysicalPage;      //高端内存页
	ULONG AllocationGranularity;
	ULONG LowestUserAddress;        //地端用户地址
	ULONG HighestUserAddress;       //高端用户地址
	ULONG ActiveProcessors;         //激活的处理器
	UCHAR NumberProcessors;         //有多少个处理器
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;


// SYSTEM_INFORMATION_CLASS 系统信息类
typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,  // 11
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,  // 0x10 -- 16
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemUnused1,
    SystemPerformanceTraceInformation,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemUnused3,
    SystemUnused4,
    SystemUnused5,
    SystemUnused6,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemSessionInformation
} SYSTEM_INFORMATION_CLASS;


#ifndef  SECTION_INHERIT
typedef enum _SECTION_INHERIT {
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT;
#endif



/******************************************************
*                                                     *
*                   部分 PE 结构                      *
*                                                     *
******************************************************/

#ifndef IMAGE_DOS_SIGNATURE
#define IMAGE_DOS_SIGNATURE 0x5a4d      // MZ
#endif

#ifndef IMAGE_NT_SIGNATURE
#define IMAGE_NT_SIGNATURE  0x00004550  // PE00
#endif

// _IMAGE_DOS_HEADER
struct MZHeader
{
	unsigned short signature; // "MZ"
	unsigned short partPag;
	unsigned short pageCnt;
	unsigned short reloCnt;
	unsigned short hdrSize;
	unsigned short minMem;
	unsigned short maxMem;
	unsigned short reloSS;
	unsigned short exeSP;
	unsigned short chksum;
	unsigned short exeIP;
	unsigned short reloCS;
	unsigned short tablOff;
	unsigned short overlay;
	unsigned char reserved[32];
	unsigned long offsetToPE;
};

// DWORD signature + _IMAGE_FILE_HEADER. 不是完全的IMAGE_FILE_HEADER.
struct PE_Header 
{
	unsigned long signature; // "PE\0\0"
	unsigned short machine;
	unsigned short numSections;
	unsigned long timeDateStamp;
	unsigned long pointerToSymbolTable;
	unsigned long numOfSymbols;
	unsigned short sizeOfOptionHeader;
	unsigned short characteristics;
};

// _IMAGE_OPTIONAL_HEADER
struct PE_ExtHeader
{
	unsigned short magic;
	unsigned char majorLinkerVersion;
	unsigned char minorLinkerVersion;
	unsigned long sizeOfCode;
	unsigned long sizeOfInitializedData;
	unsigned long sizeOfUninitializedData;
	unsigned long addressOfEntryPoint;
	unsigned long baseOfCode;
	unsigned long baseOfData;
	unsigned long imageBase;
	unsigned long sectionAlignment;
	unsigned long fileAlignment;
	unsigned short majorOSVersion;
	unsigned short minorOSVersion;
	unsigned short majorImageVersion;
	unsigned short minorImageVersion;
	unsigned short majorSubsystemVersion;
	unsigned short minorSubsystemVersion;
	unsigned long reserved1;
	unsigned long sizeOfImage;
	unsigned long sizeOfHeaders;
	unsigned long checksum;
	unsigned short subsystem;
	unsigned short DLLCharacteristics;
	unsigned long sizeOfStackReserve;
	unsigned long sizeOfStackCommit;
	unsigned long sizeOfHeapReserve;
	unsigned long sizeOfHeapCommit;
	unsigned long loaderFlags;
	unsigned long numberOfRVAAndSizes;
	unsigned long exportTableAddress;
	unsigned long exportTableSize;
	unsigned long importTableAddress;
	unsigned long importTableSize;
	unsigned long resourceTableAddress;
	unsigned long resourceTableSize;
	unsigned long exceptionTableAddress;
	unsigned long exceptionTableSize;
	unsigned long certFilePointer;
	unsigned long certTableSize;
	unsigned long relocationTableAddress;
	unsigned long relocationTableSize;
	unsigned long debugDataAddress;
	unsigned long debugDataSize;
	unsigned long archDataAddress;
	unsigned long archDataSize;
	unsigned long globalPtrAddress;
	unsigned long globalPtrSize;
	unsigned long TLSTableAddress;
	unsigned long TLSTableSize;
	unsigned long loadConfigTableAddress;
	unsigned long loadConfigTableSize;
	unsigned long boundImportTableAddress;
	unsigned long boundImportTableSize;
	unsigned long importAddressTableAddress;
	unsigned long importAddressTableSize;
	unsigned long delayImportDescAddress;
	unsigned long delayImportDescSize;
	unsigned long COMHeaderAddress;
	unsigned long COMHeaderSize;
	unsigned long reserved2;
	unsigned long reserved3;
};

// _IMAGE_SECTION_HEADER
struct SectionHeader
{
	unsigned char sectionName[8];
	unsigned long virtualSize;
	unsigned long virtualAddress;
	unsigned long sizeOfRawData;
	unsigned long pointerToRawData;
	unsigned long pointerToRelocations;
	unsigned long pointerToLineNumbers;
	unsigned short numberOfRelocations;
	unsigned short numberOfLineNumbers;
	unsigned long characteristics;
};

struct ImportDirEntry
{
	DWORD importLookupTable;
	DWORD timeDateStamp;
	DWORD fowarderChain;
	DWORD nameRVA;
	DWORD importAddressTable;
};

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;
/******************************************************
*                                                     *
*                 结构体自定义区域                    *
*                                                     *
******************************************************/


typedef struct _MY_PROCESS_INFO {
	ULONG     PID;
	ULONG	  KPEB;
	ULONG     CR3;
	CHAR      Name[16];
	ULONG     Reserved;
} MY_PROCESS_INFO, *PMY_PROCESS_INFO;



/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                      函数原型声明                         +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --      
//                                                                           --    --        
//                                                                              --

NTSYSAPI 
NTSTATUS
NTAPI
NtCreateFile(
                      OUT PHANDLE             FileHandle,
                      IN ACCESS_MASK          DesiredAccess,
                      IN POBJECT_ATTRIBUTES   ObjectAttributes,
                      //OUT PIO_STATUS_BLOCK    IoStatusBlock,
                      OUT PVOID    IoStatusBlock,
                      IN PLARGE_INTEGER       AllocationSize OPTIONAL,
                      IN ULONG                FileAttributes,
                      IN ULONG                ShareAccess,
                      IN ULONG                CreateDisposition,
                      IN ULONG                CreateOptions,
                      IN PVOID                EaBuffer OPTIONAL,
                      IN ULONG                EaLength 
					  );




NTSYSAPI 
VOID 
NTAPI 
RtlInitUnicodeString( 
     PUNICODE_STRING DestinationString, 
     PCWSTR SourceString 
     ); 


typedef enum _CREATE_FILE_TYPE
{
   CreateFileTypeNone,
   CreateFileTypeNamedPipe,
   CreateFileTypeMailslot
} CREATE_FILE_TYPE;

NTSYSAPI 
NTSTATUS
NTAPI
IoCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options
    ) ;

 

NTSYSAPI 
NTSTATUS 
NTAPI 
ZwOpenSection( 
     OUT PHANDLE SectionHandle, 
     IN ACCESS_MASK DesiredAccess, 
     IN POBJECT_ATTRIBUTES objectAttributes 
     ); 


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwMapViewOfSection( 
     IN HANDLE SectionHandle, 
     IN HANDLE ProcessHandle, 
     IN OUT PVOID *BaseAddress, 
     IN ULONG ZeroBits, 
     IN ULONG CommitSize, 
     IN OUT PLARGE_INTEGER SectionOffset OPTIONAL, 
     IN OUT PULONG ViewSize, 
     IN SECTION_INHERIT InheritDisposition, 
     IN ULONG AllocationType, 
     IN ULONG Protect 
     ); 


NTSYSAPI 
NTSTATUS 
NTAPI 
ZwUnmapViewOfSection( 
     IN HANDLE ProcessHandle, 
     IN PVOID BaseAddress 
     ); 

/*
NTSYSAPI 
NTSTATUS 
NTAPI 
NtVdmControl( 
     IN ULONG ControlCode, 
     IN PVOID ControlData 
     ); 

NTSYSAPI 
NTSTATUS
PsLookupProcessByProcessId (
    IN HANDLE ProcessId,
    OUT PEPROCESS *Process
    );

NTSYSAPI 
NTSTATUS
PsLookupThreadByThreadId (
    IN HANDLE ThreadId,
    OUT PETHREAD *Thread
    );
*/


NTSYSAPI 
NTSTATUS
NTAPI
NtQuerySystemInformation(

  IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
  OUT PVOID               SystemInformation,
  IN ULONG                SystemInformationLength,
  OUT PULONG              ReturnLength OPTIONAL 
  );


//
//

// VOID 
// RtlFreeUnicodeString(
//     IN PUNICODE_STRING  UnicodeString
//     );
// 
// NTSTATUS (WINAPI * _RtlAnsiStringToUnicodeString)
// 	(PUNICODE_STRING  DestinationString,
// 	 IN PANSI_STRING  SourceString,
// 	 IN BOOLEAN);
// 
// VOID (WINAPI *_RtlInitAnsiString)
// 	(IN OUT PANSI_STRING  DestinationString,
// 	 IN PCHAR  SourceString);


/*
HANDLE WINAPI OpenThread(
  IN  DWORD dwDesiredAccess,
  IN  BOOL bInheritHandle,
  IN  DWORD dwThreadId
);
*/


/////////////////////////////////////////////////////////////////         --          --     
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//     --     -      -     -- 
//+                                                           +//     --      -   -       -- 
//+                    一些自定义必备函数                     +//      --       -        --  
//+                                                           +//       -     sudami     -   
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//        --            --    
/////////////////////////////////////////////////////////////////          --        --      
//                                                                           --    --        
//                                                                              --

/*
// 写保护的开&关
void WPOFF()
{
	__asm {   //去掉内存保护
		cli
		mov  eax,cr0
		and  eax,not 10000h
		mov  cr0,eax
	}
}

void WPON()
{
	__asm {   //恢复内存保护  
		mov  eax,cr0
		or   eax,10000h
		mov  cr0,eax
		sti
	} 
}
*/

/* 
 * 在DDK的ntdef.h中InitializeObjectAttributes()是一个宏 
 */ 
#define InitializeObjectAttributes( p, n, a, r, s )   { \
	(p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
	(p)->RootDirectory = r;                             \
	(p)->Attributes = a;                                \
	(p)->ObjectName = n;                                \
	(p)->SecurityDescriptor = s;                        \
	(p)->SecurityQualityOfService = NULL;               \
}



///////////////声明Native API///////////////////////////////////////
// NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(
// 												 IN ULONG SystemInformationClass, //查询系统服务类型
// 												 IN PVOID SystemInformation, //接收系统信息缓冲区
// 												 IN ULONG SystemInformationLength, //接收信息缓冲区大小
// 												 OUT PULONG ReturnLength); //实际接收到的大小



// NTSYSAPI NTSTATUS NTAPI ZwOpenProcess(OUT PHANDLE ProcessHandle,IN ACCESS_MASK DesiredAccess,IN POBJECT_ATTRIBUTES
// 									  ObjectAttributes,IN PCLIENT_ID ClientId OPTIONAL);
typedef NTSTATUS (*ZWOPENPROCESS)(OUT PHANDLE ProcessHandle,IN ACCESS_MASK DesiredAccess,IN POBJECT_ATTRIBUTES
                                       ObjectAttributes,IN PCLIENT_ID ClientId OPTIONAL);
// 
// NTSYSAPI 
// NTSTATUS
// NTAPI
// ZwDuplicateObject(
// 				  IN HANDLE               SourceProcessHandle,
// 				  IN PHANDLE              SourceHandle,
// 				  IN HANDLE               TargetProcessHandle,
// 				  OUT PHANDLE             TargetHandle,
// 				  IN ACCESS_MASK          DesiredAccess OPTIONAL,
// 				  IN BOOLEAN              InheritHandle,
// 				  IN ULONG                Options );
typedef NTSTATUS (*ZWDUPLICATEOBJECT)(
									  IN HANDLE               SourceProcessHandle,
									  IN PHANDLE              SourceHandle,
									  IN HANDLE               TargetProcessHandle,
									  OUT PHANDLE             TargetHandle,
									  IN ACCESS_MASK          DesiredAccess OPTIONAL,
									  IN BOOLEAN              InheritHandle,
									  IN ULONG                Options );








// NTSYSAPI 
// NTSTATUS
// NTAPI
// ZwProtectVirtualMemory(
// 					   IN HANDLE               ProcessHandle,
// 					   IN OUT PVOID            *BaseAddress,
// 					   IN OUT PULONG           NumberOfBytesToProtect,
// 					   IN ULONG                NewAccessProtection,
// 					   OUT PULONG              OldAccessProtection );
typedef NTSTATUS (*ZWPROTECTVIRTUALMEMORY)(
					   IN HANDLE               ProcessHandle,
					   IN OUT PVOID            *BaseAddress,
					   IN OUT PULONG           NumberOfBytesToProtect,
					   IN ULONG                NewAccessProtection,
					   OUT PULONG              OldAccessProtection );


// NTSYSAPI 
// NTSTATUS
// NTAPI
// ZwWriteVirtualMemory(
// 					 IN HANDLE               ProcessHandle,
// 					 IN PVOID                BaseAddress,
// 					 IN PVOID                Buffer,
// 					 IN ULONG                NumberOfBytesToWrite,
// 					 OUT PULONG              NumberOfBytesWritten OPTIONAL );
typedef NTSTATUS (*ZWWRITEVIRTUALMEMORY)(
					 IN HANDLE               ProcessHandle,
					 IN PVOID                BaseAddress,
					 IN PVOID                Buffer,
					 IN ULONG                NumberOfBytesToWrite,
					 OUT PULONG              NumberOfBytesWritten OPTIONAL );


// NTSYSAPI 
// NTSTATUS
// NTAPI
// ZwFreeVirtualMemory(					
// 					IN HANDLE               ProcessHandle,
// 					IN PVOID                *BaseAddress,
// 					IN OUT PULONG           RegionSize,
// 					IN ULONG                FreeType );
typedef NTSTATUS (*ZWFREEVIRTUALMEMORY)(					
					IN HANDLE               ProcessHandle,
					IN PVOID                *BaseAddress,
					IN OUT PULONG           RegionSize,
					IN ULONG                FreeType );

// NTSYSAPI 
// NTSTATUS
// NTAPI
// ZwClose(
// 		IN HANDLE               ObjectHandle );
typedef NTSTATUS (*ZWCLOSE)(
		IN HANDLE               ObjectHandle );
//////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////
#endif