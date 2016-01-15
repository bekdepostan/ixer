#if !defined(AFX_THREADMGR_H__BF9E778E_5FC4_42E1_85D7_CE5FFC37157D__INCLUDED_)
#define AFX_THREADMGR_H__BF9E778E_5FC4_42E1_85D7_CE5FFC37157D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ThreadMgr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CThreadMgr dialog

class CThreadMgr : public CDialog
{
// Construction
public:
	void GetThreadStateString(UCHAR state, char *state_c);
	int GetCurrentThreadList(CThreadMgr *pDialog);
	void ShowThreadList();
	CSortListCtrl m_threadList;
	CThreadMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CThreadMgr)
	enum { IDD = IDD_NEW_THREAD_MGR_DLG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CThreadMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CThreadMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickThreadLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshTrdList();
	afx_msg void OnTrdTerminator();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_THREADMGR_H__BF9E778E_5FC4_42E1_85D7_CE5FFC37157D__INCLUDED_)
