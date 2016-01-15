//////////////////////////////////////////////////////////////////////////
// MsgHook.h

#ifndef	___MSGHOOK___
#define ___MSGHOOK___

//#include "StdAfx.h"

//extern NTSYSTEMDEBUGCONTROL			NtSystemDebugControl ;
//////////////////////////////////////////////////////////////////////////
typedef enum  _HANDLE_TYPE
{
	TYPE_FREE = 0  ,                  //must be zero!
        TYPE_WINDOW = 1 ,                 //in order of use for C code lookups
        TYPE_MENU = 2,
        TYPE_CURSOR = 3,
        TYPE_SETWINDOWPOS = 4,
        TYPE_HOOK = 5,
        TYPE_CLIPDATA = 6  ,              //clipboard data
        TYPE_CALLPROC = 7,
        TYPE_ACCELTABLE = 8,
        TYPE_DDEACCESS = 9,
        TYPE_DDECONV = 10,
        TYPE_DDEXACT = 11,          //DDE transaction tracking info.
        TYPE_MONITOR = 12,
        TYPE_KBDLAYOUT = 13   ,           //Keyboard Layout handle (HKL) object.
        TYPE_KBDFILE = 14    ,            //Keyboard Layout file object.
        TYPE_WINEVENTHOOK = 15  ,         //WinEvent hook (EVENTHOOK)
        TYPE_TIMER = 16,
        TYPE_INPUTCONTEXT = 17  ,         //Input Context info structure
        TYPE_CTYPES = 18         ,        //Count of TYPEs; Must be LAST + 1
        TYPE_GENERIC = 255               //used for generic handle validation
}HANDLE_TYPE;

typedef enum  _HOOK_TYPE{
	MY_WH_MSGFILTER = -1,
        MY_WH_JOURNALRECORD = 0,
        MY_WH_JOURNALPLAYBACK = 1,
        MY_WH_KEYBOARD = 2,
        MY_WH_GETMESSAGE = 3,
        MY_WH_CALLWNDPROC = 4,
        MY_WH_CBT = 5,
        MY_WH_SYSMSGFILTER = 6,
        MY_WH_MOUSE = 7,
        MY_WH_HARDWARE = 8,
        MY_WH_DEBUG = 9,
        MY_WH_SHELL = 10,
        MY_WH_FOREGROUNDIDLE = 11,
        MY_WH_CALLWNDPROCRET = 12,
        MY_WH_KEYBOARD_LL = 13,
        MY_WH_MOUSE_LL = 14
}HOOK_TYPE;


typedef struct HANDLEENTRY{
	PVOID  phead;          //pointer to the real object
	ULONG  pOwner;          //pointer to owning entity (pti or ppi)
	BYTE  bType;           //type of object
	BYTE  bFlags;			//flags - like destroy flag
	short  wUniq;			//uniqueness count
}HANDLEENTRY,*PHANDLEENTRY;

typedef struct SERVERINFO{ 	//si
	short wRIPFlags ;               //RIPF_ flags
	short wSRVIFlags ;              //SRVIF_ flags
	short wRIPPID ;              //PID of process to apply RIP flags to (zero means all)
	short wRIPError ;              //Error to break on (zero means all errors are treated equal)
	ULONG cHandleEntries;          //count of handle entries in array
}SERVERINFO,*PSERVERINFO;

typedef struct SHAREDINFO{
	PSERVERINFO psi;                   //tagSERVERINFO
	PHANDLEENTRY aheList;                //_HANDLEENTRY - handle table pointer
	ULONG pDispInfo;              //global displayinfo
	ULONG ulSharedDelta;           //delta between client and kernel mapping of ...
}SHAREDINFO,*PSHAREDINFO; 



typedef struct HEAD
{
	HANDLE h;
	ULONG cLockObj;
}HEAD;

typedef struct THROBJHEAD
{
	HEAD headinfo;
	PVOID pti;		//PTHREADINFO
}THROBJHEAD;

typedef  struct DESKHEAD
{
	PVOID rpdesk; //PDESKTOP
	PBYTE pSelf ; //PBYTE
}DESKHEAD;

typedef struct THRDESKHEAD
{
	THROBJHEAD ThreadObjHead ;
	DESKHEAD DesktopHead ;
}THRDESKHEAD;

typedef  struct HOOK  //hk
{
	THRDESKHEAD tshead ;
	HOOK * phkNext ;
	HOOK_TYPE iHook;		// WH_xxx hook type
	ULONG offPfn;			// º¯ÊýµØÖ·Æ«ÒÆ
	UINT flags ;			// HF_xxx flags
	int  ihmod ;
	PVOID ptiHooked;		// PTHREADINFO - Thread hooked.
	PVOID rpdesk ;          // Global hook pdesk. Only used when  hook is locked and owner is destroyed
}HOOK,*PHOOK;

typedef struct W32THREAD
{
	//W32THREAD
    PVOID    pEThread ;
	ULONG   RefCount ;
	ULONG  ptlW32 ;
	ULONG  pgdiDcattr ;
	ULONG   pgdiBrushAttr ;
	ULONG   pUMPDObjs ;
    ULONG    pUMPDHeap ;
    ULONG    dwEngAcquireCount ;
    ULONG    pSemTable ;
    ULONG    pUMPDObj ;
	//*********************
	PVOID ptl;
	PVOID ppi;
}W32THREAD;
  

typedef struct MsgHookInfo
{
	HANDLE  hHook ;
    HOOK_TYPE     iHookType;
	PVOID   pEThread ;
    ULONG    offPfn ;
	PVOID modBase;
	PVOID funAdd;
	ULONG TID;
	ULONG PID;
}MsgHookInfo;


//short ReadMemoryToInt( ULONG dwAddress);
int EnumMessageHooks(CSortListCtrl *m_msghookList);

//////////////////////////////////////////////////////////////////////////
#endif
