//////////////////////////////////////////////////////////////////////////
// KrnlMod.c

#include "precomp.h"
#include "func.h"
#include "ix.h"
#include "dll.h"
#include "..\Common\Surface.h"
#include "proc.h"
#include "drvMod.h"



PDRVMOD_ITEM GetKrnlModuleList(PULONG numOfDrv)
{
	NTSTATUS status=STATUS_SUCCESS;
	PDRVMOD_ITEM pDrvItem=NULL;

	//��drv���������ڴ�
	pDrvItem=(DRVMOD_ITEM *)ExAllocatePoolWithTag(PagedPool, sizeof(DRVMOD_ITEM)*MAX_DRIVER_NUM*2, 'drv');
	if (pDrvItem)
	{
		//�ڴ�����
		RtlZeroMemory(pDrvItem, sizeof(DRVMOD_ITEM)*MAX_DRIVER_NUM*2);
		//ZwQuerySysInfoö���ں�ģ��
		status=EnumKrnlModByZwQuerySysInfo(numOfDrv, pDrvItem);
		//���
		status=CheckHiddenDrvByPsLoadedModuleList(numOfDrv, pDrvItem);
	}

	return pDrvItem;
}

NTSTATUS EnumKrnlModByZwQuerySysInfo(PULONG numOfDrv, PDRVMOD_ITEM pDrvItem)
{
	NTSTATUS status=STATUS_SUCCESS;
	PSYSTEM_MODULE_INFORMATION pDrvModInfo=NULL;
	PVOID	pDrvtmp=NULL;
	ULONG	dwRetSize=0;
	ULONG	drvNums=0;
	ULONG	i;
	ULONG	pDrvObject=0;
	WCHAR	wName[256];
	WCHAR	wPath[256];

	//��ȡ��С
	status=ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &dwRetSize);
	//�����ڴ�
	pDrvtmp = ExAllocatePoolWithTag(PagedPool, dwRetSize, 'tmp');		//dwRetSize ��Ҫ�Ĵ�С
	if (pDrvtmp)
	{
		//�ٴ�ִ��,��ö�ٽ���ŵ�ָ�����ڴ�����
		status=ZwQuerySystemInformation(SystemModuleInformation, pDrvtmp, dwRetSize, NULL);
		if ( NT_SUCCESS(status) )
		{
			//ģ����
			drvNums = *( (PULONG)pDrvtmp );
			//ģ������ͷָ��
			pDrvModInfo=(PSYSTEM_MODULE_INFORMATION)( (PULONG)pDrvtmp + 1 );

			for (i=0; i< drvNums; i++)
			{
//				DbgPrint("������:%s\n", pDrvModInfo[i].imagePath+pDrvModInfo[i].imageNameOffset);
				//��ȡ��������
				GetDrvObjectByName(pDrvModInfo[i].imagePath+pDrvModInfo[i].imageNameOffset, &pDrvObject);
				//ת��������
				CharStringToWideChar(TRUE, pDrvModInfo[i].imagePath+pDrvModInfo[i].imageNameOffset, wName);
				//ת������·��
				CharStringToWideChar(TRUE, pDrvModInfo[i].imagePath, wPath);

				//�洢������Ϣ
				StoreOrCheckKrnlMod(TRUE, wName, (ULONG)pDrvModInfo[i].ImageBase, pDrvModInfo[i].ImageSize, pDrvObject, wPath, pDrvModInfo[i].LoadOrderIndex, numOfDrv, pDrvItem);
			}
			KdPrint(("EnumKrnlModByZwQuerySysInfo()->����ģ��:%d��\n", drvNums ));
		}
		
		ExFreePoolWithTag(pDrvtmp, 'tmp');
	}

	return status;
}

NTSTATUS CheckHiddenDrvByPsLoadedModuleList(PULONG numOfDrv, PDRVMOD_ITEM pDrvItem)
{
	NTSTATUS status=STATUS_SUCCESS;
	PDRIVER_OBJECT DrvObject=NULL;
	PLDR_DATA_TABLE_ENTRY LdrEntry=NULL;
	PLIST_ENTRY	LdrHeader=NULL, LdrNext=NULL;
	char drvName[256];
//	WCHAR wDrvPath[256];
	ULONG pDrvObject;

	DrvObject=g_IxDrvObject;
	ASSERT(DrvObject != NULL);

	LdrHeader=( (PLIST_ENTRY)DrvObject->DriverSection )->Flink;
	for (LdrNext=LdrHeader->Flink; LdrNext != LdrHeader; LdrNext=LdrNext->Flink)
	{
		LdrEntry= CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if (LdrEntry)
		{
//			RtlZeroMemory(wDrvPath, sizeof(wDrvPath));

			//ת��������
			CharStringToWideChar(FALSE, drvName, LdrEntry->BaseDllName.Buffer);
//			wcsncpy( wDrvPath, LdrEntry->FullDllName.Buffer, LdrEntry->FullDllName.Length );
//			memcpy( wDrvPath, LdrEntry->FullDllName.Buffer, LdrEntry->FullDllName.Length );

			//��ȡ��������
			GetDrvObjectByName(drvName, &pDrvObject);
			KdPrint(( "StoreOrCheckDrvMod(0x%08X, %ws)\n", (ULONG)LdrEntry->DllBase, LdrEntry->FullDllName.Buffer));

			//�洢ö�ٵ���������Ϣ
			StoreOrCheckKrnlMod(FALSE, LdrEntry->BaseDllName.Buffer, (ULONG)LdrEntry->DllBase, LdrEntry->SizeOfImage, pDrvObject, LdrEntry->FullDllName.Buffer, LdrEntry->LoadCount, numOfDrv, pDrvItem);
		}
	}

	return status;
}

NTSTATUS GetDrvObjectByName(char *pszDrvName, PULONG pDrvObject)
{
	NTSTATUS status=STATUS_SUCCESS;
	ANSI_STRING ansiDev, ansFileDev;
	UNICODE_STRING uniObjName, uniFileDev;

	char dev[256]={0};
	char sym[256]={0};
	char fileDev[256]={0};
	char fileSym[256]={0};
	//��������
	PDRIVER_OBJECT	pDrvObj=NULL;

	//����ָ������
	*pDrvObject = 0;

	//�ǿ���תUnicode������
	if (pszDrvName[0] != '\0')
	{
		strcpy(dev, "\\Driver\\");
		// "\Driver\xxx.sys"
		strcat(dev, pszDrvName);

		strcpy(fileDev, "\\FileSystem\\");
		//"\FileSystem\xxx.sys"
		strcat(fileDev, pszDrvName);

		//ȥ����׺��(��ȥ��4���ַ���"\Driver\xxx")
		RtlMoveMemory(sym, dev, strlen(dev)-4);
		//ȥ����׺��(��ȥ��4���ַ���"\FileSystem\xxx")
		RtlMoveMemory(fileSym, fileDev, strlen(dev)-4);

		RtlInitAnsiString(&ansiDev, sym);
		RtlInitAnsiString(&ansFileDev, fileSym);
		//תUnicode������
		RtlAnsiStringToUnicodeString(&uniObjName, &ansiDev, TRUE);
		//תUnicode������
		RtlAnsiStringToUnicodeString(&uniFileDev, &ansFileDev, TRUE);
	}

	//���ö���ͨ������("\Driver\xxx")
	status=ObReferenceObjectByName(&uniObjName, OBJ_CASE_INSENSITIVE, NULL, FILE_ALL_ACCESS, IoDriverObjectType, KernelMode, NULL, (PVOID *)&pDrvObj);
	if ( !NT_SUCCESS(status) )
	{
		//���ö���ͨ������("\FileSystem\xxx")
		status=ObReferenceObjectByName(&uniFileDev, OBJ_CASE_INSENSITIVE, NULL, FILE_ALL_ACCESS, IoDriverObjectType, KernelMode, NULL, (PVOID *)&pDrvObj);
		if ( !NT_SUCCESS(status) )
		{
//			DbgPrint("ObReferenceObjectByName(\"\\Driver\\xxx or \\FileSystem\\xxx\") Failed! Status: ������:%ws \t 0x%x\n", uniObjName.Buffer, status);
			return status;
		}
	}

//	DbgPrint("������:%ws \t ��������:0x%08X\n", uniObjName.Buffer, pDrvObj);
	*pDrvObject=(ULONG)pDrvObj;
	//�����������
	ObDereferenceObject(pDrvObj);
	return status;
}

/*========================================================================
*
* ������:	StoreOrCheckKrnlMod
*
* ����:		char				[IN] *pszDrvName
*			BOOLEAN				[IN] storeOrCheck
*			WCHAR				[I/O] *pwszDrvName
*			PDEVICE_OBJECT		[I/O] pDrvObj
*			char				[IN] *pszPath
*			ULONG				[IN] ulBase
*			ULONG				[IN] ulSize
*
* ��������:	�洢�����ں�ģ��
*
* ����ֵ:	BOOLEAN
*
=========================================================================*/
BOOLEAN StoreOrCheckKrnlMod(BOOLEAN storeOrCheck, PWCHAR pszDrvName, ULONG ulImageBase, ULONG ulImageSize, ULONG pDrvObject, PWCHAR pwDosPath, ULONG LoadOrderIndex, PULONG numOfDrv, PDRVMOD_ITEM pDrvItem)
{
	ULONG	i=0;

	//��ַor��С������<=0
	if (ulImageBase <= 0  ||  ulImageSize <= 0)
		return FALSE;

	//�洢
	if (storeOrCheck)
	{
		//Drv����
		(*numOfDrv)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			//��Eproc��������iָ��ĵ�ַΪ�յĵط��洢
			if ( pDrvItem[i].ulBaseAddr == 0)
			{
				/*�洢����ģ����Ϣ*/
				pDrvItem[i].hideFlags=FALSE;					//���ر��
				pDrvItem[i].ulBaseAddr=ulImageBase;				//��ַ
				pDrvItem[i].ulImageSize=ulImageSize;			//��С
				pDrvItem[i].pDrvObject=pDrvObject;				//��������
				pDrvItem[i].ulLoadOrder=LoadOrderIndex;			//����˳��
				wcscpy(pDrvItem[i].pwszDrvName, pszDrvName);	//������
				wcscpy(pDrvItem[i].pwszDrvPath, pwDosPath);		//����·��
//				DbgPrint("������:%ws \t ��������:0x%08X\n", pDrvItem[i].pwszDrvName, pDrvItem[i].pDrvObject);

				return TRUE;
			}
		}
	}
	else	//���
	{
		for (i=0; i<(*numOfDrv); i++)
		{
			//�Ѵ����򷵻�
			if (ulImageBase == pDrvItem[i].ulBaseAddr)
				return TRUE;
		}

		//�������أ�Drv��������
		(*numOfDrv)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			if (pDrvItem[i].ulBaseAddr == 0)
			{
				/*���뵽��¼��*/
				pDrvItem[i].hideFlags=TRUE;						//���ر��
				pDrvItem[i].ulBaseAddr=ulImageBase;				//��ַ
				pDrvItem[i].ulImageSize=ulImageSize;			//��С
				pDrvItem[i].pDrvObject=pDrvObject;				//��������
				pDrvItem[i].ulLoadOrder=LoadOrderIndex;			//����˳��
				wcscpy(pDrvItem[i].pwszDrvName, pszDrvName);	//������
				wcscpy(pDrvItem[i].pwszDrvPath, pwDosPath);		//����·��
			}
		}
	}

	return TRUE;
}