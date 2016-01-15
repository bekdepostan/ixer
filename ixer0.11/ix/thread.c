//////////////////////////////////////////////////////////////////////////
// thread.c

#include "precomp.h"
#include "func.h"
#include "WinOSVersion.h"
#include "Thread.h"



DWORD	g_PspExitThread=0;		//PspExitThread未导出函数地址
NTSTATUS GetThreadList(PEPROCESS pEproc, PTHREAD_ITEM pThreadOutput, ULONG bytesCount, PULONG retBytes)
{
//	LARGE_INTEGER	currTime, lastTime;
	ULONG	threadListHead, nextTread;
	ULONG	eThread, index=0;
	PTHREAD_ITEM pThread=NULL;
	pThread=(PTHREAD_ITEM)ExAllocatePoolWithTag(NonPagedPool, sizeof(THREAD_ITEM), 'thr');
	if (!pThread)
		return STATUS_UNSUCCESSFUL;

	//获取该进程的线程链表头
	threadListHead = ( (ULONG)pEproc + g_EPROCESS_ThreadListHead); //*(DWORD*)
	nextTread = *(DWORD*)threadListHead;
	//遍历线程
	while(nextTread != threadListHead)
	{
		//得到线程对象指针
		eThread = nextTread - g_ETHREAD_ThreadListEntry;
		KdPrint(("cid:%d \t eThread:0x%08X \n", *(ULONG*)( eThread+g_ETHREAD_UniqueThreadID ), eThread));

		pThread->tid = *(ULONG*)( eThread+g_ETHREAD_UniqueThreadID );							//线程TID
		pThread->eThread = eThread;																//ETHREAD
		pThread->teb = *(ULONG*)( eThread+g_ETHREAD_Teb );										//Teb
		pThread->priority= *(CHAR *)( eThread+g_ETHREAD_Priority );								//优先级
		pThread->win32ThreadStartAddr = *(PVOID *)( eThread+g_ETHREAD_win32ThreadStartAddr );	//线程入口地址
		pThread->contextSwitches= *(ULONG *)( eThread+g_ETHREAD_ContextSwitches );				//切换次数
		pThread->state = *(UCHAR *)( eThread+g_ETHREAD_State );									//状态

		if ( bytesCount < sizeof(THREAD_ITEM)*(index+1) )
		{
			if (pThread)
				ExFreePool(pThread);

			*retBytes=0;
			return STATUS_UNSUCCESSFUL;
		}

		//整块结构体存入指向R3的结构体数组缓冲区中
		RtlCopyMemory(&pThreadOutput[index], pThread, sizeof(THREAD_ITEM));
		index++;

		//下一个线程
		nextTread = *(DWORD*)nextTread;
	}

	if (pThread)
		ExFreePool(pThread);

	*retBytes=(index)*sizeof(THREAD_ITEM);

	return STATUS_SUCCESS;
}

/*========================================================================
*
* 函数名:	KillThreadByThreadID
*
* 参数:		ULONG		[IN]	killedThreadPID
*
* 功能描述:	插APC结束线程通过线程CID
*
* 返回值:	void
*
=========================================================================*/
void KillThreadByThreadCID(ULONG threadCID)
{
	NTSTATUS	ntStatus;
	HANDLE		hThread=NULL;
	PETHREAD	pEthread;
	PEPROCESS	pEprocess;
	PKAPC		Kapc;
	ULONG		killedProcessPID=-1;

	//如果之前没有得到未导出函数PspExitThread 的话，只好退出
	if(g_PspExitThread==0)
	{
		DbgPrint("PspExitThread=0\n");
		return;
	}

	ntStatus = PsLookupThreadByThreadId( (PVOID)threadCID, &pEthread );
	if (ntStatus != STATUS_SUCCESS) //没得到线程object，则返回
	{
		DbgPrint("[Thread.c]:PsLookupThreadByThreadId(%x) failed!\n", threadCID);
		return;
	}

	//申请一个KAPC空间
	Kapc = (PKAPC)ExAllocatePool( NonPagedPool, sizeof(KAPC) );
	//初始化KAPC
	KeInitializeApc( Kapc, 
					(PKTHREAD)pEthread, 
					0, 
					KillThreadApcRoutine, 
					NULL, 
					NULL, 
					KernelMode, 
					NULL );
	//插入APC到该进程，来结束当前线程
	if (KeInsertQueueApc( Kapc, Kapc, NULL, 2))
	{
		dprintf("[Thread.c] 结束线程CID:%d 成功..\n", threadCID);
	}

	ObDereferenceObject(pEthread);
	return;
}

VOID KillThreadApcRoutine( IN struct _KAPC *Apc, IN OUT PKNORMAL_ROUTINE *NormalRoutine, IN OUT PVOID *NormalContext, IN OUT PVOID *SystemArgument1, IN OUT PVOID *SystemArgument2 )
{
	_asm
	{
		push 0
		mov eax, g_PspExitThread
		call  eax
	}
}