#if !defined(AFX_SSDTMGR_H__AFD83EB2_A71B_417D_8F66_8DC88A88DCDA__INCLUDED_)
#define AFX_SSDTMGR_H__AFD83EB2_A71B_417D_8F66_8DC88A88DCDA__INCLUDED_

#include "..\COMMON\Surface.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SSDTMgr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSSDTMgr dialog

class CSSDTMgr : public CDialog
{
// Construction
public:
	int ssdtSeled;
	DWORD	numOfNativeAddr;
	PDWORD	pData;
	DWORD	numOfFunName;	//服务函数个数
	PSSDT_NAME pssdtName;	//服务函数名

	int GetCurrentSSDTList(CSortListCtrl *m_ssdtList);
	void ShowSSDTList();
	CSSDTMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSSDTMgr)
	enum { IDD = IDD_LSSDT_MGR };
	CStatic	m_statusSsdt;
	CSortListCtrl	m_ssdtList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSSDTMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSSDTMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickSsdtLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshSsdtList();
	afx_msg void OnSsdtRestore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SSDTRESTOREMGR_H__AFD83EB2_A71B_417D_8F66_8DC88A88DCDA__INCLUDED_)
