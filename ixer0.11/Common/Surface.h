//////////////////////////////////////////////////////////////////////////
// DataStruct.h

#ifndef  ___DATA_STRUCT___
#define	 ___DATA_STRUCT___

//数据结构定义
#include <windef.h>
//#include <wdm.h>
#include "ioctl.h"


/* 宏定义 =================================================== */

#define MAX_FILE_NAME 255

// 长度
#define MAXNAMELEN     128
#define MAXPATHLEN     256
// 缓冲区
#define MAXBUFFERSIZE	4096


// 假设数量
#define MAX_EPROCESS_NUM	1024	//进程数
//进程结构体
typedef struct _PROCESS_ITEM
{
		BOOLEAN		hideFlag;						//隐藏标记
//		BOOLEAN		isVerified;						//数字签名
		ULONG		ulPid;							//进程PID
		ULONG		ulPPid;							//父进程ID
		ULONG		pEproc;							//EPROCESS
		CHAR		pszImageName[MAXNAMELEN];		//映像名称
		CHAR		pszImagePath[MAXPATHLEN];		//映像路径
		CHAR		r3Access[MAXNAMELEN];			//用户层访问状态
		CHAR		pszFileCorp[MAXNAMELEN];		//文件厂商
} PROCESS_ITEM, *PPROCESS_ITEM;

// 线程边界
#define MAX_THREAD_NUM		512		//线程数
//线程结构体
typedef struct _THREAD_ITEM
{
	ULONG	tid;					//CID
	ULONG	eThread;				//ETHREAD
	ULONG	teb;					//Teb
	CHAR	priority;				//优先级
	PVOID	win32ThreadStartAddr;	//Win32线程入口地址
	ULONG	contextSwitches;		//切换次数
	UCHAR	state;					//状态
}THREAD_ITEM, *PTHREAD_ITEM;

//线程状态
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
} THREAD_STATE;

// Dll边界
#define MAX_Dll_NUM			512		//Dll数
//dll结构体
typedef struct _DLL_ITEM
{
//	BOOLEAN	hideFlag;						//隐藏标记
//	BOOLEAN	isVerified;						//数字签名
	ULONG	ulBaseAddr;
	ULONG	ulImageSize;
	WCHAR	pwszPath[MAX_PATH];
//	CHAR	lpszFileCorp[MAXNAMELEN];		//文件厂商
}DLL_ITEM, *PDLL_ITEM;


// 驱动模块边界
#define MAX_DRIVER_NUM		512		//驱动数
//系统模块结构体
typedef struct _SYSTEM_MODULE_INFORMATION {
	HANDLE	Section;                 // Not filled in
	PVOID	MappedBase;
	PVOID	ImageBase;
	ULONG	ImageSize;
	ULONG	Flags;
	USHORT  LoadOrderIndex;
	USHORT  InitOrderIndex;
	USHORT  LoadCount;
	USHORT  imageNameOffset;		// imagePath + imageNameOffset = "360Box.sys"
	CHAR    imagePath[256];			// "NDIS.sys"、"\SystemRoot\system32\DRIVERS\xxx.sys"、"\??\C:\WINDOWS\system32\drivers\xxx.sys"、"\??\E:\xxx.sys"
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct {
    ULONG   dwNumberOfModules;
    SYSTEM_MODULE_INFORMATION   smi;
} MODULES, *PMODULES;

//驱动、内核模块结构体
typedef struct _DRVMOD_ITEM
{
	BOOLEAN		hideFlags;					//隐藏标记
//	BOOLEAN		isVerified;					//数字签名
	ULONG		ulBaseAddr;					//基址
	ULONG		ulImageSize;				//大小
	ULONG		pDrvObject;					//驱动对象
	ULONG		ulLoadOrder;				//加载顺序
	WCHAR		pwszDrvName[MAXNAMELEN];	//映像名称
	WCHAR		pwszDrvPath[MAXPATHLEN];	//映像路径
	CHAR		pszFileCorp[MAXNAMELEN];	//文件厂商
} DRVMOD_ITEM, *PDRVMOD_ITEM;

typedef struct _DRV_SRVNAME
{
	CHAR	szSrvName[MAXNAMELEN];
	CHAR	szSrvPath[MAXPATHLEN];
} DRV_SRVNAME, *PDRV_SRVNAME;

//SSDT数据结构
typedef struct _SSDT_INFO
{
	CHAR		funName[MAXNAMELEN];	//函数名
	ULONG		currentAddr;			//当前地址
	ULONG		nativeAddr;				//原始地址
	CHAR		imagePath[MAXPATHLEN];	//镜像路径
} SSDT_INFO, *PSSDT_INFO;

//
//用于存放得到的ShadowSSDT的函数和函数名
//
typedef struct _PSHADOW_INFO
{
	CHAR		funcName[MAXNAMELEN];	//函数名
	ULONG		currentAddr;			//当前地址
	ULONG		nativeAddr;				//原始地址
	CHAR		imagePath[MAXPATHLEN];	//镜像路径
} SHADOW_INFO, *PSHADOW_INFO;

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

//msghook
#define MAX_HOOK 0xfff

typedef struct _HOOK_INFO
{
	ULONG	Handle;
	ULONG	FuncOffset;
	ULONG	FuncBaseAddr;
	ULONG	iHook;
	ULONG	pid;
	ULONG	tid;
	CHAR	ProcessName[16];
	CHAR	ProcessPath[MAX_PATH];
} HOOK_INFO, *PHOOK_INFO;


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

//注册表
typedef struct _REGISTRY_KEY
{
	WCHAR	Name[255];
} REGISTRY_KEY, *PREGISTRY_KEY;

typedef struct _REGISTRY_VALUE
{
	ULONG	Type;
	ULONG	DataOffset;
	ULONG	DataLength;
	ULONG	NameLength;
	WCHAR	Name[1];
} REGISTRY_VALUE, *PREGISTRY_VALUE;

typedef struct _VALUE_KEY
{
	WCHAR	Key[MAX_PATH];
	WCHAR	Value[MAX_FILE_NAME];
} VALUE_KEY, *PVALUE_KEY;

typedef struct _SET_VALUE_KEY
{
	WCHAR	keyPath[MAX_PATH];
	WCHAR	valueName[MAX_FILE_NAME];
	ULONG	valType;
	PVOID	Data;
} SET_VALUE_KEY, *PSET_VALUE_KEY;

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