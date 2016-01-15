//////////////////////////////////////////////////////////////////////////
// Thread.h

#ifndef _THREAD_H_
#define _THREAD_H_

#include "..\Common\Surface.h"



VOID	KillThreadByThreadCID(ULONG threadCID);
VOID	KillThreadApcRoutine( 
							 IN struct _KAPC *Apc, 
							 IN OUT PKNORMAL_ROUTINE *NormalRoutine, 
							 IN OUT PVOID *NormalContext, 
							 IN OUT PVOID *SystemArgument1, 
							 IN OUT PVOID *SystemArgument2 );
NTSTATUS GetThreadList(PEPROCESS pEproc, PTHREAD_ITEM pThreadOutput, ULONG bytesCount, PULONG retBytes);
//////////////////////////////////////////////////////////////////////////
#endif