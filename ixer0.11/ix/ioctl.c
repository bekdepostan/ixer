/*===================================================================
 * FileName:
 *		ioctl.c
 * 
 * Author: Chinghoi		Email:  Chinghoi@vip.qq.com
 *
 * Description:
 *
 * Date:
 *		build:
 *			Aug 11��2012
 *		revision:
 *			Jan 28��2013
 *
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "msghook.h"
#include "ShadowSSDT.h"
#include "ssdt.h"
#include "drvMod.h"
#include "Thread.h"
#include "dll.h"
#include "WinOSVersion.h"
#include "proc.h"
#include "file.h"
#include "reg.h"
#include "net.h"
#include "..\Common\Surface.h"
#include "..\Common\ioctl.h"


/*========================================================================
*
* ������:	IxioControl
*
* ����:		PVOID				[IN] InputBuffer
*			ULONG				[IN] InputBufferLength
*			PVOID				[IN] OutputBuffer
*			ULONG				[IN] OutputBufferLength
*			ULONG				[IN] IoControlCode
*			PIO_STATUS_BLOCK	[IN] IoStatus
*
* ��������:	������ǲ����
*
* ����ֵ:	BOOLEAN
*
=========================================================================*/
BOOLEAN
IxioControl(
			IN PVOID InputBuffer,
			IN ULONG InputBufferLength,
			IN PVOID OutputBuffer,
			IN ULONG OutputBufferLength,
			IN ULONG IoControlCode,
			IN PIO_STATUS_BLOCK pIoStatus)
{
	NTSTATUS		status = STATUS_SUCCESS;					//Ĭ�Ϸ���ֵ��״̬
	ULONG			irpInfo= 0;

	dprintf("[$XTR] IxioControl(InputBuf=0x%08x, inputBufLen=0x%x, OutputBuf=0x%08x, outputBufLen=0x%x, IoControlCode=0x%08x, pIoStatus=0x%08x)\n",
		InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength, IoControlCode, pIoStatus);

	switch (IoControlCode)
	{
		//ö�١������̲����ظ���
	case IOCTL_ENUM_PROCESS:
		{
			PULONG			OutBuf;
			ULONG			i=0;
			ULONG			h=0;
			ULONG			numOfProc=0;

			dprintf("[$XTR->IOCTL_ENUM_PROCESS]\n");

			OutBuf = (PULONG)OutputBuffer;
			//ö�ٽ���
			status = GetProcessList();

			if ( OutputBufferLength < sizeof(ULONG) )
			{
				dprintf("OutputBufferLength is Not Enough %d\n", sizeof(ULONG));
				status=STATUS_BUFFER_TOO_SMALL;
				goto End;
			}

			//����EPROCESS�����ȡ���̸���
			for (i=0; i<MAX_EPROCESS_NUM; i++)
			{
				if (Eproc[i] != 0)
					numOfProc++;	//����EPROCESS��������

				//���ؽ���
				if ( hideEproc[i] != 0 && i < 128 )
				{
					h++;
					numOfProc++;
				}
			}
			dprintf("[$ixer]->���̼���: %d, ���ؽ���: %d\n", numOfProc, h );

			//�ѽ��̸���������OutputBuffer����ring3
			RtlCopyMemory( OutBuf, &numOfProc, sizeof(ULONG) );

			irpInfo = sizeof(ULONG);
			status=STATUS_SUCCESS;
			break;
		}

			//��ȡ�����б�
	case IOCTL_GET_PROCESS_LIST:
		{
			PPROCESS_ITEM	pProcOutput;
			ULONG			i=0;
			ULONG			j=0;
			ULONG			k=0;

			dprintf("[$XTR->IOCTL_GET_PROCESS_LIST]\n");

			j = 0;
			pProcOutput = (PPROCESS_ITEM)OutputBuffer;

			if (KeGetCurrentIrql() != PASSIVE_LEVEL)
			{
				status=STATUS_UNSUCCESSFUL;
				break;
			}

			dprintf("PID: \t\t\t\t\t ImagePath:\n");
			for (i=0; i<MAX_EPROCESS_NUM; i++)
			{
				if (Eproc[i] != 0)
				{
					pProcOutput[j].hideFlag = FALSE;
					pProcOutput[j].pEproc = (ULONG)Eproc[i];						//EPROCESS
					pProcOutput[j].ulPid = *(PULONG)( (ULONG)Eproc[i] + g_EPROCESS_UniqueProcessId);		//����ID
					pProcOutput[j].ulPPid = *(PULONG)( (ULONG)Eproc[i] + g_EPROCESS_InheritedFromUniqueProcessId);		//������ID
//					RtlCopyString(procItem[j].lpszImageName, (PCSTR)( (ULONG)Eproc[i] + g_EPROCESS_ImageFileName )); //����ӳ����
					//������
					strcpy( pProcOutput[j].pszImageName, PsGetProcessImageFileName( (PEPROCESS)pProcOutput[j].pEproc ) );

					//����·��
					GetImageFullPathByEprocess( (PEPROCESS)pProcOutput[j].pEproc, pProcOutput[j].pszImagePath);

					//��ӡ������Ϣ
//					dprintf(" %d \t\t\t\t\t   %s\n", pProcOutput[j].ulPid , &pProcOutput[j].pszImagePath);
					dprintf("hideFlag: %d Pid: %d \t\t\t\t\t   %s\n", pProcOutput[j].hideFlag, pProcOutput[j].ulPid , &pProcOutput[j].pszImagePath);
					j++;
				}
			}

//			h = j-1;
			for (k=0; k<128; k++)
			{
				if ( hideEproc[k] != 0 && k < 128 )
				{
					// ������ؽ���
					pProcOutput[j].hideFlag = TRUE;
					pProcOutput[j].pEproc = (ULONG)hideEproc[k];			//EPROCESS
					pProcOutput[j].ulPid = *(PULONG)( (ULONG)hideEproc[k] + g_EPROCESS_UniqueProcessId);	//����ID
					pProcOutput[j].ulPPid = *(PULONG)( (ULONG)hideEproc[k] + g_EPROCESS_InheritedFromUniqueProcessId);	//������ID

					//������
					strcpy( pProcOutput[j].pszImageName, PsGetProcessImageFileName( (PEPROCESS)pProcOutput[j].pEproc ) );
					//����·��
					GetImageFullPathByEprocess( (PEPROCESS)pProcOutput[j].pEproc, pProcOutput[j].pszImagePath);

					//��ӡ������Ϣ
//					dprintf("[$ixer]->��⵽���ؽ���: %d \t\t\t %s\n", pProcOutput[j].ulPid , &pProcOutput[j].pszImagePath);
					dprintf("hideFlag: %d Pid: %d \t\t\t\t\t   %s\n", pProcOutput[j].hideFlag, pProcOutput[j].ulPid , &pProcOutput[j].pszImagePath);
					j++;
				}
			}

			irpInfo = j * sizeof(PROCESS_ITEM);
			status=STATUS_SUCCESS;
			break;
		}

			//��������
	case IOCTL_PROCESS_KILLER:
		{
			HANDLE			dwPid;
			PEPROCESS		pEprocess;

			dprintf("[$XTR->IOCTL_PROCESS_KILLER]\n");

			if ( InputBufferLength < sizeof(ULONG) )
			{
				dprintf("$XTR:\tInputBufferLength:0x%08x", InputBufferLength);
				status = STATUS_INVALID_BUFFER_SIZE;
				goto End;
			}

			if (InputBuffer == NULL)
			{
				dprintf("$XTR:\tInputBuffer == NULL\n");
				status = STATUS_INVALID_PARAMETER;
				goto End;
			}

			__try
			{
				memcpy( &dwPid, InputBuffer, sizeof(dwPid) );
				//ͨ��PsLookupProcessByProcessId�õ�EPROCESS
				status = PsLookupProcessByProcessId( dwPid, &pEprocess );
				if ( !NT_SUCCESS(status) )
				{
					dprintf("PsLookupProcessByProcessId Failed!\n");
					goto End;
				}

				status = KillProcByPspTerminateThreadByPointer( pEprocess );
				if (NT_SUCCESS(status))
				{
					dprintf("Pid:%d terminated succeed!\n", dwPid);
					break;
				}
			}
			__except( EXCEPTION_EXECUTE_HANDLER)
			{
				status = GetExceptionCode();
				dprintf("Terminate process Pid:%d Failed! ErrorCode:%d\n", status);
				break;
			}

			break;
		}

			//ö�١����Dll
	case IOCTL_GET_Dll_LIST:
		{
			PDLL_ITEM		pDllItem=NULL;
			PDLL_ITEM		pDllOutput=NULL;

			HANDLE			dwPid;
			ULONG			i=0;
			ULONG			numOfDll=0; //��ʼ��Dll����Ϊ0

			dprintf("[$XTR->IOCTL_GET_Dll_LIST]\n");

			if ( InputBufferLength < sizeof(ULONG) )
			{
				dprintf("InputBufferLength is Not Enough %d\n", sizeof(ULONG));
				status=STATUS_BUFFER_TOO_SMALL;
				goto End;
			}
			//�������Pid
			memcpy( &dwPid, InputBuffer, sizeof(dwPid) );

			//pDllOutputָ�������Ring3�Ļ�����
			pDllOutput = (PDLL_ITEM)OutputBuffer;
			//�������������
			RtlZeroMemory(pDllOutput, OutputBufferLength);

			//ö��Dll����
			pDllItem = GetDllList( (ULONG)dwPid, &numOfDll);
			if (pDllItem != NULL)
			{
				for (i=0; i<numOfDll; i++)
				{
					if (pDllItem[i].ulBaseAddr != 0)
					{
						RtlCopyMemory(&pDllOutput[i], &pDllItem[i], sizeof(DLL_ITEM));
					}
				}

				irpInfo = numOfDll*sizeof(DLL_ITEM);
				status=STATUS_SUCCESS;

				IxExFreePool(pDllItem);
				break;
			}
			else
				status=STATUS_UNSUCCESSFUL;

			break;
		}

			//ģ���ս���
	case IOCTL_DLL_TERMINATOR:
		{
			NTSTATUS	ntStatus;
			PEPROCESS	pEprocess;
			HANDLE		hProcess;

			//����ģ����������PID
			ULONG procId = *(ULONG *)InputBuffer;
			//ģ���ַ
			PVOID baseAddr = *(PVOID *)( (ULONG)InputBuffer + 4);

			dprintf("[$XTR->IOCTL_DLLMOD_TERMINATOR]\n");

			//�ɽ���pid�õ����̵� PEPROCESS
			ntStatus = PsLookupProcessByProcessId( (HANDLE)procId, &pEprocess );
			if (ntStatus != STATUS_SUCCESS)
			{
				DbgPrint("KillModuleByZwUnmapViewOfSection: PsLookupProcessByProcessId Error! ProcessId=%8X \n", procId);
				break;
			}

			//�ɽ��̶���õ����̾��
			ntStatus = ObOpenObjectByPointer(pEprocess, 0, NULL, 0, NULL, KernelMode, &hProcess);
			if (ntStatus != STATUS_SUCCESS) 
			{
				DbgPrint("ObOpenObjectByPointer Error! ProcessId=%8X \n", procId); 
				break;
			} 

//			KeAttachProcess( pEprocess );
			//���ȳ�����ZwUnmapViewOfSection��ж�����ģ��
			ntStatus = ZwUnmapViewOfSection(hProcess, baseAddr);
			if (ntStatus != STATUS_SUCCESS)
			{
				DbgPrint("ZwUnmapViewOfSection Error! ProcessId=%8X BaseAddress=%8X ntStatus=%8X\n", procId, baseAddr, ntStatus);
			}
			else  
			{
				DbgPrint("^_^ ZwUnmapViewOfSection ok! ProcessId=%8X BaseAddress=%8X\n", procId, baseAddr);
			}  

//			KeDetachProcess();
			ZwClose( hProcess );
			IxObDereferenceObject(pEprocess);
			break;
		}

			//�����߳��б�
	case IOCTL_GET_THREAD_LIST:
		{
			PEPROCESS	pInputEproc;
			PTHREAD_ITEM pThreadOutput;
			ULONG		retBytes;

			dprintf("[$XTR->IOCTL_GET_THREAD_LIST]\n");
			//R3�����߳����ڽ���eprocess
			pInputEproc = (PEPROCESS)*( (PULONG)InputBuffer );

			pThreadOutput = (PTHREAD_ITEM)OutputBuffer;

			status=GetThreadList(pInputEproc, pThreadOutput, OutputBufferLength, &retBytes);

			irpInfo=retBytes;
			break;
		}

			//�߳��ս���
	case IOCTL_THREAD_TERMINATOR:
		dprintf("[$XTR->IOCTL_THREAD_TERMINATOR]\n");
		KillThreadByThreadCID( *(ULONG *)InputBuffer );
		break;

			//ö�١��������
	case IOCTL_ENUM_DRIVER:
		{
			PDRVMOD_ITEM pDrvItem;
			PDRVMOD_ITEM pDrvOutput;
			ULONG	numOfDrv=0; //drv����
			ULONG	sumOfDrv=0;	//��������
			ULONG	i=0;

			dprintf("[$XTR->IOCTL_ENUM_DRIVER]\n");

			pDrvOutput=(PDRVMOD_ITEM)OutputBuffer;
			pDrvItem=GetKrnlModuleList(&numOfDrv);

			for (i=0; i<numOfDrv; i++)
			{
				//��ַ����0������ntkrnl**.exe�Ļ�ַ
				if (pDrvItem[i].ulBaseAddr > 0   &&  pDrvItem[i].ulBaseAddr >= pDrvItem[0].ulBaseAddr)	//(ntkrnl**.exe�����ڵ�1��)
				{
//					DbgPrint("��ַ:0x%08X \t ��������:0x%08X \t ����·��:%ws\n", pDrvItem[i].ulBaseAddr, pDrvItem[i].pDrvObject, pDrvItem[i].pwszDrvPath);
					//�ѹ�ϣ�����������Ϣ���������������
					RtlCopyMemory(&pDrvOutput[sumOfDrv], &pDrvItem[i], sizeof(DRVMOD_ITEM));
					//��������
					sumOfDrv++;
				}
			}

			irpInfo=sumOfDrv*sizeof(DRVMOD_ITEM);
			status=STATUS_SUCCESS;
			break;
		}

			//��ȡSSDT����
	case IOCTL_GET_SSDT_INFO:
		{
			//��ȡssdt����
			ULONG	numOfSrv = GetSSDTNumber();
			ULONG	dataLen = numOfSrv*sizeof(SSDT_INFO);

			dprintf("[$XTR->IOCTL_GET_SSDT_INFO]\n");

			//ssdt����
			if ( OutputBufferLength < dataLen )
			{
				if ( OutputBufferLength == sizeof(ULONG) )
				{
					dprintf("[$XTR->IOCTL_GET_SSDT_INFO]->GetSSDTNumber() ssdt����:%d\n", numOfSrv);
					//ssdt����
					*( (PULONG)OutputBuffer ) = numOfSrv;

					irpInfo=sizeof(ULONG);
					status=STATUS_SUCCESS;
					break;
				}

				status=STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//������������
			RtlZeroMemory(OutputBuffer, (KeServiceDescriptorTable->ntoskrnl.ServiceLimit) * sizeof(SSDT_INFO));
			//��������
			GetSSDTinfo( OutputBuffer );

			irpInfo = (KeServiceDescriptorTable->ntoskrnl.ServiceLimit) * sizeof(SSDT_INFO);
			status=STATUS_SUCCESS;
			break;
		}

			//�ָ�ssdt
	case IOCTL_RESTORE_SSDT:
		{
			dprintf("[$ixer->IOCTL_RESTORE_SSDT]\n");

			if ( !InputBuffer || InputBufferLength<sizeof(SSDTINFO) )
			{
				dprintf("���뻺�����򳤶���Ч!\n");

				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if ( !RestoreSSDTItem( (PSSDTINFO)InputBuffer ) )
			{
				dprintf("�ָ�ssdt��ʧ��!\n");
				status=STATUS_UNSUCCESSFUL;
			}

			break;
		}

			//��ȡShadowSSDT����
	case IOCTL_GET_SHADOWSSDT_INFO:
		{
			//��ȡShadowSsdt����
			ULONG	numOfShadowSrv = GetSSDTShadowNumber();
			ULONG	dataLen = numOfShadowSrv * sizeof(SHADOW_INFO);
			KAPC_STATE pApcStatus;

			dprintf("[$XTR->IOCTL_GET_SHADOWSSDT_INFO]\n");
			//����ShadowSsdt����
			if ( OutputBufferLength < dataLen )
			{
				if ( OutputBufferLength == sizeof(ULONG) )
				{
					dprintf("[$XTR->IOCTL_GET_SHADOWSSDT_INFO]->GetSSDTShadowNumber() ShadowSsdt����:%d\n", numOfShadowSrv);
					*( (PULONG)OutputBuffer ) = numOfShadowSrv;
					irpInfo = sizeof(ULONG);
					break;
				}

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			/*����ShadowSsdt����*/
			//attach to GUI process.
			status=AttachGuiProcess( &pApcStatus );
			if ( !NT_SUCCESS(status) )
				break;

			numOfShadowSrv=GetSSDTShadowFuncAddr( (PSHADOW_INFO)OutputBuffer, &dataLen);
			KdPrint(("����������=%ld\n",numOfShadowSrv));
			irpInfo = dataLen;
			status = STATUS_SUCCESS;

			//��ҿ�
			KeUnstackDetachProcess(&pApcStatus);
			break;
		}

			//�ָ�Shadow
	case IOCTL_RESTORE_SHADOW:
		{
			dprintf("[$ixer->IOCTL_RESTORE_SHADOW]\n");
			if ( !InputBuffer || InputBufferLength<sizeof(SSDTINFO) )
			{
				dprintf("���뻺�����򳤶���Ч!\n");
				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if ( !RestoreShadowSSDTItem( (PSSDTINFO)InputBuffer) )
			{
				dprintf("�ָ�Shadow SSDT����ʧ��!\n");
				status=STATUS_UNSUCCESSFUL;
			}

			break;
		}

			//��ȡMsgHook��Ϣ
	case IOCTL_GET_MSGHOOK_INFO:
		{
			dprintf("[$ixer->IOCTL_GET_MSGHOOK_INFO]\n");
			RtlZeroMemory( OutputBuffer, OutputBufferLength);

			status = GetAllMsgHookInfo( OutputBuffer );
			irpInfo= OutputBufferLength;

			break;
		}

			//��ȡ�ļ�����Ŀ
	case IOCTL_GET_DIR_NUM:
		{
			PCHAR	ioDirName=NULL;
			PVOID	pDirInfo=NULL;
			ULONG	dwRetSize=0;
			ULONG	Num=0;

			dprintf("[$ixer->IOCTL_GET_DIR_NUM]\n");

			if ( !InputBuffer || InputBufferLength<3 || ( OutputBufferLength<sizeof(ULONG) ) )
			{
				KdPrint(("����Ĳ�������\n"));
				status=STATUS_INVALID_PARAMETER;
				break;
			}

			ioDirName=(PCHAR)InputBuffer; 
			//��ѯĿ¼�ļ���Ϣ
			pDirInfo = QueryDirectoryFileInfo( ioDirName, &dwRetSize );
			//��С/Ŀ¼�ṹ=����
			Num = dwRetSize/sizeof(DIRECTORY_INFO);

			KdPrint(("Ŀ¼����:%d\n", Num));
			//���ļ��������������������
			RtlCopyMemory( OutputBuffer, &Num, sizeof(ULONG) );

			IxExFreePool( pDirInfo );
			irpInfo = sizeof(ULONG);
			break;
		}

		//��ȡ�ļ��С�Ŀ¼�ļ���Ϣ
	case IOCTL_GET_DIR_INFO:
		{
			ULONG	Num=0;
			PDIRECTORY_INFO pDirFile=NULL;
			PCHAR	ioDirName=NULL;
			PVOID	pDirInfo=NULL;
			ULONG	dwRetSize=0;
			ULONG	i=0;

			dprintf("[$ixer->IOCTL_GET_DIR_INFO]\n");

			if ( !InputBuffer || InputBufferLength<3 || ( OutputBufferLength<sizeof(DIRECTORY_INFO) ) )
			{
				KdPrint(("����Ĳ�������\n"));
				status=STATUS_INVALID_PARAMETER;
				break;
			}

			ioDirName=(PCHAR)InputBuffer;
			//��ѯĿ¼�ļ�����
			pDirInfo = QueryDirectoryFileInfo( ioDirName, &dwRetSize );

			//���ļ�����Ϣ���������������
			RtlCopyMemory( OutputBuffer, pDirInfo, dwRetSize );

			KdPrint(("���ش�С��%d\n", dwRetSize));
			pDirFile=(PDIRECTORY_INFO)pDirInfo;
			//��С/Ŀ¼�ṹ=����
			Num = dwRetSize/sizeof(DIRECTORY_INFO);
			for (i=0; i<Num; i++)
			{
				KdPrint(("%s\n", pDirFile->FileName));
				pDirFile++;
			}

			IxExFreePool( pDirInfo );
			irpInfo = dwRetSize;
			break;
		}

		//ɾ���ļ�
	case IOCTL_DELETE_FILE:
		{
			dprintf("[$ixer->IOCTL_DELETE_FILE]\n");
			if ( InputBufferLength < (MAX_PATH + MAXNAMELEN) * sizeof(WCHAR) )
			{
				dprintf( "$ixer:\tInputBufferLength:0x%08x", InputBufferLength );
				status = STATUS_INVALID_BUFFER_SIZE;
				break;
			}

			if ( InputBuffer == NULL )
			{
				dprintf( "$ixer:InputBuffer == NULL" );
				status = STATUS_INVALID_PARAMETER;

				break;
			}

			status = IxDeleteFile( InputBuffer );
			break;
		}

		//��ȡ���ߴ�
	case IOCTL_GET_REGKEY_SIZE:
		{
			dprintf("[$ixer->IOCTL_GET_REGKEY_SIZE]\n");

			if ( OutputBufferLength < 3 * sizeof(ULONG) )
			{
				dprintf("$ixer:OutputBufferLength:0x%08x\n", OutputBufferLength);

				status = STATUS_INVALID_BUFFER_SIZE;
				break;
			}

			if ( InputBufferLength < MAX_PATH * sizeof(WCHAR) )
			{
				dprintf("$ixer:InputBufferLength:0x%08x\n", InputBufferLength);

				status = STATUS_INVALID_BUFFER_SIZE;
				break;
			}

			if ( OutputBuffer == NULL )
			{
				dprintf("$ixer:OutputBuffer == NULL\n");

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			if ( InputBuffer == NULL )
			{
				dprintf("$ixer:InputBuffer == NULL\n");

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			status = QueryKeyItemInfo( InputBuffer, OutputBuffer, 3 * sizeof(ULONG) );
			irpInfo = 3 * sizeof(ULONG);

			break;
		}

		//��ȡ��
	case IOCTL_GET_REGKEY:
		{
			dprintf("[$ixer->IOCTL_GET_REGKEY]\n");

			if ( InputBufferLength < (MAX_PATH + 1) * sizeof(WCHAR) )
			{
				dprintf("$ixer:InputBufferLength:0x%08x\n", InputBufferLength);

				status = STATUS_INVALID_BUFFER_SIZE;
				break;
			}

			if ( OutputBuffer == NULL )
			{
				dprintf("$ixer:OutputBuffer == NULL\n");

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			if ( InputBuffer == NULL )
			{
				dprintf("$ixer:InputBuffer == NULL\n");

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			status = EnumerateKey( &((LPWSTR)InputBuffer)[1], OutputBuffer, *(PUSHORT)InputBuffer );
			irpInfo = OutputBufferLength;

			break;
		}

		//��ȡ��ֵ
	case IOCTL_GET_REGVALUE:
		{
			dprintf("[$ixer->IOCTL_GET_REGVALUE]\n");

			if ( InputBufferLength < (MAX_PATH + 1) * sizeof(WCHAR) )
			{
				dprintf("$ixer:InputBufferLength:0x%08x\n", InputBufferLength);

				status = STATUS_INVALID_BUFFER_SIZE;
				break;
			}

			if ( OutputBuffer == NULL )
			{
				dprintf("[$ixer:OutputBuffer == NULL\n");

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			if ( InputBuffer == NULL )
			{
				dprintf("[$ixer:InputBuffer == NULL\n");

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			status = EnumerateValueKey( &((LPWSTR)InputBuffer)[1], OutputBuffer, *(PUSHORT)InputBuffer );
			irpInfo = OutputBufferLength;
			break;
		}

		/* ɾ��ע����ֵ */
	case IOCTL_DELETE_REGVALUE:
		{
			dprintf("[$ixer->IOCTL_DELETE_REGVALUE]\n");

			if ( InputBufferLength < sizeof(VALUE_KEY) )
			{
				dprintf("$ixer:InputBufferLength:0x%08x\n", InputBufferLength);

				status = STATUS_INVALID_BUFFER_SIZE;
				break;
			}

			if ( InputBuffer == NULL )
			{
				dprintf("[$ixer:InputBuffer == NULL\n");

				status = STATUS_INVALID_PARAMETER;
				break;
			}

			status = DeleteValueKey( ((PVALUE_KEY)InputBuffer)->Key, ((PVALUE_KEY)InputBuffer)->Value );
			break;
		}
			//ö��TCP
	case IOCTL_ENUM_TCP:
		{
			PVOID	pOut=NULL;
			ULONG	OutLen=0; 

			dprintf("[$ixer->IOCTL_ENUM_TCP]\n");

			if( OutputBufferLength < sizeof(CONNINFO102) )
			{
				KdPrint(("���������������Ч\n"));

				status=STATUS_BUFFER_OVERFLOW;
				break;
			}

			pOut = EnumPortInformation( &OutLen, TCPPORT );
			if(!pOut)
			{
				KdPrint(("��ȡTCP�˿���Ϣʧ��!\n"));

				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if( OutputBufferLength<OutLen )
			{
				KdPrint(("���������̫С,ӦΪ%ld\n", OutLen));

				IxExFreePool(pOut);
				status=STATUS_BUFFER_OVERFLOW;
				break;
			} 

			RtlCopyMemory(OutputBuffer, pOut, OutLen);

			IxExFreePool(pOut);
			irpInfo = OutLen;
			break;
		}

			//ö��UDP
	case IOCTL_ENUM_UDP:
		{
			PVOID	pOut=NULL;
			ULONG	OutLen=0;  

			dprintf("[$ixer->IOCTL_ENUM_UDP]\n");

			if( OutputBufferLength < sizeof(UDPCONNINFO) )
			{
				KdPrint(("���������������Ч\n"));

				status=STATUS_BUFFER_OVERFLOW;
				break;
			}

			pOut = EnumPortInformation( &OutLen, UDPPORT );
			if(!pOut)
			{
				KdPrint(("��ȡUDP�˿���Ϣʧ��!\n"));

				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if( OutputBufferLength<OutLen )
			{
				KdPrint(("���������̫С,ӦΪ%ld\n",OutLen));

				IxExFreePool(pOut);
				status=STATUS_BUFFER_OVERFLOW;
				break;
			}

			RtlCopyMemory( OutputBuffer, pOut, OutLen );

			IxExFreePool(pOut);
			irpInfo = OutLen;
			break;
		}

	/* ������� */
	default:
			dprintf("[$XTR] IRP_MJ_DEVICE_CONTROL: IoControlCode=0x%x (%04x,%04x)\n",
				IoControlCode, DEVICE_TYPE_FROM_CTL_CODE(IoControlCode),
				IoGetFunctionCodeFromCtlCode(IoControlCode));

		status = STATUS_INVALID_PARAMETER;
		break;
	}

End:
/*	dprintf("[$XTR]->IxioControl() end.\n");*/
	pIoStatus->Information = irpInfo;
	pIoStatus->Status = status;
	return TRUE;
}