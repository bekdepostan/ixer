//////////////////////////////////////////////////////////////////////////
// DataStruct.h

#ifndef  ___DATA_STRUCT___
#define	 ___DATA_STRUCT___

//#include "global.h"
#include <windef.h>

//数据结构定义
#define MAXNAMELEN     128
#define MAXPATHLEN     256

#define MAX_PROCESS_NUM		500
#define MAXBUFFERSIZE		4096
//进程结构体
typedef struct _PROCESS_ITEM
{
		BOOL		hideFlag;						//隐藏标记
//		BOOLEAN		isVerified;						//数字签名
//		BOOLEAN		ring3Access;					//用户层访问
//		PEPROCESS	pEPROCESS;						//EPROCESS
		HANDLE		ulPid;							//进程PID
		HANDLE		ulPPid;							//父进程ID
		CHAR		lpszImageName[MAXNAMELEN];		//映像名称
		CHAR		lpszImagePath[MAXPATHLEN];		//映像路径
		CHAR		lpszFileCorp[MAXNAMELEN];		//文件厂商
} PROCESS_ITEM, *PPROCESS_ITEM;

//进程项结构体
typedef struct _DATA_MEM_POOL{
	ULONG		procItemNum;
	PROCESS_ITEM processItems[MAX_PROCESS_NUM];
}DATA_MEM_POOL, *PDATA_MEM_POOL;
//
DATA_MEM_POOL g_dataMemPool;


#define MAX_THREAD_NUM	200
// 线程链表结构体
typedef struct _THREAD_ITEM{
	BOOL	deleted;
	ULONG	tid;					//TID
	ULONG	eThread;				//ETHREAD
	ULONG	teb;					//Teb
	CHAR	priority;				//优先级
	PVOID	win32ThreadStartAddr;	//Win32线程入口地址
	ULONG	contextSwitches;		//切换次数
	UCHAR	state;					//状态
}THREAD_ITEM,*PTHREAD_ITEM;

//线程状态结构
typedef enum _THREAD_STATE
{
		StateInitialized,
		StateReady,
		StateRunning,
		StateStandby,
		StateTerminated,
		StateWait,
		StateTransition,
		StateUnknown
}THREAD_STATE;

#define MAX_MODULE_NUM	200
// dll链表结构体
typedef struct _MODULE_ITEM{
	BOOL	deleted;				//是否已删除
	ULONG	pid;					//DLL所属进程PID
	PVOID	baseAddress;			//基址
	PVOID	entryPoint;				//入口地址
	ULONG	sizeOfImage;			//镜像大小
	CHAR	name[MAXNAMELEN];		//DLL名
	CHAR	path[MAXPATHLEN];		//DLL路径
}MODULE_ITEM, *PMODULE_ITEM;

//进程项结构体


//驱动/内核模块 结构体
#define MAX_KRNLMOD_NUM		300
//系统模块结构体
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

//ring0和ring3共享的数据内存池


//SSDT数据结构
typedef	struct	_SSDT_INFO
{
	unsigned int	index;					//服务函数索引
	char			funName[MAXNAMELEN];	//函数名
	DWORD			currentAddr;			//当前地址
	DWORD			nativeAddr;				//原始地址
	char			imagePath[MAX_PATH];	//镜像路径
	BOOL			HideFlag;				//
} SSDT_INFO, *PSSDT_INFO;

typedef	struct _SSDT_NAME
{
	char			funName[MAXNAMELEN];	//函数名
	unsigned int	index;					//函数索引
} SSDT_NAME,*PSSDT_NAME;

typedef	struct _SSDTINFO
{
	DWORD			index;				//服务函数在SSDT表中的序数
	DWORD			funAddr;			//服务函数地址
} SSDTINFO, *PSSDTINFO;

//文件、目录信息
typedef struct _DIRECTORY_INFO{
	CHAR FileName[50];      //??
	LARGE_INTEGER	AllocationSize;
    TIME_FIELDS		CreationTime;
	TIME_FIELDS		LastAccessTime ; 
	TIME_FIELDS		LastWriteTime;  
	TIME_FIELDS		ChangeTime; 	
	ULONG     FileAttributes ;    
}DIRECTORY_INFO ,*PDIRECTORY_INFO ;

//删除文件信息
typedef struct _DELETE_INFO{
	BOOLEAN  IsDelete;
}DELETE_INFO,*PDELETE_INFO;

//网络连接
typedef	struct InterNetLinkInfo
{
	BOOL	HideFlag;
	CHAR	Type[6];
	CHAR	SrcIP[25];
	CHAR	DesIP[25];
	DWORD	SrcPort;
	DWORD	DesPort;
	CHAR	State[32];
	DWORD	Pid;
	CHAR	ImagePath[MAX_PATH];
} NETLINKINFO, *PNETLINKINFO;
///////////////////////////////////////
#endif