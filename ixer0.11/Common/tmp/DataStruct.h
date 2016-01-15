//////////////////////////////////////////////////////////////////////////
// DataStruct.h

#ifndef  ___DATA_STRUCT___
#define	 ___DATA_STRUCT___

#include <windef.h>
//数据结构定义
#define MAXNAMELEN     128
#define MAXPATHLEN     256
#define MAX_PROCESS_NUM		500
//进程结构体
typedef struct _PROCESS_ITEM
{
//		BOOLEAN		hideFlag;						//隐藏标记
//		BOOLEAN		isVerified;						//数字签名
//		BOOLEAN		ring3Access;					//用户层访问
		PEPROCESS	pEPROCESS;						//EPROCESS
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
///////////////////////////////////////
#endif