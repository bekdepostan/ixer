/*===================================================================
 * Filename msghook.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 消息钩子
 *
 * Date:   2013-5-18 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "proc.h"
#include "WinOSVersion.h"
#include "common.h"
#include "msghook.h"


ULONG	aphkStartOffset = 0x0f4;
ULONG	pDeskInfoOffset = 0x40;
CHAR	CurrentName[16];
CHAR	CurrentImagePath[256];
ULONG	Index;

/*========================================================================
*
* 函数名:	GetAllMsgHookInfo
*
* 参数:		PHOOK_INFO	[I/O]	HookInfo
*
* 功能描述:	获取所有线程的Hook信息
*
* 返回值:	NTSTATUS
*
=========================================================================*/
NTSTATUS
GetAllMsgHookInfo( PHOOK_INFO HookInfo )
{
	NTSTATUS	Status;
	PLIST_ENTRY	Entry;
	PETHREAD	Thread;
	PEPROCESS	Process;

	Status = STATUS_SUCCESS;

	if (PsActiveProcessHead == NULL)
	{
		KdPrint(( "$ixer:\tPsActiveProcessHead == NULL" ));
		return STATUS_INVALID_PARAMETER;
	}

	/* 遍历 ActiveProcessList */
	Entry = PsActiveProcessHead->Flink;

	Index = 0;

	while (Entry != PsActiveProcessHead)
	{
		if ((*(ULONG *)((ULONG)Entry - g_EPROCESS_ActiveProcessLinks + g_EPROCESS_Flags) & 0x4) == 0)
		{
			Process = (PEPROCESS)((ULONG)Entry - g_EPROCESS_ActiveProcessLinks);
			// DbgPrint( "%s", PsGetProcessImageFileName( Process ) );

			memset( CurrentName, 0, 16 );
			RtlZeroMemory( CurrentImagePath, 256 );
			// 
			strncpy( CurrentName, PsGetProcessImageFileName( Process ), 16 );
			//获取镜像路径
			GetImageFullPathByEprocess( Process,  CurrentImagePath );

			__try
			{
				/* 遍历进程的线程 */
				for (Thread = GetNextProcessThread( Process, NULL );
									Thread != NULL;
						Thread = GetNextProcessThread( Process, Thread ))
				{
					Status = STATUS_SUCCESS;

					//遍历线程获取的钩子信息
					GetMsgHookInfo( HookInfo, Thread );

				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				Status = GetExceptionCode();

				KdPrint(( "$ixer:\tEXCEPTION_EXECUTE_HANDLER:\t0x%08x", Status ));
			}
		}

		Entry = Entry->Flink;
	}

	return Status;
}

/*========================================================================
*
* 函数名:	GetMsgHookInfo
*
* 参数:		PHOOK_INFO	[I/O]	HookInfo
*			PETHREAD	[IN]	Thread
*
* 功能描述:	获取此线程的Hook信息
*
* 返回值:	VOID
*
=========================================================================*/
VOID GetMsgHookInfo( PHOOK_INFO HookInfo, PETHREAD Thread )
{
	PHOOK		phk;
	ULONG		n;

	_gptiCurrent =  *((PULONG)((ULONG)Thread + g_ETHREAD_Win32Thread));

	//
	KdPrint(( "pethread:\t%x", Thread ));
	KdPrint(( "_gptiCurrent:\t%x", _gptiCurrent ));
	//

	if (_gptiCurrent == 0)
	{
		return;
	}
	n = WH_MINHOOK;

	while (n != WH_MAXHOOK)
	{
		phk = (PHOOK)PhkFirstValid( _gptiCurrent, n );

		//
		KdPrint(( "phk:\t%x", phk ));
		//

		while (phk)
		{
			if (phk->ihmod != -1)
			{
				HookInfo[Index].FuncBaseAddr = ((PULONG)(*((PULONG)(_gptiCurrent + 0x2C)) + 0xA8))[phk->ihmod];
			}
			else
			{
				HookInfo[Index].FuncBaseAddr = -1;
			}

//			cid = (PCLIENT_ID)((ULONG)Thread + g_ETHREAD_CidOffset);

			HookInfo[Index].Handle			= phk->hmodule;
			HookInfo[Index].FuncOffset		= phk->offPfn;
			HookInfo[Index].iHook			= phk->iHook;
			HookInfo[Index].tid				= (ULONG)PsGetThreadId( Thread );			//(ULONG)cid->UniqueThread;
			HookInfo[Index].pid				= (ULONG)PsGetThreadProcessId( Thread );	//(ULONG)cid->UniqueProcess;

			strncpy( HookInfo[Index].ProcessName, CurrentName, 16 );
			strncpy( HookInfo[Index].ProcessPath, CurrentImagePath, 256);

			//
			KdPrint(("*****************************"));
			KdPrint(("* index:\t%d", Index));
			KdPrint(("* hmodule:\t%x", HookInfo[Index].Handle));
			KdPrint(("* offPfn:\t%x", HookInfo[Index].FuncOffset));
			KdPrint(("* FuncBaseAddr:\t%x", HookInfo[Index].FuncBaseAddr));
			KdPrint(("* iHook:\t%x", HookInfo[Index].iHook));
			KdPrint(("* tid:\t%d", HookInfo[Index].tid));
			KdPrint(("* pid:\t%d", HookInfo[Index].pid));
			DbgPrint("* Name:\t%s", HookInfo[Index].ProcessName);
			DbgPrint("* imagePath:\t%s", HookInfo[Index].ProcessPath);
			DbgPrint("*****************************");
			//

			Index++;
			phk = PhkNextValid( phk );
		}

		n++;
	}
}

PHOOK PhkFirstValid( ULONG pti, int nFilterType )
{
	PHOOK phk;

	phk = ((PHOOK *)(pti + aphkStartOffset))[nFilterType + 1];

	if (phk == NULL)
	{
		phk = (*(PDESKTOPINFO *)(pti + pDeskInfoOffset))->aphkStart[nFilterType + 1];
	}

	if ((phk != NULL) && (phk->flags & HF_DESTROYED))
	{
		phk = PhkNextValid( phk );
	}

	return phk;
}


PHOOK PhkNextValid( PHOOK phk )
{
	do
	{
		if (phk->phkNext != 0)
		{
			phk = (PHOOK)phk->phkNext;
		}
		else if (!(phk->flags & HF_GLOBAL))
		{
			phk = (*(PDESKTOPINFO *)(_gptiCurrent + pDeskInfoOffset))->aphkStart[phk->iHook + 1];
		}
		else
		{
			return NULL;
		}

	} while ((phk != NULL) && (phk->flags & HF_DESTROYED));

	return phk;
}