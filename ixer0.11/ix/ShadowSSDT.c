//////////////////////////////////////////////////////////////////////////
// ShadowSSDT.c

#include "precomp.h"
#include "drvMod.h"
#include "..\Common\Surface.h"
#include "common.h"
#include "ssdt.h"
#include "ShadowSSDT.h"



/*========================================================================
*
* 函数名:	GetSSDTShadowNumber
*
* 参数:		
*
* 功能描述:	获取服务个数
*
* 返回值:	ULONG
*
=========================================================================*/
ULONG
GetSSDTShadowNumber( VOID )
{
	PSYSTEM_SERVICE_TABLE_SHADOW KeServiceDescriptorTableShadow = NULL;

	KeServiceDescriptorTableShadow = (PSYSTEM_SERVICE_TABLE_SHADOW)GetSSDTShadowTable();
	if ( !KeServiceDescriptorTableShadow )
		return 0;

	return KeServiceDescriptorTableShadow->ServiceLimit;
}

/*========================================================================

/* 获取ShadowSSDT表地址偏移 */
ULONG GetSSDTShadowTableAddr()
{
	ULONG		ktrdCid;
	NTSTATUS	status;
	ULONG		ethread;
	ULONG		shadowTableAddr=0;
	ULONG		i=0;

	/* 尝试枚举系统线程获取 KTHREAD */
	for( ktrdCid = 8; ktrdCid < 32768; ktrdCid += 4)
	{
		status = PsLookupThreadByThreadId( (HANDLE)ktrdCid, &(PETHREAD)ethread );
		if( NT_SUCCESS(status) )
		{	// 
			if( *(PULONG)( ethread + 0x0e0 )  !=  (ULONG)KeServiceDescriptorTable )
			{
				/* 获取 KTHREAD.ServiceTable 指针值 */
				shadowTableAddr =  *(PULONG)( ethread + 0x0e0 );
				break;
			}
		}
	}

	return shadowTableAddr;
}

//解析ShadowSSDT表的地址结构
PSYSTEM_SERVICE_TABLE_SHADOW GetSSDTShadowTable()
{
	PSYSTEM_SERVICE_TABLE_SHADOW KeServiceDescriptorTableShadow = NULL;

	//SSDTShadowTable地址偏移 强制转换成 KeServiceDescriptorTableShadow
	KeServiceDescriptorTableShadow = (PSYSTEM_SERVICE_TABLE_SHADOW)GetSSDTShadowTableAddr();
	if ( !KeServiceDescriptorTableShadow )
		return NULL;

	KeServiceDescriptorTableShadow = &KeServiceDescriptorTableShadow[1];
	return KeServiceDescriptorTableShadow;
}

/* 获取当前SSDTShadow index、函数地址、当前所在模块 */
ULONG GetSSDTShadowFuncAddr( PSHADOW_INFO pShadowInfo, PULONG outputLen)
{
	PSYSTEM_SERVICE_TABLE_SHADOW KeServiceDescriptorTableShadow = NULL;
	ULONG		iCount = 0;
	ULONG		ulDrvNums=0;
	ULONG		iDrv=0;
	PDRVMOD_ITEM ptagDrvItem;
	PDRVMOD_ITEM pWin32kInfo;
	ANSI_STRING				AnsiString;
	UNICODE_STRING			unico;

	if ( !pShadowInfo || !outputLen )
		return 0;

	KeServiceDescriptorTableShadow = (PSYSTEM_SERVICE_TABLE_SHADOW)GetSSDTShadowTable();
	if ( !KeServiceDescriptorTableShadow )
		return 0;

	//输出缓冲区尺寸 < 数量 * 尺寸，则返回 数量 * 尺寸
	if ( *outputLen < KeServiceDescriptorTableShadow->ServiceLimit * sizeof(SHADOW_INFO) )
	{
		*outputLen = (ULONG)KeServiceDescriptorTableShadow->ServiceLimit * sizeof(SHADOW_INFO);
		return 0;
	}

	//枚举驱动信息
	ptagDrvItem=GetKrnlModuleList(&ulDrvNums);

	/* 填充SSDTShadow的 index、当前函数地址 */
	for (iCount=0; iCount < KeServiceDescriptorTableShadow->ServiceLimit; iCount++)
	{
		pShadowInfo[iCount].currentAddr = (ULONG)KeServiceDescriptorTableShadow->ServiceTable[iCount];

//		KdPrint(("ShadowSSDT index:%d \t funcAddr:0x%08x\n", iCount, (ULONG)KeServiceDescriptorTableShadow->ServiceTable[iCount]));

		/* 查找Shadow函数当前所在的内核模块 */
		for ( iDrv = 0; iDrv < ulDrvNums; iDrv++ )
		{
			/* 查找ssdt函数当前所在的内核模块 */
			if ( CompareModule( (PVOID)pShadowInfo[iCount].currentAddr, &ptagDrvItem[iDrv]) )
			{
				RtlInitUnicodeString( &unico, ptagDrvItem[iDrv].pwszDrvPath );
				RtlUnicodeStringToAnsiString( &AnsiString, &unico, TRUE );

				/* 填充当前Shadow函数所在模块路径 */
				strcpy( pShadowInfo[iCount].imagePath, AnsiString.Buffer );
				RtlFreeAnsiString( &AnsiString );

//				KdPrint(("ssdt函数地址:0x%08X \t 模块路径:%s\n", pShadowInfo[iCount].currentAddr, pShadowInfo[iCount].imagePath));
				break;
			}
		}
	}

	//改写输出缓冲区尺寸、返回个数
	*outputLen = iCount * sizeof(SHADOW_INFO);
	return iCount;
}

/* attach to GUI process. */
NTSTATUS AttachGuiProcess( PKAPC_STATE pApcStatus )
{
	NTSTATUS	status=STATUS_SUCCESS;
	ULONG		osMajor = 0;
	PEPROCESS	csrssEproc;


	//获取os主板本号
	status = PsGetVersion( &osMajor, NULL, NULL, NULL );
	if ( NT_SUCCESS(status) )
	{
		//vista以上不兼容
		if (osMajor != 6)
		{
			//获取Csrss.exe进程EPROCESS
			status=PsLookupProcessByProcessId( (PVOID)GetCsrssPid(), &csrssEproc);
			if ( !NT_SUCCESS(status) )
			{
				KdPrint(("attach to GUI process failed！\n"));
				return STATUS_UNSUCCESSFUL;
			}
		}
	}

	// StackAttach(guiEproc, ApcStatus)
	KeStackAttachProcess(csrssEproc, pApcStatus);
	return status;
}

///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：枚举Csrss.exe进程PID
//	输入参数：无
//	输出参数：返回Csrss.exe进程的PID
//
///////////////////////////////////////////////////////////////////////////////////
HANDLE	GetCsrssPid()
{
	NTSTATUS			ntStatus;
	HANDLE				hProc, hObject;
	HANDLE				CsrssPid = (HANDLE)0;
	OBJECT_ATTRIBUTES	objAttr;
	CLIENT_ID			cid;
	int					i;
	UNICODE_STRING		ApiPortName;
	POBJECT_NAME_INFORMATION ObjName;
	PSYSTEM_HANDLE_INFORMATION_EX Handles;



	RtlInitUnicodeString( &ApiPortName, L"\\Windows\\ApiPort" );

	//获取句柄信息
	Handles = GetInfoTable( SystemHandleInformation );
	if( Handles == NULL )
	{
		DbgPrint("[GetCsrssPid]->GetInfoTable() Error\n");
		return 0;
	}

	ObjName = ExAllocatePool( PagedPool, 0x2000 );

	for( i = 0; i != Handles->NumberOfHandles; i++ )
	{
		if ( Handles->Information[i].ObjectTypeNumber == 21 ) //Port object，Win2kSP1下找不到21端口
		{
			InitializeObjectAttributes( &objAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
			cid.UniqueProcess = (HANDLE)Handles->Information[i].ProcessId;
			cid.UniqueThread  = 0;

			//打开进程
			ntStatus = ZwOpenProcess( &hProc, PROCESS_DUP_HANDLE, &objAttr, &cid );
			if( NT_SUCCESS(ntStatus) )
			{
				//复制句柄
				ntStatus = ZwDuplicateObject( hProc, (HANDLE)Handles->Information[i].Handle,
												NtCurrentProcess(), &hObject, 0, 0, DUPLICATE_SAME_ACCESS );
				if( NT_SUCCESS(ntStatus) )
				{
					//查询对象
					ntStatus = ZwQueryObject( hObject, ObjectNameInformation, ObjName, 0x2000, NULL);
					if( NT_SUCCESS(ntStatus) )             
					{
						if (ObjName->Name.Buffer != NULL)
						{
							if ( wcsncmp( ApiPortName.Buffer, ObjName->Name.Buffer, 20 ) == 0 )
							{
								//获取Csrss.exe进程Pid
								CsrssPid = (HANDLE)Handles->Information[i].ProcessId;

								ZwClose( hProc );
								ZwClose( hObject );
								IxExFreePool( Handles );
								IxExFreePool( ObjName );
								return CsrssPid;
							}
						} 
					} else 
						DbgPrint("Error in Query Object\n");

					ZwClose(hObject);
				} else 
					DbgPrint("Error on duplicating object\n");

				ZwClose(hProc);
			} else 
				DbgPrint("Could not open process\n");
		}
	}

	IxExFreePool( Handles );
	IxExFreePool( ObjName );
	return 0;
}

BOOLEAN
RestoreShadowSSDTItem( IN	PSSDTINFO	pInfo )
{
	NTSTATUS status;
	ULONG	SysMajor=0;
	KIRQL	oldirql;
	PMDL	pmdl;
	PULONG	pWriteMem=NULL;
	KAPC_STATE pApcStatus;
	PSYSTEM_SERVICE_TABLE_SHADOW KeServiceDescriptorTableShadow = NULL;

	KeServiceDescriptorTableShadow = (PSYSTEM_SERVICE_TABLE_SHADOW)GetSSDTShadowTable();
	if( !KeServiceDescriptorTableShadow )
	{
		//KdPrint(("[RestoreShadowSSDTItem]	获取KeServiceDescriptorTableShadow失败!"));
		return	FALSE;
	}


	//attach to GUI process.
	status=AttachGuiProcess( &pApcStatus );
	if ( !NT_SUCCESS(status) )
		return FALSE;

	if(KeGetCurrentIrql()<DISPATCH_LEVEL)
	{
		KeRaiseIrql(DISPATCH_LEVEL, &oldirql); 
	}

	pWriteMem=(PULONG)CreateWriteMDL( (ULONG)KeServiceDescriptorTableShadow->ServiceTable,
					KeServiceDescriptorTableShadow->ServiceLimit*sizeof(ULONG), &pmdl );

	pWriteMem[pInfo->index]=pInfo->funAddr;

	FreeWriteMDL(pmdl,(PVOID)pWriteMem);

	if(KeGetCurrentIrql()==DISPATCH_LEVEL)
	{
		KeLowerIrql(oldirql);
	}

	//解挂靠
	KeUnstackDetachProcess(&pApcStatus);
	//KdPrint(("[RestoreShadowSSDTItem] 恢复SHADOW SSDT服务地址成功\n"));
	return TRUE;
}