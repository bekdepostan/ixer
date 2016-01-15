// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__5526DA91_0920_4C04_8C60_7DC49461FF91__INCLUDED_)
#define AFX_STDAFX_H__5526DA91_0920_4C04_8C60_7DC49461FF91__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _WIN32_WINNT   0x0500

//MFC Header Files
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

//////////////////////////////////////////////////////////////////////////
// C RunTime Header Files
#include <string.h>

//////////////////////////////////////////////////////////////////////////
// windows Header Files
#include <winioctl.h>
#include <NTSecAPI.h>		//PUNICODE_STRING����
#include <tlhelp32.h>		//MODULEENTRY32����
#include <Winbase.h>		//GlobalAlloc����
#include <WinUser.h>		//

//����NTDLL.DLL�Ŀ�
#pragma comment ( lib,"LIB\\ntdll.lib")  
#pragma comment(linker,"/defaultlib:ntdll.lib")
#pragma comment(lib,"Kernel32.lib") 

//////////////////////////////////////////////////////////////////////////
// ����ͷ�ļ�
#include "..\Common\ioctl.h"
#include "ixer.h"
#include "..\Common\W2KShadowSSDT.h"
#include "..\Common\XPShadowSSDT.h"
#include "..\Common\W2K3ShadowSSDT.h"
#include "..\Common\VistaShadowSSDT.h"

//////////////////////////////////////////////////////////////////////////
// DataClass
#include "DataClass/TypeStruct.h"
typedef PVOID PMDL;
#include "../Common/Surface.h"
//Ŀ¼��Ϣ��չ
typedef struct _DIRECTORY_INFO_EX{
	CString			path;
	DIRECTORY_INFO  DirectoryInfo;
}DIRECTORY_INFO_EX ,*PDIRECTORY_INFO_EX;

//////////////////////////////////////////////////////////////////////////
// UIClass
#include "UIClass/SortListCtrl/SortListCtrl.h"
#include "UIClass/SortListCtrl/SortHeaderCtrl.h"

//////////////////////////////////////////////////////////////////////////
//�ⲿ����
extern HANDLE g_hDriver;
extern char g_ctrlBuff[];								//DeviceIoControl�������������buffer  

//extern DATA_MEM_POOL *g_dataMemPool;					//ring0��ring3����������ڴ�� 

extern PROCESS_ITEM		g_allProcessItems[];

extern ULONG	g_allProcessNum;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5526DA91_0920_4C04_8C60_7DC49461FF91__INCLUDED_)

