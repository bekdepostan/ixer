#if !defined(AFX_NETWORKMGR_H__9247E2BF_00A4_415D_897F_0E291CA37BFB__INCLUDED_)
#define AFX_NETWORKMGR_H__9247E2BF_00A4_415D_897F_0E291CA37BFB__INCLUDED_

#include "..\COMMON\Surface.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NetWorkMgr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNetWorkMgr dialog

class CNetWorkMgr : public CDialog
{
// Construction
public:
	void ShowNetWorkList();
	int EnumNetWorkLink( CSortListCtrl *m_netList, PDWORD numOfTcp, PDWORD numOfUdp );
	CNetWorkMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNetWorkMgr)
	enum { IDD = IDD_LNETWORK_MGR };
	CStatic	m_stusNet;
	CSortListCtrl	m_netList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNetWorkMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNetWorkMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickNetworkLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshNetList();
	afx_msg void OnConnectProKiller();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NETWORKMGR_H__9247E2BF_00A4_415D_897F_0E291CA37BFB__INCLUDED_)
