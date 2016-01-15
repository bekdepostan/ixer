#if !defined(AFX_ADDNEWKEYITEM_H__EE20EC78_EF00_4118_B99A_62DD9A63EE1C__INCLUDED_)
#define AFX_ADDNEWKEYITEM_H__EE20EC78_EF00_4118_B99A_62DD9A63EE1C__INCLUDED_

#include "rootkit.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddNewKeyItem.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddNewKeyItem dialog

class CAddNewKeyItem : public CDialog
{
// Construction
public:
	CString strRegType;
	DWORD dwIndex;
	BOOL m_bWinText;
	CAddNewKeyItem(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddNewKeyItem)
	enum { IDD = IDD_REG_ADDITEM };
	CString	m_newItem;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddNewKeyItem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddNewKeyItem)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDNEWKEYITEM_H__EE20EC78_EF00_4118_B99A_62DD9A63EE1C__INCLUDED_)
