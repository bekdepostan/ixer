//////////////////////////////////////////////////////////////////////////
// thread.c

#include "precomp.h"
#include "func.h"
#include "WinOSVersion.h"
#include "Thread.h"



DWORD	g_PspExitThread=0;		//PspExitThreadδ����������ַ
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
	threadListHead = ( (ULONG)pEproc + g_EPROCESS_ThreadListHead); //*(DWORD*)
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

/*========================================================================
*
* ������:	KillThreadByThreadID
*
* ����:		ULONG		[IN]	killedThreadPID
*
* ��������:	��APC�����߳�ͨ���߳�CID
*
* ����ֵ:	void
*
=========================================================================*/
void KillThreadByThreadCID(ULONG threadCID)
{
	NTSTATUS	ntStatus;
	HANDLE		hThread=NULL;
	PETHREAD	pEthread;
	PKAPC		Kapc;
	ULONG		killedProcessPID=-1;

	//���֮ǰû�еõ�δ��������PspExitThread �Ļ���ֻ���˳�
	if(g_PspExitThread==0)
	{
		DbgPrint("PspExitThread=0\n");
		return;
	}

	ntStatus = PsLookupThreadByThreadId( (PVOID)threadCID, &pEthread );
	if (ntStatus != STATUS_SUCCESS) //û�õ��߳�object���򷵻�
	{
		DbgPrint("[Thread.c]:PsLookupThreadByThreadId(%x) failed!\n", threadCID);
		return;
	}

	//����һ��KAPC�ռ�
	Kapc = (PKAPC)ExAllocatePool( NonPagedPool, sizeof(KAPC) );
	//��ʼ��KAPC
	KeInitializeApc( Kapc, 
					(PKTHREAD)pEthread, 
					0, 
					KillThreadApcRoutine, 
					NULL, 
					NULL, 
					KernelMode, 
					NULL );
	//����APC���ý��̣���������ǰ�߳�
	if (KeInsertQueueApc( Kapc, Kapc, NULL, 2))
	{
		dprintf("[Thread.c] �����߳�CID:%d �ɹ�..\n", threadCID);
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