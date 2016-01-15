//////////////////////////////////////////////////////////////////////////
// thread.c

#include "precomp.h"
#include "WindowsVersion.h"
#include "Thread.h"

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
	threadListHead = (ULONG)pEproc + g_EPROCESS_ThreadListHead;
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