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

	//判断哈希表是否就绪
	if (pDrvObjHash->drvObjHash)
	{
		//初始化
		pDrvObjHash->drvNums=MAX_DRIVER_NUM;
		for (i=0; i<MAX_DRIVER_NUM; i++)
		{
			//哈希表清零
			RtlZeroMemory(&pDrvObjHash->drvObjHash[i], sizeof(pDrvObjHash->drvObjHash[i]));
		}
	}
	else
		return STATUS_UNSUCCESSFUL;

	//ZwQuerySysInfo枚举内核模块
	status=EnumKrnlModByZwQuerySysInfo( numOfDrv );
	//检测

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

	//获取大小
	status=ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &dwRetSize);
	//申请内存
	pBuftmp = ExAllocatePoolWithTag(NonPagedPool, dwRetSize, 'tmp');		//dwRetSize 需要的大小
	if (pBuftmp)
	{
		//再次执行,将枚举结果放到指定的内存区域
		status = ZwQuerySystemInformation(SystemModuleInformation, pBuftmp, dwRetSize, NULL);
		if ( NT_SUCCESS(status) )
		{
			//模块数
			numOfDrv = (PULONG)pBuftmp;
			//模块列表头指针
			pSysModInfo=(PSYSTEM_MODULE_INFORMATION)( (PULONG)pBuftmp + 1 );

			for (i=0; i< (*numOfDrv); i++)
			{
				DbgPrint("驱动名:%s\n", pSysModInfo->imagePath+pSysModInfo->imageNameOffset);
				StoreOrCheckKrnlMod(pSysModInfo->imagePath+pSysModInfo->imageNameOffset, TRUE, NULL, NULL, pSysModInfo->imagePath, (ULONG)pSysModInfo->ImageBase, pSysModInfo->ImageSize, pSysModInfo->LoadCount);
				pSysModInfo++;
			}
			KdPrint(("EnumKrnlModByZwQuerySysInfo()->驱动模块:%d个\n", (*numOfDrv) ));
		}
	}

	if (pBuftmp)
		ExFreePool(pBuftmp);

	return status;
}

/*========================================================================
*
* 函数名:	StoreOrCheckKrnlMod
*
* 参数:		char				[IN] *pszDrvName
*			BOOLEAN				[IN] storeOrCheck
*			WCHAR				[I/O] *pwszDrvName
*			PDEVICE_OBJECT		[I/O] pDrvObj
*			char				[IN] *pszPath
*			ULONG				[IN] ulBase
*			ULONG				[IN] ulSize
*
* 功能描述:	存储或检测内核模块
*
* 返回值:	BOOLEAN
*
=========================================================================*/
NTSTATUS StoreOrCheckKrnlMod( char *pszDrvName, BOOLEAN storeOrCheck, WCHAR *pwszDrvName, PDEVICE_OBJECT pDrvObject, char *pszPath, ULONG ulBase, ULONG ulSize, ULONG LoadCount )
{
	NTSTATUS status=STATUS_SUCCESS;
	UNICODE_STRING uniObjName, uniName, uniPath;
	ANSI_STRING ansi, ansiName, ansiPath;

	PDRIVER_OBJECT pDrvObj=NULL;	//驱动对象
	PLDR_DATA_TABLE_ENTRY pLdr=NULL;

	char name[256]={0};
	char name2[256]={0};

	WCHAR wName[256]={'\0'};
	WCHAR wName2[256]={'\0'};
	ULONG	index, j, k, l;

	//DrvObj已有则检测
	if (pDrvObject != NULL)
	{
		ObReferenceObject(pDrvObject);	//引用对象
		goto Chk;
	}

	//判断驱动名是否为空
	if (pszDrvName != '\0')
	{
		RtlInitAnsiString(&ansi, pszDrvName);
		//转Unicode设备名
		RtlAnsiStringToUnicodeString(&uniName, &ansi, TRUE);
	}

	//判断驱动名是否为空
	if (pszPath)
	{
		RtlInitAnsiString(&ansiPath, pszPath);
		//转Unicode路径
		RtlAnsiStringToUnicodeString(&uniPath, &ansiPath, TRUE);
	}

	//存储或检测
	if (storeOrCheck)
	{
		strcpy(name, "\\driver\\");
		strcat(name, pszDrvName);		// "\driver\xxx.sys"

		//去除后缀名(减去后4个字符，"\driver\xxx")
		RtlMoveMemory(name2, name, strlen(name)-4);
		RtlInitAnsiString(&ansiName, name2);
		//转Unicode对象名
		RtlAnsiStringToUnicodeString(&uniObjName, &ansiName, TRUE);
	}
	else
	{
		//判断驱动名是否为空
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
	//引用对象通过名字
	status=ObReferenceObjectByName(&uniObjName, OBJ_CASE_INSENSITIVE, NULL, FILE_ALL_ACCESS, IoDriverObjectType, KernelMode, NULL, (PVOID *)&pDrvObj);
	if ( NT_SUCCESS(status) )
	{
		if (pDrvObj->DriverSection)
		{
			pLdr = (PLDR_DATA_TABLE_ENTRY)(pDrvObj->DriverSection);
		}

		//存储或检测
		if (storeOrCheck)
		{
			//调用哈希函数计算index
			index = HashCalc(pDrvObj);

Again:
			if (index<MAX_DRIVER_NUM)
			{
				//碰撞了
				if (pDrvObjHash->drvObjHash[index].pDrvObject != 0)
				{
					//重新调用哈希函数计算index
					index=(index + 1)%1024;
					goto Again;
				}
				else
				{
					//存入哈希表
					pDrvObjHash->drvObjHash[index].hideFlags=FALSE;								//隐藏标记
					pDrvObjHash->drvObjHash[index].pDrvObject=(ULONG)pDrvObj;					//驱动对象
					pDrvObjHash->drvObjHash[index].ulImageSize=pDrvObj->DriverSize;				//驱动大小
					pDrvObjHash->drvObjHash[index].ulUnLoadAddr=(ULONG)pDrvObj->DriverUnload;	//卸载例程
					pDrvObjHash->drvObjHash[index].ulLoadedCount=LoadCount;						//加载顺序

					if (pLdr)
					{
						pDrvObjHash->drvObjHash[index].ulBaseAddr=(ULONG)pLdr->DllBase;			//驱动基址
						pDrvObjHash->drvObjHash[index].ulEntryPoint=(ULONG)pLdr->EntryPoint;	//入口点
						//驱动名
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pLdr->BaseDllName.Buffer, (pLdr->BaseDllName.Length));
						//驱动路径
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pLdr->FullDllName.Buffer, (pLdr->FullDllName.Length));
					}
					else
					{
						//Ldr为空，则用传入的驱动名
						if (pwszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pwszDrvName, wcslen(pwszDrvName));	//驱动名
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pwszDrvName, wcslen(pwszDrvName));	//驱动路径
						}
						else if (pszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, uniName.Buffer, uniName.Length);		//驱动名
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, uniPath.Buffer, uniPath.Length);		//驱动路径
						}
					}
				}
			}

			return STATUS_SUCCESS;
		}
		else	//检测隐藏驱动
		{
Chk:
			//调用哈希函数计算index
			index = HashCalc(pDrvObj);

			if (index<MAX_DRIVER_NUM)
			{
				for (j=0; j<MAX_DRIVER_NUM; j++)
				{
					//驱动无详细信息
					if ( (PDRIVER_OBJECT)pDrvObjHash->drvObjHash[index].pDrvObject == (PDRIVER_OBJECT)1 )
					{
						if (pDrvObjHash->drvObjHash[index].ulBaseAddr == pDrvObj->Size)
							//fix Me~
							return STATUS_SUCCESS;
					}
				}

				if (pDrvObjHash->drvObjHash[index].pDrvObject == 0)
				{
					//记录隐藏驱动
					pDrvObjHash->drvObjHash[index].hideFlags=TRUE;								//隐藏标记
					pDrvObjHash->drvObjHash[index].pDrvObject=(ULONG)pDrvObj;					//驱动对象
					pDrvObjHash->drvObjHash[index].ulImageSize=pDrvObj->DriverSize;				//驱动大小
					pDrvObjHash->drvObjHash[index].ulUnLoadAddr=(ULONG)pDrvObj->DriverUnload;	//卸载例程
					pDrvObjHash->drvObjHash[index].ulLoadedCount=LoadCount;						//加载顺序

					if (pLdr)
					{
						pDrvObjHash->drvObjHash[index].ulBaseAddr=(ULONG)pLdr->DllBase;			//驱动基址
						pDrvObjHash->drvObjHash[index].ulEntryPoint=(ULONG)pLdr->EntryPoint;	//入口点
						//驱动名
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pLdr->BaseDllName.Buffer, (pLdr->BaseDllName.Length));
						//驱动路径
						RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pLdr->FullDllName.Buffer, (pLdr->FullDllName.Length));
					}
					else
					{
						//Ldr为空，则用传入的驱动名
						if (pwszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pwszDrvName, wcslen(pwszDrvName));	//驱动名
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pwszDrvName, wcslen(pwszDrvName));	//驱动路径
						}
						else if (pszDrvName)
						{
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, uniName.Buffer, uniName.Length);		//驱动名
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, uniPath.Buffer, uniPath.Length);		//驱动路径
						}
					}

					return STATUS_SUCCESS;
				}
				else
				{
					//已存在
					if ( (PDRIVER_OBJECT)pDrvObjHash->drvObjHash[index].pDrvObject == pDrvObj)
					{
						ObDereferenceObject(pDrvObj);
						return STATUS_SUCCESS;
					}
					else
					{
						for (k=MAX_DRIVER_NUM; k>0; k--)
						{
							//这里还不为空且又不等！说明碰撞了
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
						//记录隐藏驱动
						pDrvObjHash->drvObjHash[index].hideFlags=TRUE;								//隐藏标记
						pDrvObjHash->drvObjHash[index].pDrvObject=(ULONG)pDrvObj;					//驱动对象
						pDrvObjHash->drvObjHash[index].ulImageSize=pDrvObj->DriverSize;				//驱动大小
						pDrvObjHash->drvObjHash[index].ulUnLoadAddr=(ULONG)pDrvObj->DriverUnload;	//卸载例程
						pDrvObjHash->drvObjHash[index].ulLoadedCount=LoadCount;						//加载顺序

						if (pLdr)
						{
							pDrvObjHash->drvObjHash[index].ulBaseAddr=(ULONG)pLdr->DllBase;			//驱动基址
							pDrvObjHash->drvObjHash[index].ulEntryPoint=(ULONG)pLdr->EntryPoint;	//入口点
							//驱动名
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pLdr->BaseDllName.Buffer, (pLdr->BaseDllName.Length));
							//驱动路径
							RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pLdr->FullDllName.Buffer, (pLdr->FullDllName.Length));
						}
						else
						{
							//Ldr为空，则用传入的驱动名
							if (pwszDrvName)
							{
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, pwszDrvName, wcslen(pwszDrvName));	//驱动名
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, pwszDrvName, wcslen(pwszDrvName));	//驱动路径
							}
							else if (pszDrvName)
							{
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvName, uniName.Buffer, uniName.Length);		//驱动名
								RtlCopyMemory(pDrvObjHash->drvObjHash[index].lpwszDrvPath, uniPath.Buffer, uniPath.Length);		//驱动路径
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
	else	//引用对象失败
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

//哈希散列函数，以DriverObject为存储对象
ULONG HashCalc( PDRIVER_OBJECT pDrvObjectAddr )
{
	ULONG index;

	__asm {
			mov eax, pDrvObjectAddr
			and eax, 0x000fffff			//只要后5位？
			mov pDrvObjectAddr, eax
		 }

	//除留余数法
	index=(ULONG)(pDrvObjectAddr) % 1019;
	return index;
}

//解引用驱动对象
VOID iObDereferObject(PDRIVER_OBJECT pDrvObject)
{
	if (pDrvObject != (PDRIVER_OBJECT)1)
	{
		if (IsValidAddr( (ULONG)pDrvObject ) == VALID_PAGE)
		{
			__try
			{
				ObDereferenceObject(pDrvObject);	//解引用对象
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{}
		}
	}
}