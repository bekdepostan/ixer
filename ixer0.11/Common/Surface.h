//////////////////////////////////////////////////////////////////////////
// DataStruct.h

#ifndef  ___DATA_STRUCT___
#define	 ___DATA_STRUCT___

//���ݽṹ����
#include <windef.h>
//#include <wdm.h>
#include "ioctl.h"


/* �궨�� =================================================== */

#define MAX_FILE_NAME 255

// ����
#define MAXNAMELEN     128
#define MAXPATHLEN     256
// ������
#define MAXBUFFERSIZE	4096


// ��������
#define MAX_EPROCESS_NUM	1024	//������
//���̽ṹ��
typedef struct _PROCESS_ITEM
{
		BOOLEAN		hideFlag;						//���ر��
//		BOOLEAN		isVerified;						//����ǩ��
		ULONG		ulPid;							//����PID
		ULONG		ulPPid;							//������ID
		ULONG		pEproc;							//EPROCESS
		CHAR		pszImageName[MAXNAMELEN];		//ӳ������
		CHAR		pszImagePath[MAXPATHLEN];		//ӳ��·��
		CHAR		r3Access[MAXNAMELEN];			//�û������״̬
		CHAR		pszFileCorp[MAXNAMELEN];		//�ļ�����
} PROCESS_ITEM, *PPROCESS_ITEM;

// �̱߽߳�
#define MAX_THREAD_NUM		512		//�߳���
//�߳̽ṹ��
typedef struct _THREAD_ITEM
{
	ULONG	tid;					//CID
	ULONG	eThread;				//ETHREAD
	ULONG	teb;					//Teb
	CHAR	priority;				//���ȼ�
	PVOID	win32ThreadStartAddr;	//Win32�߳���ڵ�ַ
	ULONG	contextSwitches;		//�л�����
	UCHAR	state;					//״̬
}THREAD_ITEM, *PTHREAD_ITEM;

//�߳�״̬
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

// Dll�߽�
#define MAX_Dll_NUM			512		//Dll��
//dll�ṹ��
typedef struct _DLL_ITEM
{
//	BOOLEAN	hideFlag;						//���ر��
//	BOOLEAN	isVerified;						//����ǩ��
	ULONG	ulBaseAddr;
	ULONG	ulImageSize;
	WCHAR	pwszPath[MAX_PATH];
//	CHAR	lpszFileCorp[MAXNAMELEN];		//�ļ�����
}DLL_ITEM, *PDLL_ITEM;


// ����ģ��߽�
#define MAX_DRIVER_NUM		512		//������
//ϵͳģ��ṹ��
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
	CHAR    imagePath[256];			// "NDIS.sys"��"\SystemRoot\system32\DRIVERS\xxx.sys"��"\??\C:\WINDOWS\system32\drivers\xxx.sys"��"\??\E:\xxx.sys"
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

typedef struct {
    ULONG   dwNumberOfModules;
    SYSTEM_MODULE_INFORMATION   smi;
} MODULES, *PMODULES;

//�������ں�ģ��ṹ��
typedef struct _DRVMOD_ITEM
{
	BOOLEAN		hideFlags;					//���ر��
//	BOOLEAN		isVerified;					//����ǩ��
	ULONG		ulBaseAddr;					//��ַ
	ULONG		ulImageSize;				//��С
	ULONG		pDrvObject;					//��������
	ULONG		ulLoadOrder;				//����˳��
	WCHAR		pwszDrvName[MAXNAMELEN];	//ӳ������
	WCHAR		pwszDrvPath[MAXPATHLEN];	//ӳ��·��
	CHAR		pszFileCorp[MAXNAMELEN];	//�ļ�����
} DRVMOD_ITEM, *PDRVMOD_ITEM;

typedef struct _DRV_SRVNAME
{
	CHAR	szSrvName[MAXNAMELEN];
	CHAR	szSrvPath[MAXPATHLEN];
} DRV_SRVNAME, *PDRV_SRVNAME;

//SSDT���ݽṹ
typedef struct _SSDT_INFO
{
	CHAR		funName[MAXNAMELEN];	//������
	ULONG		currentAddr;			//��ǰ��ַ
	ULONG		nativeAddr;				//ԭʼ��ַ
	CHAR		imagePath[MAXPATHLEN];	//����·��
} SSDT_INFO, *PSSDT_INFO;

//
//���ڴ�ŵõ���ShadowSSDT�ĺ����ͺ�����
//
typedef struct _PSHADOW_INFO
{
	CHAR		funcName[MAXNAMELEN];	//������
	ULONG		currentAddr;			//��ǰ��ַ
	ULONG		nativeAddr;				//ԭʼ��ַ
	CHAR		imagePath[MAXPATHLEN];	//����·��
} SHADOW_INFO, *PSHADOW_INFO;

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

//ע���
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