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

	//按drv个数申请内存
	pDrvItem=(DRVMOD_ITEM *)ExAllocatePoolWithTag(PagedPool, sizeof(DRVMOD_ITEM)*MAX_DRIVER_NUM*2, 'drv');
	if (pDrvItem)
	{
		//内存清零
		RtlZeroMemory(pDrvItem, sizeof(DRVMOD_ITEM)*MAX_DRIVER_NUM*2);
		//ZwQuerySysInfo枚举内核模块
		status=EnumKrnlModByZwQuerySysInfo(numOfDrv, pDrvItem);
		//检测
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

	//获取大小
	status=ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &dwRetSize);
	//申请内存
	pDrvtmp = ExAllocatePoolWithTag(PagedPool, dwRetSize, 'tmp');		//dwRetSize 需要的大小
	if (pDrvtmp)
	{
		//再次执行,将枚举结果放到指定的内存区域
		status=ZwQuerySystemInformation(SystemModuleInformation, pDrvtmp, dwRetSize, NULL);
		if ( NT_SUCCESS(status) )
		{
			//模块数
			drvNums = *( (PULONG)pDrvtmp );
			//模块链表头指针
			pDrvModInfo=(PSYSTEM_MODULE_INFORMATION)( (PULONG)pDrvtmp + 1 );

			for (i=0; i< drvNums; i++)
			{
//				DbgPrint("驱动名:%s\n", pDrvModInfo[i].imagePath+pDrvModInfo[i].imageNameOffset);
				//获取驱动对象
				GetDrvObjectByName(pDrvModInfo[i].imagePath+pDrvModInfo[i].imageNameOffset, &pDrvObject);
				//转换驱动名
				CharStringToWideChar(TRUE, pDrvModInfo[i].imagePath+pDrvModInfo[i].imageNameOffset, wName);
				//转换驱动路径
				CharStringToWideChar(TRUE, pDrvModInfo[i].imagePath, wPath);

				//存储驱动信息
				StoreOrCheckKrnlMod(TRUE, wName, (ULONG)pDrvModInfo[i].ImageBase, pDrvModInfo[i].ImageSize, pDrvObject, wPath, pDrvModInfo[i].LoadOrderIndex, numOfDrv, pDrvItem);
			}
			KdPrint(("EnumKrnlModByZwQuerySysInfo()->驱动模块:%d个\n", drvNums ));
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

			//转换驱动名
			CharStringToWideChar(FALSE, drvName, LdrEntry->BaseDllName.Buffer);
//			wcsncpy( wDrvPath, LdrEntry->FullDllName.Buffer, LdrEntry->FullDllName.Length );
//			memcpy( wDrvPath, LdrEntry->FullDllName.Buffer, LdrEntry->FullDllName.Length );

			//获取驱动对象
			GetDrvObjectByName(drvName, &pDrvObject);
			KdPrint(( "StoreOrCheckDrvMod(0x%08X, %ws)\n", (ULONG)LdrEntry->DllBase, LdrEntry->FullDllName.Buffer));

			//存储枚举到的驱动信息
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
	//驱动对象
	PDRIVER_OBJECT	pDrvObj=NULL;

	//清零指针内容
	*pDrvObject = 0;

	//非空则转Unicode驱动名
	if (pszDrvName[0] != '\0')
	{
		strcpy(dev, "\\Driver\\");
		// "\Driver\xxx.sys"
		strcat(dev, pszDrvName);

		strcpy(fileDev, "\\FileSystem\\");
		//"\FileSystem\xxx.sys"
		strcat(fileDev, pszDrvName);

		//去除后缀名(减去后4个字符，"\Driver\xxx")
		RtlMoveMemory(sym, dev, strlen(dev)-4);
		//去除后缀名(减去后4个字符，"\FileSystem\xxx")
		RtlMoveMemory(fileSym, fileDev, strlen(dev)-4);

		RtlInitAnsiString(&ansiDev, sym);
		RtlInitAnsiString(&ansFileDev, fileSym);
		//转Unicode对象名
		RtlAnsiStringToUnicodeString(&uniObjName, &ansiDev, TRUE);
		//转Unicode对象名
		RtlAnsiStringToUnicodeString(&uniFileDev, &ansFileDev, TRUE);
	}

	//引用对象通过名字("\Driver\xxx")
	status=ObReferenceObjectByName(&uniObjName, OBJ_CASE_INSENSITIVE, NULL, FILE_ALL_ACCESS, IoDriverObjectType, KernelMode, NULL, (PVOID *)&pDrvObj);
	if ( !NT_SUCCESS(status) )
	{
		//引用对象通过名字("\FileSystem\xxx")
		status=ObReferenceObjectByName(&uniFileDev, OBJ_CASE_INSENSITIVE, NULL, FILE_ALL_ACCESS, IoDriverObjectType, KernelMode, NULL, (PVOID *)&pDrvObj);
		if ( !NT_SUCCESS(status) )
		{
//			DbgPrint("ObReferenceObjectByName(\"\\Driver\\xxx or \\FileSystem\\xxx\") Failed! Status: 驱动名:%ws \t 0x%x\n", uniObjName.Buffer, status);
			return status;
		}
	}

//	DbgPrint("驱动名:%ws \t 驱动对象:0x%08X\n", uniObjName.Buffer, pDrvObj);
	*pDrvObject=(ULONG)pDrvObj;
	//解除对象引用
	ObDereferenceObject(pDrvObj);
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
BOOLEAN StoreOrCheckKrnlMod(BOOLEAN storeOrCheck, PWCHAR pszDrvName, ULONG ulImageBase, ULONG ulImageSize, ULONG pDrvObject, PWCHAR pwDosPath, ULONG LoadOrderIndex, PULONG numOfDrv, PDRVMOD_ITEM pDrvItem)
{
	ULONG	i=0;

	//基址or大小都不能<=0
	if (ulImageBase <= 0  ||  ulImageSize <= 0)
		return FALSE;

	//存储
	if (storeOrCheck)
	{
		//Drv计数
		(*numOfDrv)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			//找Eproc数组索引i指向的地址为空的地方存储
			if ( pDrvItem[i].ulBaseAddr == 0)
			{
				/*存储驱动模块信息*/
				pDrvItem[i].hideFlags=FALSE;					//隐藏标记
				pDrvItem[i].ulBaseAddr=ulImageBase;				//基址
				pDrvItem[i].ulImageSize=ulImageSize;			//大小
				pDrvItem[i].pDrvObject=pDrvObject;				//驱动对象
				pDrvItem[i].ulLoadOrder=LoadOrderIndex;			//加载顺序
				wcscpy(pDrvItem[i].pwszDrvName, pszDrvName);	//驱动名
				wcscpy(pDrvItem[i].pwszDrvPath, pwDosPath);		//驱动路径
//				DbgPrint("驱动名:%ws \t 驱动对象:0x%08X\n", pDrvItem[i].pwszDrvName, pDrvItem[i].pDrvObject);

				return TRUE;
			}
		}
	}
	else	//检测
	{
		for (i=0; i<(*numOfDrv); i++)
		{
			//已存在则返回
			if (ulImageBase == pDrvItem[i].ulBaseAddr)
				return TRUE;
		}

		//发现隐藏，Drv计数递增
		(*numOfDrv)++;

		for (i=0; i<MAX_Dll_NUM*2; i++)
		{
			if (pDrvItem[i].ulBaseAddr == 0)
			{
				/*加入到记录集*/
				pDrvItem[i].hideFlags=TRUE;						//隐藏标记
				pDrvItem[i].ulBaseAddr=ulImageBase;				//基址
				pDrvItem[i].ulImageSize=ulImageSize;			//大小
				pDrvItem[i].pDrvObject=pDrvObject;				//驱动对象
				pDrvItem[i].ulLoadOrder=LoadOrderIndex;			//加载顺序
				wcscpy(pDrvItem[i].pwszDrvName, pszDrvName);	//驱动名
				wcscpy(pDrvItem[i].pwszDrvPath, pwDosPath);		//驱动路径
			}
		}
	}

	return TRUE;
}