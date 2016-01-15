//////////////////////////////////////////////////////////////////////////
// DataStruct.h

#ifndef  ___DATA_STRUCT___
#define	 ___DATA_STRUCT___

//#include "global.h"
#include <windef.h>

//���ݽṹ����
#define MAXNAMELEN     128
#define MAXPATHLEN     256

#define MAX_PROCESS_NUM		500
#define MAXBUFFERSIZE		4096
//���̽ṹ��
typedef struct _PROCESS_ITEM
{
		BOOL		hideFlag;						//���ر��
//		BOOLEAN		isVerified;						//����ǩ��
//		BOOLEAN		ring3Access;					//�û������
//		PEPROCESS	pEPROCESS;						//EPROCESS
		HANDLE		ulPid;							//����PID
		HANDLE		ulPPid;							//������ID
		CHAR		lpszImageName[MAXNAMELEN];		//ӳ������
		CHAR		lpszImagePath[MAXPATHLEN];		//ӳ��·��
		CHAR		lpszFileCorp[MAXNAMELEN];		//�ļ�����
} PROCESS_ITEM, *PPROCESS_ITEM;

//������ṹ��
typedef struct _DATA_MEM_POOL{
	ULONG		procItemNum;
	PROCESS_ITEM processItems[MAX_PROCESS_NUM];
}DATA_MEM_POOL, *PDATA_MEM_POOL;
//
DATA_MEM_POOL g_dataMemPool;


#define MAX_THREAD_NUM	200
// �߳�����ṹ��
typedef struct _THREAD_ITEM{
	BOOL	deleted;
	ULONG	tid;					//TID
	ULONG	eThread;				//ETHREAD
	ULONG	teb;					//Teb
	CHAR	priority;				//���ȼ�
	PVOID	win32ThreadStartAddr;	//Win32�߳���ڵ�ַ
	ULONG	contextSwitches;		//�л�����
	UCHAR	state;					//״̬
}THREAD_ITEM,*PTHREAD_ITEM;

//�߳�״̬�ṹ
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
// dll����ṹ��
typedef struct _MODULE_ITEM{
	BOOL	deleted;				//�Ƿ���ɾ��
	ULONG	pid;					//DLL��������PID
	PVOID	baseAddress;			//��ַ
	PVOID	entryPoint;				//��ڵ�ַ
	ULONG	sizeOfImage;			//�����С
	CHAR	name[MAXNAMELEN];		//DLL��
	CHAR	path[MAXPATHLEN];		//DLL·��
}MODULE_ITEM, *PMODULE_ITEM;

//������ṹ��


//����/�ں�ģ�� �ṹ��
#define MAX_KRNLMOD_NUM		300
//ϵͳģ��ṹ��
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

//ring0��ring3����������ڴ��


//SSDT���ݽṹ
typedef	struct	_SSDT_INFO
{
	unsigned int	index;					//����������
	char			funName[MAXNAMELEN];	//������
	DWORD			currentAddr;			//��ǰ��ַ
	DWORD			nativeAddr;				//ԭʼ��ַ
	char			imagePath[MAX_PATH];	//����·��
	BOOL			HideFlag;				//
} SSDT_INFO, *PSSDT_INFO;

typedef	struct _SSDT_NAME
{
	char			funName[MAXNAMELEN];	//������
	unsigned int	index;					//��������
} SSDT_NAME,*PSSDT_NAME;

typedef	struct _SSDTINFO
{
	DWORD			index;				//��������SSDT���е�����
	DWORD			funAddr;			//��������ַ
} SSDTINFO, *PSSDTINFO;

//�ļ���Ŀ¼��Ϣ
typedef struct _DIRECTORY_INFO{
	CHAR FileName[50];      //??
	LARGE_INTEGER	AllocationSize;
    TIME_FIELDS		CreationTime;
	TIME_FIELDS		LastAccessTime ; 
	TIME_FIELDS		LastWriteTime;  
	TIME_FIELDS		ChangeTime; 	
	ULONG     FileAttributes ;    
}DIRECTORY_INFO ,*PDIRECTORY_INFO ;

//ɾ���ļ���Ϣ
typedef struct _DELETE_INFO{
	BOOLEAN  IsDelete;
}DELETE_INFO,*PDELETE_INFO;

//��������
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