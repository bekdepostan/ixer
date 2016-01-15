//////////////////////////////////////////////////////////////////////////
// KrnlMod.c

#include "precomp.h"
#include "ix.h"
#include "Dll.h"
#include "..\Common\DataStruct.h"
#include "Process.h"
#include "KrnlMod.h"


extern PDRV_HASH pDrvObjHash;

NTSTATUS GetKrnlModuleList(PULONG numOfDrv)
{
	NTSTATUS status=STATUS_SUCCESS;
	int		i;

	//�жϹ�ϣ���Ƿ����
	if (pDrvObjHash->drvObjHash)
	{
		//��ʼ��
		pDrvObjHash->drvNums=MAX_DRIVER_NUM;
		for (i=0; i<MAX_DRIVER_NUM; i++)
		{
			//��ϣ������
			RtlZeroMemory(&pDrvObjHash->drvObjHash[i], sizeof(pDrvObjHash->drvObjHash[i]));
		}
	}
	else
		return STATUS_UNSUCCESSFUL;

	//ZwQuerySysInfoö���ں�ģ��
	status=EnumKrnlModByZwQuerySysInfo( numOfDrv );
	//���

	return status;
}

NTSTATUS EnumKrnlModByZwQuerySysInfo( PULONG numOfDrv )
{
	NTSTATUS status=STATUS_SUCCESS;
	PSYSTEM_MODULE_INFORMATION pSysModInfo=NULL;
	PVOID	pBuftmp=NULL;
	ULONG	dwRetSize=0;
	ULONG	i;

	PULONG ulDrvObj=NULL;
	PULONG ulEntryPoint=NULL;
	PULONG ulUnLoadAddr=NULL;

	//��ȡ��С
	status=ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &dwRetSize);
	//�����ڴ�
	pBuftmp = ExAllocatePoolWithTag(NonPagedPool, dwRetSize, 'tmp');		//dwRetSize ��Ҫ�Ĵ�С
	if (pBuftmp)
	{
		//�ٴ�ִ��,��ö�ٽ���ŵ�ָ�����ڴ�����
		status = ZwQuerySystemInformation(SystemModuleInformation, pBuftmp, dwRetSize, NULL);
		if ( NT_SUCCESS(status) )
		{
			//ģ����
			numOfDrv = (PULONG)pBuftmp;
			//ģ���б�ͷָ��
			pSysModInfo=(PSYSTEM_MODULE_INFORMATION)( (PULONG)pBuftmp + 1 );

			for (i=0; i< (*numOfDrv); i++)
			{
				DbgPrint("������:%s\n", pSysModInfo->imagePath+pSysModInfo->imageNameOffset);
				StoreOrCheckKrnlMod(pSysModInfo->imagePath+pSysModInfo->imageNameOffset, TRUE, NULL, NULL, pSysModInfo->imagePath, (ULONG)pSysModInfo->ImageBase, pSysModInfo->ImageSize, pSysModInfo->LoadCount);
				pSysModInfo++;
			}
			KdPrint(("EnumKrnlModByZwQuerySysInfo()->����ģ��:%d��\n", (*numOfDrv) ));
		}
	}

	if (pBuftmp)
		ExFreePool(pBuftmp);

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
NTSTATUS StoreOrCheckKrnlMod( char *pszDrvName, BOOLEAN storeOrCheck, WCHAR *pwszDrvName, PDEVICE_OBJECT pDrvObject, char *pszPath, ULONG ulBase, ULONG ulSize, ULONG LoadCount )
{
	NTSTATUS status=STATUS_SUCCESS;
	UNICODE_STRING uniObjName, uniName, uniPath;
	ANSI_STRING ansi, ansiName, ansiPath;

	PDRIVER_OBJECT pDrvObj=NULL;	//��������
	PLDR_DATA_TABLE_ENTRY pLdr=NULL;

	char name[256]={0};
	char name2[256]={0};

	WCHAR wName[256]={'\0'};
	WCHAR wName2[256]={'\0'};
	ULONG	index, j, k, l;

	//DrvObj��������
	if (pDrvObject != NULL)
	{
		ObReferenceObject(pDrvObject);	//���ö���
		goto Chk;
	}

	//�ж��������Ƿ�Ϊ��
	if (pszDrvName != '\0')
	{
		RtlInitAnsiString(&ansi, pszDrvName);
		//תUnicode�豸��
		RtlAnsiStringToUnicodeString(&uniName, &ansi, TRUE);
	}

	//�ж��������Ƿ�Ϊ��
	if (pszPath)
	{
		RtlInitAnsiString(&ansiPath, pszPath);
		//תUnicode·��
		RtlAnsiStringToUnicodeString(&uniPath, &ansiPath, TRUE);
	}

	//�洢����
	if (storeOrCheck)
	{
		strcpy(name, "\\driver\\");
		strcat(name, pszDrvName);		// "\driver\xxx.sys"

		//ȥ����׺��(��ȥ��4���ַ���"\driver\xxx")
		RtlMoveMemory(name2, name, strlen(name)-4);
		RtlInitAnsiString(&ansiName, name2);
		//תUnicode������
		RtlAnsiStringToUnicodeString(&uniObjName, &ansiName, TRUE);
	}
	else
	{
		//�ж��������Ƿ�Ϊ��
		if (pwszDrvName == '\0')
		{
			return STATUS_UNSUCCESSFUL;
		}

		__try
		{
			wcscpy(wName, L"\\driver\\");
			wcscat(wName, pwszDrvName);		// "\driver\xxx.sys"	

			//
			RtlMoveMemory(wName2, wName, wcslen(wName) * 2 +1);
			RtlInitUnicodeString(&uniObjName, wName2);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{}
	}

//	status=IoGetDeviceObjectPointer(xxx);
	//���ö���ͨ������
	status=ObReferenceObjectByName(&uniObjName, OBJ_CASE_INSENSITIVE, NULL, FILE_ALL_ACCESS, IoDriverObjectType, KernelMode, NULL, (PVOID *)&pDrvObj);
	if ( NT_SUCCESS(status) )
	{
		if (pDrvObj->DriverSection)
		{
			pLdr = (PLDR_DATA_TABLE_ENTRY)(pDrvObj->DriverSection);
		}

		//�洢����
		if (storeOrCheck)
		{
			//���ù�ϣ��������index
			index = HashCalc(pDrvObj);

Again:
			if (index<MAX_DRIVER_NUM)
			{
				//��ײ��
				if (pDrvObjHash->drvObjHash[index].pDrvObject != 0)
				{
					//���µ��ù�ϣ��������index
					index=(index + 1)%1024;
					goto Again;
				}
				else
				{
					//�����ϣ��
					pDrvObjHash->drvObjHash[index].hideFlags=FALSE;								//���ر��
					pDrvObjHash->drvObjHash[index].pDrvObject=(ULONG)pDrvObj;					//��������
					pDrvObjHash->drvObjHash[index].ulImageSize=pDrvObj->DriverSize;				//������С
					pDrvObjHash->drvObjHash[index].ulUnLoadAddr=(ULONG)pDrvObj->DriverUnload;	//ж������
					pDrvObjHash->drvObjHash[index].ulLoadedCount=LoadCount;						//����˳��

					if (pLdr)
					{
						pDrvObjHash->drvObjHash[index].ulBaseAddr=(ULONG)pLdr->DllBase;			//������ַ
						pDrvObjHash->drvObjHash[index].ulEntryPoint=(ULONG)pLdr->EntryPoint;	//��ڵ�
						//������
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pLdr->BaseDllName.Buffer, (pLdr->BaseDllName.Length));
						//����·��
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pLdr->FullDllName.Buffer, (pLdr->FullDllName.Length));
					}
					else
					{
						//LdrΪ�գ����ô����������
						if (pwszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pwszDrvName, wcslen(pwszDrvName));	//������
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pwszDrvName, wcslen(pwszDrvName));	//����·��
						}
						else if (pszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, uniName.Buffer, uniName.Length);		//������
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, uniPath.Buffer, uniPath.Length);		//����·��
						}
					}
				}
			}

			return STATUS_SUCCESS;
		}
		else	//�����������
		{
Chk:
			//���ù�ϣ��������index
			index = HashCalc(pDrvObj);

			if (index<MAX_DRIVER_NUM)
			{
				for (j=0; j<MAX_DRIVER_NUM; j++)
				{
					//��������ϸ��Ϣ
					if ( (PDRIVER_OBJECT)pDrvObjHash->drvObjHash[index].pDrvObject == (PDRIVER_OBJECT)1 )
					{
						if (pDrvObjHash->drvObjHash[index].ulBaseAddr == pDrvObj->Size)
							//fix Me~
							return STATUS_SUCCESS;
					}
				}

				if (pDrvObjHash->drvObjHash[index].pDrvObject == 0)
				{
					//��¼��������
					pDrvObjHash->drvObjHash[index].hideFlags=TRUE;								//���ر��
					pDrvObjHash->drvObjHash[index].pDrvObject=(ULONG)pDrvObj;					//��������
					pDrvObjHash->drvObjHash[index].ulImageSize=pDrvObj->DriverSize;				//������С
					pDrvObjHash->drvObjHash[index].ulUnLoadAddr=(ULONG)pDrvObj->DriverUnload;	//ж������
					pDrvObjHash->drvObjHash[index].ulLoadedCount=LoadCount;						//����˳��

					if (pLdr)
					{
						pDrvObjHash->drvObjHash[index].ulBaseAddr=(ULONG)pLdr->DllBase;			//������ַ
						pDrvObjHash->drvObjHash[index].ulEntryPoint=(ULONG)pLdr->EntryPoint;	//��ڵ�
						//������
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pLdr->BaseDllName.Buffer, (pLdr->BaseDllName.Length));
						//����·��
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pLdr->FullDllName.Buffer, (pLdr->FullDllName.Length));
					}
					else
					{
						//LdrΪ�գ����ô����������
						if (pwszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pwszDrvName, wcslen(pwszDrvName));	//������
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pwszDrvName, wcslen(pwszDrvName));	//����·��
						}
						else if (pszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, uniName.Buffer, uniName.Length);		//������
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, uniPath.Buffer, uniPath.Length);		//����·��
						}
					}

					return STATUS_SUCCESS;
				}
				else
				{
					//�Ѵ���
					if ( (PDRIVER_OBJECT)pDrvObjHash->drvObjHash[index].pDrvObject == pDrvObj)
					{
						ObDereferenceObject(pDrvObj);
						return STATUS_SUCCESS;
					}
					else
					{
						for (k=MAX_DRIVER_NUM; k>0; k--)
						{
							//���ﻹ��Ϊ�����ֲ��ȣ�˵����ײ��
							if ( (PDRIVER_OBJECT)pDrvObjHash->drvObjHash[index].pDrvObject == (pDrvObj) )
							{
								ObDereferenceObject(pDrvObj);
								return STATUS_UNSUCCESSFUL;
							}
							else if (pDrvObjHash->drvObjHash[index].pDrvObject == 0)
							{
								goto hide;
							}

							index=(index + 1)%1024;
							if (index>MAX_DRIVER_NUM)
							{
								//Hash overflow~
								ObDereferenceObject(pDrvObj);
								return STATUS_UNSUCCESSFUL;
							}
						}

hide:
						//��¼��������
						pDrvObjHash->drvObjHash[index].hideFlags=TRUE;								//���ر��
						pDrvObjHash->drvObjHash[index].pDrvObject=(ULONG)pDrvObj;					//��������
						pDrvObjHash->drvObjHash[index].ulImageSize=pDrvObj->DriverSize;				//������С
						pDrvObjHash->drvObjHash[index].ulUnLoadAddr=(ULONG)pDrvObj->DriverUnload;	//ж������
						pDrvObjHash->drvObjHash[index].ulLoadedCount=LoadCount;						//����˳��

						if (pLdr)
						{
							pDrvObjHash->drvObjHash[index].ulBaseAddr=(ULONG)pLdr->DllBase;			//������ַ
							pDrvObjHash->drvObjHash[index].ulEntryPoint=(ULONG)pLdr->EntryPoint;	//��ڵ�
							//������
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pLdr->BaseDllName.Buffer, (pLdr->BaseDllName.Length));
							//����·��
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pLdr->FullDllName.Buffer, (pLdr->FullDllName.Length));
						}
						else
						{
							//LdrΪ�գ����ô����������
							if (pwszDrvName)
							{
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pwszDrvName, wcslen(pwszDrvName));	//������
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pwszDrvName, wcslen(pwszDrvName));	//����·��
							}
							else if (pszDrvName)
							{
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, uniName.Buffer, uniName.Length);		//������
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, uniPath.Buffer, uniPath.Length);		//����·��
							}
						}

						return STATUS_SUCCESS;
					}
				}
			}
		}

	if (pDrvObj)
		ObDereferenceObject(pDrvObj);

		return STATUS_SUCCESS;
	}
	else	//���ö���ʧ��
	{
		for (l=0; l<MAX_DRIVER_NUM; l++)
		{
			if (pDrvObjHash->drvObjHash[l].pDrvObject == 0)
			{
				index=l;
				goto Fail;
			}
		}

Fail:
		(PDRIVER_OBJECT)pDrvObjHash->drvObjHash[index].pDrvObject=(PDRIVER_OBJECT)1;
		pDrvObjHash->drvObjHash[index].hideFlags=FALSE;
		pDrvObjHash->drvObjHash[index].ulBaseAddr=ulBase;
		pDrvObjHash->drvObjHash[index].ulImageSize=ulSize;
		pDrvObjHash->drvObjHash[index].ulEntryPoint=0;
		pDrvObjHash->drvObjHash[index].ulUnLoadAddr=0;
		pDrvObjHash->drvObjHash[index].ulLoadedCount=LoadCount;
		RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, uniName.Buffer, uniName.Length);
		RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, uniPath.Buffer, uniPath.Length);

		return STATUS_SUCCESS;
	}
}

//��ϣɢ�к�������DriverObjectΪ�洢����
ULONG HashCalc( PDRIVER_OBJECT pDrvObjectAddr )
{
	ULONG index;

	__asm {
			mov eax, pDrvObjectAddr
			and eax, 0x000fffff			//ֻҪ��5λ��
			mov pDrvObjectAddr, eax
		 }

	//����������
	index=(ULONG)(pDrvObjectAddr) % 1019;
	return index;
}

//��������������
VOID iObDereferObject(PDRIVER_OBJECT pDrvObject)
{
	if (pDrvObject != (PDRIVER_OBJECT)1)
	{
		if (IsValidAddr( (ULONG)pDrvObject ) == VALID_PAGE)
		{
			__try
			{
				ObDereferenceObject(pDrvObject);	//�����ö���
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{}
		}
	}
}