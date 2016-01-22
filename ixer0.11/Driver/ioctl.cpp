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
 *			Aug 11，2012
 *		revision:
 *			Jan 28，2013
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
* 函数名:	IxioControl
*
* 参数:		PVOID				[IN] InputBuffer
*			ULONG				[IN] InputBufferLength
*			PVOID				[IN] OutputBuffer
*			ULONG				[IN] OutputBufferLength
*			ULONG				[IN] IoControlCode
*			PIO_STATUS_BLOCK	[IN] IoStatus
*
* 功能描述:	驱动派遣例程
*
* 返回值:	BOOLEAN
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
	NTSTATUS		status = STATUS_SUCCESS;					//默认返回值、状态
	ULONG			irpInfo= 0;

	dprintf("[$XTR] IxioControl(InputBuf=0x%08x, inputBufLen=0x%x, OutputBuf=0x%08x, outputBufLen=0x%x, IoControlCode=0x%08x, pIoStatus=0x%08x)\n",
		InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength, IoControlCode, pIoStatus);

	switch (IoControlCode)
	{
		//枚举、检测进程并返回个数
	case IOCTL_ENUM_PROCESS:
		{
			PULONG			OutBuf;
			ULONG			i=0;
			ULONG			h=0;
			ULONG			numOfProc=0;

			dprintf("[$XTR->IOCTL_ENUM_PROCESS]\n");

			OutBuf = (PULONG)OutputBuffer;
			//枚举进程
			status = GetProcessList();

			if ( OutputBufferLength < sizeof(ULONG) )
			{
				dprintf("OutputBufferLength is Not Enough %d\n", sizeof(ULONG));
				status=STATUS_BUFFER_TOO_SMALL;
				goto End;
			}

			//遍历EPROCESS数组获取进程个数
			for (i=0; i<MAX_EPROCESS_NUM; i++)
			{
				if (Eproc[i] != 0)
					numOfProc++;	//进程EPROCESS个数递增

				//隐藏进程
				if ( hideEproc[i] != 0 && i < 128 )
				{
					h++;
					numOfProc++;
				}
			}
			dprintf("[$ixer]->进程计数: %d, 隐藏进程: %d\n", numOfProc, h );

			//把进程个数拷贝到OutputBuffer传给ring3
			RtlCopyMemory( OutBuf, &numOfProc, sizeof(ULONG) );

			irpInfo = sizeof(ULONG);
			status=STATUS_SUCCESS;
			break;
		}

			//获取进程列表
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
					pProcOutput[j].ulPid = *(PULONG)( (ULONG)Eproc[i] + g_EPROCESS_UniqueProcessId);		//进程ID
					pProcOutput[j].ulPPid = *(PULONG)( (ULONG)Eproc[i] + g_EPROCESS_InheritedFromUniqueProcessId);		//父进程ID
//					RtlCopyString(procItem[j].lpszImageName, (PCSTR)( (ULONG)Eproc[i] + g_EPROCESS_ImageFileName )); //拷贝映像名
					//镜像名
					strcpy( pProcOutput[j].pszImageName, PsGetProcessImageFileName( (PEPROCESS)pProcOutput[j].pEproc ) );

					//镜像路径
					GetImageFullPathByEprocess( (PEPROCESS)pProcOutput[j].pEproc, pProcOutput[j].pszImagePath);

					//打印调试信息
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
					// 标记隐藏进程
					pProcOutput[j].hideFlag = TRUE;
					pProcOutput[j].pEproc = (ULONG)hideEproc[k];			//EPROCESS
					pProcOutput[j].ulPid = *(PULONG)( (ULONG)hideEproc[k] + g_EPROCESS_UniqueProcessId);	//进程ID
					pProcOutput[j].ulPPid = *(PULONG)( (ULONG)hideEproc[k] + g_EPROCESS_InheritedFromUniqueProcessId);	//父进程ID

					//镜像名
					strcpy( pProcOutput[j].pszImageName, PsGetProcessImageFileName( (PEPROCESS)pProcOutput[j].pEproc ) );
					//镜像路径
					GetImageFullPathByEprocess( (PEPROCESS)pProcOutput[j].pEproc, pProcOutput[j].pszImagePath);

					//打印调试信息
//					dprintf("[$ixer]->检测到隐藏进程: %d \t\t\t %s\n", pProcOutput[j].ulPid , &pProcOutput[j].pszImagePath);
					dprintf("hideFlag: %d Pid: %d \t\t\t\t\t   %s\n", pProcOutput[j].hideFlag, pProcOutput[j].ulPid , &pProcOutput[j].pszImagePath);
					j++;
				}
			}

			irpInfo = j * sizeof(PROCESS_ITEM);
			status=STATUS_SUCCESS;
			break;
		}

			//结束进程
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
				//通过PsLookupProcessByProcessId得到EPROCESS
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

			//枚举、检测Dll
	case IOCTL_GET_Dll_LIST:
		{
			PDLL_ITEM		pDllItem=NULL;
			PDLL_ITEM		pDllOutput=NULL;

			HANDLE			dwPid;
			ULONG			i=0;
			ULONG			numOfDll=0; //初始化Dll个数为0

			dprintf("[$XTR->IOCTL_GET_Dll_LIST]\n");

			if ( InputBufferLength < sizeof(ULONG) )
			{
				dprintf("InputBufferLength is Not Enough %d\n", sizeof(ULONG));
				status=STATUS_BUFFER_TOO_SMALL;
				goto End;
			}
			//传入进程Pid
			memcpy( &dwPid, InputBuffer, sizeof(dwPid) );

			//pDllOutput指向输出到Ring3的缓冲区
			pDllOutput = (PDLL_ITEM)OutputBuffer;
			//清零输出缓冲区
			RtlZeroMemory(pDllOutput, OutputBufferLength);

			//枚举Dll数据
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

			//模块终结者
	case IOCTL_DLL_TERMINATOR:
		{
			NTSTATUS	ntStatus;
			PEPROCESS	pEprocess;
			HANDLE		hProcess;

			//传入模块所属进程PID
			ULONG procId = *(ULONG *)InputBuffer;
			//模块基址
			PVOID baseAddr = *(PVOID *)( (ULONG)InputBuffer + 4);

			dprintf("[$XTR->IOCTL_DLLMOD_TERMINATOR]\n");

			//由进程pid得到进程的 PEPROCESS
			ntStatus = PsLookupProcessByProcessId( (HANDLE)procId, &pEprocess );
			if (ntStatus != STATUS_SUCCESS)
			{
				DbgPrint("KillModuleByZwUnmapViewOfSection: PsLookupProcessByProcessId Error! ProcessId=%8X \n", procId);
				break;
			}

			//由进程对象得到进程句柄
			ntStatus = ObOpenObjectByPointer(pEprocess, 0, NULL, 0, NULL, KernelMode, &hProcess);
			if (ntStatus != STATUS_SUCCESS) 
			{
				DbgPrint("ObOpenObjectByPointer Error! ProcessId=%8X \n", procId); 
				break;
			} 

//			KeAttachProcess( pEprocess );
			//首先尝试用ZwUnmapViewOfSection来卸载这个模块
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

			//遍历线程列表
	case IOCTL_GET_THREAD_LIST:
		{
			PEPROCESS	pInputEproc;
			PTHREAD_ITEM pThreadOutput;
			ULONG		retBytes;

			dprintf("[$XTR->IOCTL_GET_THREAD_LIST]\n");
			//R3传入线程所在进程eprocess
			pInputEproc = (PEPROCESS)*( (PULONG)InputBuffer );

			pThreadOutput = (PTHREAD_ITEM)OutputBuffer;

			status=GetThreadList(pInputEproc, pThreadOutput, OutputBufferLength, &retBytes);

			irpInfo=retBytes;
			break;
		}

			//线程终结者
	case IOCTL_THREAD_TERMINATOR:
		dprintf("[$XTR->IOCTL_THREAD_TERMINATOR]\n");
		KillThreadByThreadCID( *(ULONG *)InputBuffer );
		break;

			//枚举、检测驱动
	case IOCTL_ENUM_DRIVER:
		{
			PDRVMOD_ITEM pDrvItem;
			PDRVMOD_ITEM pDrvOutput;
			ULONG	numOfDrv=0; //drv计数
			ULONG	sumOfDrv=0;	//驱动总数
			ULONG	i=0;

			dprintf("[$XTR->IOCTL_ENUM_DRIVER]\n");

			pDrvOutput=(PDRVMOD_ITEM)OutputBuffer;
			pDrvItem=GetKrnlModuleList(&numOfDrv);

			for (i=0; i<numOfDrv; i++)
			{
				//基址大于0、大于ntkrnl**.exe的基址
				if (pDrvItem[i].ulBaseAddr > 0   &&  pDrvItem[i].ulBaseAddr >= pDrvItem[0].ulBaseAddr)	//(ntkrnl**.exe总是在第1个)
				{
//					DbgPrint("基址:0x%08X \t 驱动对象:0x%08X \t 驱动路径:%ws\n", pDrvItem[i].ulBaseAddr, pDrvItem[i].pDrvObject, pDrvItem[i].pwszDrvPath);
					//把哈希表里的驱动信息拷贝到输出缓冲区
					RtlCopyMemory(&pDrvOutput[sumOfDrv], &pDrvItem[i], sizeof(DRVMOD_ITEM));
					//总数自增
					sumOfDrv++;
				}
			}

			irpInfo=sumOfDrv*sizeof(DRVMOD_ITEM);
			status=STATUS_SUCCESS;
			break;
		}

			//获取SSDT数据
	case IOCTL_GET_SSDT_INFO:
		{
			//获取ssdt数量
			ULONG	numOfSrv = GetSSDTNumber();
			ULONG	dataLen = numOfSrv*sizeof(SSDT_INFO);

			dprintf("[$XTR->IOCTL_GET_SSDT_INFO]\n");

			//ssdt数量
			if ( OutputBufferLength < dataLen )
			{
				if ( OutputBufferLength == sizeof(ULONG) )
				{
					dprintf("[$XTR->IOCTL_GET_SSDT_INFO]->GetSSDTNumber() ssdt数量:%d\n", numOfSrv);
					//ssdt个数
					*( (PULONG)OutputBuffer ) = numOfSrv;

					irpInfo=sizeof(ULONG);
					status=STATUS_SUCCESS;
					break;
				}

				status=STATUS_BUFFER_TOO_SMALL;
				break;
			}

			//清空输出缓冲区
			RtlZeroMemory(OutputBuffer, (KeServiceDescriptorTable->ntoskrnl.ServiceLimit) * sizeof(SSDT_INFO));
			//请求数据
			GetSSDTinfo( OutputBuffer );

			irpInfo = (KeServiceDescriptorTable->ntoskrnl.ServiceLimit) * sizeof(SSDT_INFO);
			status=STATUS_SUCCESS;
			break;
		}

			//恢复ssdt
	case IOCTL_RESTORE_SSDT:
		{
			dprintf("[$ixer->IOCTL_RESTORE_SSDT]\n");

			if ( !InputBuffer || InputBufferLength<sizeof(SSDTINFO) )
			{
				dprintf("输入缓冲区或长度无效!\n");

				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if ( !RestoreSSDTItem( (PSSDTINFO)InputBuffer ) )
			{
				dprintf("恢复ssdt项失败!\n");
				status=STATUS_UNSUCCESSFUL;
			}

			break;
		}

			//获取ShadowSSDT数据
	case IOCTL_GET_SHADOWSSDT_INFO:
		{
			//获取ShadowSsdt数量
			ULONG	numOfShadowSrv = GetSSDTShadowNumber();
			ULONG	dataLen = numOfShadowSrv * sizeof(SHADOW_INFO);
			KAPC_STATE pApcStatus;

			dprintf("[$XTR->IOCTL_GET_SHADOWSSDT_INFO]\n");
			//请求ShadowSsdt数量
			if ( OutputBufferLength < dataLen )
			{
				if ( OutputBufferLength == sizeof(ULONG) )
				{
					dprintf("[$XTR->IOCTL_GET_SHADOWSSDT_INFO]->GetSSDTShadowNumber() ShadowSsdt数量:%d\n", numOfShadowSrv);
					*( (PULONG)OutputBuffer ) = numOfShadowSrv;
					irpInfo = sizeof(ULONG);
					break;
				}

				status = STATUS_BUFFER_TOO_SMALL;
				break;
			}

			/*请求ShadowSsdt数据*/
			//attach to GUI process.
			status=AttachGuiProcess( &pApcStatus );
			if ( !NT_SUCCESS(status) )
				break;

			numOfShadowSrv=GetSSDTShadowFuncAddr( (PSHADOW_INFO)OutputBuffer, &dataLen);
			KdPrint(("服务函数个数=%ld\n",numOfShadowSrv));
			irpInfo = dataLen;
			status = STATUS_SUCCESS;

			//解挂靠
			KeUnstackDetachProcess(&pApcStatus);
			break;
		}

			//恢复Shadow
	case IOCTL_RESTORE_SHADOW:
		{
			dprintf("[$ixer->IOCTL_RESTORE_SHADOW]\n");
			if ( !InputBuffer || InputBufferLength<sizeof(SSDTINFO) )
			{
				dprintf("输入缓冲区或长度无效!\n");
				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if ( !RestoreShadowSSDTItem( (PSSDTINFO)InputBuffer) )
			{
				dprintf("恢复Shadow SSDT函数失败!\n");
				status=STATUS_UNSUCCESSFUL;
			}

			break;
		}

			//获取MsgHook信息
	case IOCTL_GET_MSGHOOK_INFO:
		{
			dprintf("[$ixer->IOCTL_GET_MSGHOOK_INFO]\n");
			RtlZeroMemory( OutputBuffer, OutputBufferLength);

			status = GetAllMsgHookInfo( OutputBuffer );
			irpInfo= OutputBufferLength;

			break;
		}

			//获取文件夹数目
	case IOCTL_GET_DIR_NUM:
		{
			PCHAR	ioDirName=NULL;
			PVOID	pDirInfo=NULL;
			ULONG	dwRetSize=0;
			ULONG	Num=0;

			dprintf("[$ixer->IOCTL_GET_DIR_NUM]\n");

			if ( !InputBuffer || InputBufferLength<3 || ( OutputBufferLength<sizeof(ULONG) ) )
			{
				KdPrint(("传入的参数错误\n"));
				status=STATUS_INVALID_PARAMETER;
				break;
			}

			ioDirName=(PCHAR)InputBuffer; 
			//查询目录文件信息
			pDirInfo = QueryDirectoryFileInfo( ioDirName, &dwRetSize );
			//大小/目录结构=数量
			Num = dwRetSize/sizeof(DIRECTORY_INFO);

			KdPrint(("目录数量:%d\n", Num));
			//把文件夹数拷贝到输出缓冲区
			RtlCopyMemory( OutputBuffer, &Num, sizeof(ULONG) );

			IxExFreePool( pDirInfo );
			irpInfo = sizeof(ULONG);
			break;
		}

		//获取文件夹、目录文件信息
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
				KdPrint(("传入的参数错误\n"));
				status=STATUS_INVALID_PARAMETER;
				break;
			}

			ioDirName=(PCHAR)InputBuffer;
			//查询目录文件数据
			pDirInfo = QueryDirectoryFileInfo( ioDirName, &dwRetSize );

			//把文件夹信息拷贝到输出缓冲区
			RtlCopyMemory( OutputBuffer, pDirInfo, dwRetSize );

			KdPrint(("返回大小：%d\n", dwRetSize));
			pDirFile=(PDIRECTORY_INFO)pDirInfo;
			//大小/目录结构=数量
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

		//删除文件
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

		//获取键尺寸
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

		//获取键
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

		//获取键值
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

		/* 删除注册表键值 */
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
			//枚举TCP
	case IOCTL_ENUM_TCP:
		{
			PVOID	pOut=NULL;
			ULONG	OutLen=0; 

			dprintf("[$ixer->IOCTL_ENUM_TCP]\n");

			if( OutputBufferLength < sizeof(CONNINFO102) )
			{
				KdPrint(("输出缓冲区长度无效\n"));

				status=STATUS_BUFFER_OVERFLOW;
				break;
			}

			pOut = EnumPortInformation( &OutLen, TCPPORT );
			if(!pOut)
			{
				KdPrint(("获取TCP端口信息失败!\n"));

				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if( OutputBufferLength<OutLen )
			{
				KdPrint(("输出缓冲区太小,应为%ld\n", OutLen));

				IxExFreePool(pOut);
				status=STATUS_BUFFER_OVERFLOW;
				break;
			} 

			RtlCopyMemory(OutputBuffer, pOut, OutLen);

			IxExFreePool(pOut);
			irpInfo = OutLen;
			break;
		}

			//枚举UDP
	case IOCTL_ENUM_UDP:
		{
			PVOID	pOut=NULL;
			ULONG	OutLen=0;  

			dprintf("[$ixer->IOCTL_ENUM_UDP]\n");

			if( OutputBufferLength < sizeof(UDPCONNINFO) )
			{
				KdPrint(("输出缓冲区长度无效\n"));

				status=STATUS_BUFFER_OVERFLOW;
				break;
			}

			pOut = EnumPortInformation( &OutLen, UDPPORT );
			if(!pOut)
			{
				KdPrint(("获取UDP端口信息失败!\n"));

				status=STATUS_UNSUCCESSFUL;
				break;
			}

			if( OutputBufferLength<OutLen )
			{
				KdPrint(("输出缓冲区太小,应为%ld\n",OutLen));

				IxExFreePool(pOut);
				status=STATUS_BUFFER_OVERFLOW;
				break;
			}

			RtlCopyMemory( OutputBuffer, pOut, OutLen );

			IxExFreePool(pOut);
			irpInfo = OutLen;
			break;
		}

	/* 其他情况 */
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