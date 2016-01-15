/*===================================================================
 * Filename ssdt.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 系统服务分派表
 *
 * Date:   2013-5-12 Original from Chinghoi
 *
 * Version: 1.0
 ==================================================================*/

#ifndef _SSDT_H_
#define _SSDT_H_


/* 头文件 ================================================= */
#include <ntddk.h>
#include "ntdefine.h"
#include "..\Common\Surface.h"

/*
typedef NTSTATUS (NTAPI *NTPROC)();
typedef NTPROC* PNTPROC;

typedef struct _SYSTEM_SERVICE_TABLE
{
	PNTPROC ServiceTable; // array of entry points
	PULONG CounterTable; // array of usage counters
	ULONG ServiceLimit; // number of table entries
	PCHAR ArgumentTable; // array of byte counts
}
SYSTEM_SERVICE_TABLE,
*PSYSTEM_SERVICE_TABLE,
**PPSYSTEM_SERVICE_TABLE;

typedef struct _SERVICE_DESCRIPTOR_TABLE
{
	SYSTEM_SERVICE_TABLE ntoskrnl; // ntoskrnl.exe (native api)
	SYSTEM_SERVICE_TABLE win32k; // win32k.sys (gdi/user support)
	SYSTEM_SERVICE_TABLE Table3; // not used
	SYSTEM_SERVICE_TABLE Table4; // not used
}
SYSTEM_DESCRIPTOR_TABLE,
*PSYSTEM_DESCRIPTOR_TABLE,
**PPSYSTEM_DESCRIPTOR_TABLE;
*/
/* 函数声明 ================================================ */

extern PSYSTEM_DESCRIPTOR_TABLE KeServiceDescriptorTable;

ULONG
GetSSDTNumber( VOID );

VOID
GetSSDTinfo( PVOID AddrBuffer );

BOOLEAN
CompareModule( PVOID Base, PDRVMOD_ITEM pDrvItem );

VOID
GetSSDTAddrByPhyFile( PDRVMOD_ITEM pDrvItem, PSSDT_INFO Record );

VOID
GetSSDTNameByPhyFile( PSSDT_INFO Record );

PVOID
FindRealSSDTaddr( PVOID BaseAddress );

BOOLEAN
RestoreSSDTItem(IN PSSDTINFO	pInfo);

ULONG
SetSSDTServiceAddr(IN unsigned int Index,IN ULONG	NewServiceAdd);

extern
int swprintf(wchar_t *,const wchar_t *,...);


#endif