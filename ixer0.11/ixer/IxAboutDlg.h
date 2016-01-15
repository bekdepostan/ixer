#if !defined(AFX_IXABOUTDLG_H__6B0FD673_A18C_455D_93A0_4E18A0DD88C6__INCLUDED_)
#define AFX_IXABOUTDLG_H__6B0FD673_A18C_455D_93A0_4E18A0DD88C6__INCLUDED_

#include "UICLASS\HYPERLINK\Hyperlink.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// IxAboutDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CIxAboutDlg dialog

class CIxAboutDlg : public CDialog
{
// Construction
public:
	CHyperLink m_BlogLink;
	CHyperLink m_MicroLink;
	CIxAboutDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CIxAboutDlg)
	enum { IDD = IDD_LABOUT_DLG };
	CEdit	m_Emial;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIxAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CIxAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IXABOUTDLG_H__6B0FD673_A18C_455D_93A0_4E18A0DD88C6__INCLUDED_)
