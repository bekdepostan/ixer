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

	//��ȡ�ý��̵��߳�����ͷ
	threadListHead = (ULONG)pEproc + g_EPROCESS_ThreadListHead;
	nextTread = *(DWORD*)threadListHead;
	//�����߳�
	while(nextTread != threadListHead)
	{
		//�õ��̶߳���ָ��
		eThread = nextTread - g_ETHREAD_ThreadListEntry;
		KdPrint(("cid:%d \t eThread:0x%08X \n", *(ULONG*)( eThread+g_ETHREAD_UniqueThreadID ), eThread));

		pThread->tid = *(ULONG*)( eThread+g_ETHREAD_UniqueThreadID );							//�߳�TID
		pThread->eThread = eThread;																//ETHREAD
		pThread->teb = *(ULONG*)( eThread+g_ETHREAD_Teb );										//Teb
		pThread->priority= *(CHAR *)( eThread+g_ETHREAD_Priority );								//���ȼ�
		pThread->win32ThreadStartAddr = *(PVOID *)( eThread+g_ETHREAD_win32ThreadStartAddr );	//�߳���ڵ�ַ
		pThread->contextSwitches= *(ULONG *)( eThread+g_ETHREAD_ContextSwitches );				//�л�����
		pThread->state = *(UCHAR *)( eThread+g_ETHREAD_State );									//״̬

		if ( bytesCount < sizeof(THREAD_ITEM)*(index+1) )
		{
			if (pThread)
				ExFreePool(pThread);

			*retBytes=0;
			return STATUS_UNSUCCESSFUL;
		}

		//����ṹ�����ָ��R3�Ľṹ�����黺������
		RtlCopyMemory(&pThreadOutput[index], pThread, sizeof(THREAD_ITEM));
		index++;

		//��һ���߳�
		nextTread = *(DWORD*)nextTread;
	}

	if (pThread)
		ExFreePool(pThread);

	*retBytes=(index)*sizeof(THREAD_ITEM);

	return STATUS_SUCCESS;
}