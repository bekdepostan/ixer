// ixer.h : main header file for the IXER application
//

#if !defined(AFX_IXER_H__AEDB1578_C437_4A90_9DCF_AC6B86139C07__INCLUDED_)
#define AFX_IXER_H__AEDB1578_C437_4A90_9DCF_AC6B86139C07__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <afxtempl.h>
#include "resource.h"		// main symbols


extern ULONG	g_dwPId;		//进程ID(选中item的进程ID)

//公共函数声明
void MyOutputDebugString(const char * sz, ...);
VOID	showErrBox( void );
/////////////////////////////////////////////////////////////////////////////
// CixerApp:
// See ixer.cpp for the implementation of this class
//

class CixerApp : public CWinApp
{
public:
	CixerApp();	//构造函数

//	bool            m_bVerInfo;//是否存在版本信息

// Overrides 重载
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CixerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation 实现

	//{{AFX_MSG(CixerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG

public:

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IXER_H__AEDB1578_C437_4A90_9DCF_AC6B86139C07__INCLUDED_)
