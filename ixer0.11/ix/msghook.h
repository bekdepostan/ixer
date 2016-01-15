/*===================================================================
 * Filename msghook.h
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

#ifndef _MSGHOOK_H_
#define _MSGHOOK_H_

#include "..\Common\Surface.h"
#include "Dll.h"

/* 常量、结构定义 ================================================ */

/*
 * SetWindowsHook() codes
 */
#define WH_MIN              (-1)
#define WH_MSGFILTER        (-1)
#define WH_JOURNALRECORD    0
#define WH_JOURNALPLAYBACK  1
#define WH_KEYBOARD         2
#define WH_GETMESSAGE       3
#define WH_CALLWNDPROC      4
#define WH_CBT              5
#define WH_SYSMSGFILTER     6
#define WH_MOUSE            7
#define WH_HARDWARE         8
#define WH_DEBUG            9
#define WH_SHELL           10
#define WH_FOREGROUNDIDLE  11

#define WH_CALLWNDPROCRET	12
#define WH_KEYBOARD_LL	13
#define WH_MOUSE_LL	14

#define WH_MAX             11
#define WH_MINHOOK         WH_MIN
#define WH_MAXHOOK         WH_MAX

/*
 * Hook defines.
 */
#define HF_GLOBAL          0x0001
#define HF_ANSI            0x0002
#define HF_NEEDHC_SKIP     0x0004
#define HF_HUNG            0x0008      // Hook Proc hung don't call if system
#define HF_HOOKFAULTED     0x0010      // Hook Proc faulted
#define HF_NOPLAYBACKDELAY 0x0020      // Ignore requested delay
#define HF_WX86KNOWNDLL    0x0040      // Hook Module is x86 machine type
#define HF_DESTROYED       0x0080      // Set by FreeHook

typedef struct tagHOOK
{
	ULONG	hmodule;
	ULONG	_Z_[4];
	ULONG	phkNext;	// 14h
	ULONG	iHook;		// 18h
	ULONG	offPfn;		// 1Ch
	ULONG	flags;		// 20h
	ULONG	ihmod;		// 24h
	ULONG	ptiHooked;
	ULONG	rpdesk;
} HOOK, *PHOOK;

#define CWINHOOKS       (WH_MAX - WH_MIN + 1)

typedef struct tagDESKTOPINFO
{

	PVOID				pvDesktopBase;          // For handle validation
	PVOID				pvDesktopLimit;         // ???
	ULONG				spwnd;                 // Desktop window
	ULONG				fsHooks;                // Deskop global hooks
	PHOOK				aphkStart[CWINHOOKS];  // List of hooks
	ULONG				spwndShell;            // Shell window
	ULONG				ppiShellProcess;        // Shell Process
	ULONG				spwndBkGnd;            // Shell background window
	ULONG				spwndTaskman;          // Task-Manager window
	ULONG				spwndProgman;          // Program-Manager window
	int					nShellHookPwnd;         // ???
	ULONG				papwndShellHook;		// ???
	int					cntMBox;                // ???
} DESKTOPINFO, *PDESKTOPINFO;

/* 全局变量 ================================================ */

ULONG	_gptiCurrent;


/* 函数声明 ================================================ */

NTSTATUS MsgInit( VOID );

VOID GetMsgHookInfo( PHOOK_INFO HookInfo, PETHREAD Thread );

PHOOK PhkFirstValid( ULONG pti, int nFilterType );

PHOOK PhkNextValid( PHOOK phk );

NTSTATUS
GetAllMsgHookInfo( PHOOK_INFO HookInfo );
//////////////////////////////////////////////////////////////////////////
#endif