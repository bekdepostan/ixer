//////////////////////////////////////////////////////////////////////////
// Thread.h

#ifndef _THREAD_H_
#define _THREAD_H_

#include "..\Common\DataStruct.h"



NTSTATUS GetThreadList(PEPROCESS pEproc, PTHREAD_ITEM pThreadOutput, ULONG bytesCount, PULONG retBytes);
//////////////////////////////////////////////////////////////////////////
#endif