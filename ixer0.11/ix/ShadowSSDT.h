/*===================================================================
 * Filename ShadowSSDT.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: ShadowSSDT
 *
 * Date:   2013-5-14 Original from Chinghoi
 *
 * Version: 1.0
 ==================================================================*/

#ifndef _SHADOW_SSDT_H_
#define _SHADOW_SSDT_H_

#include "..\Common\Surface.h"
#include "common.h"



HANDLE
GetCsrssPid();

NTSTATUS
AttachGuiProcess( PRKAPC_STATE pApcStatus );

ULONG
GetSSDTShadowFuncAddr( PSHADOW_INFO pShadowInfo, PULONG outputLen);

ULONG
GetSSDTShadowTableAddr();

PSYSTEM_SERVICE_TABLE_SHADOW
GetSSDTShadowTable();

ULONG
GetSSDTShadowNumber( VOID );

BOOLEAN
RestoreShadowSSDTItem( IN	PSSDTINFO	pInfo );

//////////////////////////////////////////////////////////////////////////
#endif