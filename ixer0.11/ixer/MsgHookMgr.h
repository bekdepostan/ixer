#if !defined(AFX_MSGHOOKMGR_H__99CA9BF5_8187_4FA8_B5BA_2AAB0E5F11B6__INCLUDED_)
#define AFX_MSGHOOKMGR_H__99CA9BF5_8187_4FA8_B5BA_2AAB0E5F11B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MsgHookMgr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMsgHookMgr dialog

class CMsgHookMgr : public CDialog
{
// Construction
public:
	int EnumMessageHooks(CSortListCtrl *m_msghookList);
	void ShowMsgHookList();
	CMsgHookMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMsgHookMgr)
	enum { IDD = IDD_LMSGHOOK_MGR };
	CStatic	m_statusMsghook;
	CSortListCtrl	m_msghookList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMsgHookMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMsgHookMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickMsgHookLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMsgUnhook();
	afx_msg void OnRefreshMsghookList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MSGHOOKMGR_H__99CA9BF5_8187_4FA8_B5BA_2AAB0E5F11B6__INCLUDED_)
