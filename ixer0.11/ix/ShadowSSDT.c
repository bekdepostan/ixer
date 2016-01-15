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
* ������:	GetSSDTShadowNumber
*
* ����:		
*
* ��������:	��ȡ�������
*
* ����ֵ:	ULONG
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

/* ��ȡShadowSSDT���ַƫ�� */
ULONG GetSSDTShadowTableAddr()
{
	ULONG		ktrdCid;
	NTSTATUS	status;
	ULONG		ethread;
	ULONG		shadowTableAddr=0;
	ULONG		i=0;

	/* ����ö��ϵͳ�̻߳�ȡ KTHREAD */
	for( ktrdCid = 8; ktrdCid < 32768; ktrdCid += 4)
	{
		status = PsLookupThreadByThreadId( (HANDLE)ktrdCid, &(PETHREAD)ethread );
		if( NT_SUCCESS(status) )
		{	// 
			if( *(PULONG)( ethread + 0x0e0 )  !=  (ULONG)KeServiceDescriptorTable )
			{
				/* ��ȡ KTHREAD.ServiceTable ָ��ֵ */
				shadowTableAddr =  *(PULONG)( ethread + 0x0e0 );
				break;
			}
		}
	}

	return shadowTableAddr;
}

//����ShadowSSDT��ĵ�ַ�ṹ
PSYSTEM_SERVICE_TABLE_SHADOW GetSSDTShadowTable()
{
	PSYSTEM_SERVICE_TABLE_SHADOW KeServiceDescriptorTableShadow = NULL;

	//SSDTShadowTable��ַƫ�� ǿ��ת���� KeServiceDescriptorTableShadow
	KeServiceDescriptorTableShadow = (PSYSTEM_SERVICE_TABLE_SHADOW)GetSSDTShadowTableAddr();
	if ( !KeServiceDescriptorTableShadow )
		return NULL;

	KeServiceDescriptorTableShadow = &KeServiceDescriptorTableShadow[1];
	return KeServiceDescriptorTableShadow;
}

/* ��ȡ��ǰSSDTShadow index��������ַ����ǰ����ģ�� */
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

	//����������ߴ� < ���� * �ߴ磬�򷵻� ���� * �ߴ�
	if ( *outputLen < KeServiceDescriptorTableShadow->ServiceLimit * sizeof(SHADOW_INFO) )
	{
		*outputLen = (ULONG)KeServiceDescriptorTableShadow->ServiceLimit * sizeof(SHADOW_INFO);
		return 0;
	}

	//ö��������Ϣ
	ptagDrvItem=GetKrnlModuleList(&ulDrvNums);

	/* ���SSDTShadow�� index����ǰ������ַ */
	for (iCount=0; iCount < KeServiceDescriptorTableShadow->ServiceLimit; iCount++)
	{
		pShadowInfo[iCount].currentAddr = (ULONG)KeServiceDescriptorTableShadow->ServiceTable[iCount];

//		KdPrint(("ShadowSSDT index:%d \t funcAddr:0x%08x\n", iCount, (ULONG)KeServiceDescriptorTableShadow->ServiceTable[iCount]));

		/* ����Shadow������ǰ���ڵ��ں�ģ�� */
		for ( iDrv = 0; iDrv < ulDrvNums; iDrv++ )
		{
			/* ����ssdt������ǰ���ڵ��ں�ģ�� */
			if ( CompareModule( (PVOID)pShadowInfo[iCount].currentAddr, &ptagDrvItem[iDrv]) )
			{
				RtlInitUnicodeString( &unico, ptagDrvItem[iDrv].pwszDrvPath );
				RtlUnicodeStringToAnsiString( &AnsiString, &unico, TRUE );

				/* ��䵱ǰShadow��������ģ��·�� */
				strcpy( pShadowInfo[iCount].imagePath, AnsiString.Buffer );
				RtlFreeAnsiString( &AnsiString );

//				KdPrint(("ssdt������ַ:0x%08X \t ģ��·��:%s\n", pShadowInfo[iCount].currentAddr, pShadowInfo[iCount].imagePath));
				break;
			}
		}
	}

	//��д����������ߴ硢���ظ���
	*outputLen = iCount * sizeof(SHADOW_INFO);
	return iCount;
}

/* attach to GUI process. */
NTSTATUS AttachGuiProcess( PKAPC_STATE pApcStatus )
{
	NTSTATUS	status=STATUS_SUCCESS;
	ULONG		osMajor = 0;
	PEPROCESS	csrssEproc;


	//��ȡos���屾��
	status = PsGetVersion( &osMajor, NULL, NULL, NULL );
	if ( NT_SUCCESS(status) )
	{
		//vista���ϲ�����
		if (osMajor != 6)
		{
			//��ȡCsrss.exe����EPROCESS
			status=PsLookupProcessByProcessId( (PVOID)GetCsrssPid(), &csrssEproc);
			if ( !NT_SUCCESS(status) )
			{
				KdPrint(("attach to GUI process failed��\n"));
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
//	����ʵ�֣�ö��Csrss.exe����PID
//	�����������
//	�������������Csrss.exe���̵�PID
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

	//��ȡ�����Ϣ
	Handles = GetInfoTable( SystemHandleInformation );
	if( Handles == NULL )
	{
		DbgPrint("[GetCsrssPid]->GetInfoTable() Error\n");
		return 0;
	}

	ObjName = ExAllocatePool( PagedPool, 0x2000 );

	for( i = 0; i != Handles->NumberOfHandles; i++ )
	{
		if ( Handles->Information[i].ObjectTypeNumber == 21 ) //Port object��Win2kSP1���Ҳ���21�˿�
		{
			InitializeObjectAttributes( &objAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL );
			cid.UniqueProcess = (HANDLE)Handles->Information[i].ProcessId;
			cid.UniqueThread  = 0;

			//�򿪽���
			ntStatus = ZwOpenProcess( &hProc, PROCESS_DUP_HANDLE, &objAttr, &cid );
			if( NT_SUCCESS(ntStatus) )
			{
				//���ƾ��
				ntStatus = ZwDuplicateObject( hProc, (HANDLE)Handles->Information[i].Handle,
												NtCurrentProcess(), &hObject, 0, 0, DUPLICATE_SAME_ACCESS );
				if( NT_SUCCESS(ntStatus) )
				{
					//��ѯ����
					ntStatus = ZwQueryObject( hObject, ObjectNameInformation, ObjName, 0x2000, NULL);
					if( NT_SUCCESS(ntStatus) )             
					{
						if (ObjName->Name.Buffer != NULL)
						{
							if ( wcsncmp( ApiPortName.Buffer, ObjName->Name.Buffer, 20 ) == 0 )
							{
								//��ȡCsrss.exe����Pid
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
		//KdPrint(("[RestoreShadowSSDTItem]	��ȡKeServiceDescriptorTableShadowʧ��!"));
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

	//��ҿ�
	KeUnstackDetachProcess(&pApcStatus);
	//KdPrint(("[RestoreShadowSSDTItem] �ָ�SHADOW SSDT�����ַ�ɹ�\n"));
	return TRUE;
}