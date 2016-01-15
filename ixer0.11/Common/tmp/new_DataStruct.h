//////////////////////////////////////////////////////////////////////////
// DataStruct.h

#ifndef  ___DATA_STRUCT___
#define	 ___DATA_STRUCT___

#include <windef.h>
//���ݽṹ����
#define MAXNAMELEN     128
#define MAXPATHLEN     256
#define MAX_PROCESS_NUM		500
//���̽ṹ��
typedef struct _PROCESS_ITEM
{
//		BOOLEAN		hideFlag;						//���ر��
//		BOOLEAN		isVerified;						//����ǩ��
//		BOOLEAN		ring3Access;					//�û������
		PEPROCESS	pEPROCESS;						//EPROCESS
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
///////////////////////////////////////
#endif