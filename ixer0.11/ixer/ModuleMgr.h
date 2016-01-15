#if !defined(AFX_MODULEMGR_H__48D5DA75_2903_49EB_A2B9_93CE5BA0F0B6__INCLUDED_)
#define AFX_MODULEMGR_H__48D5DA75_2903_49EB_A2B9_93CE5BA0F0B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ModuleMgr.h : header file
//

extern ULONG	g_dwPId;		//进程ID(选中item的进程ID)
extern CString	g_strProcName;	//选中item的进程名
/////////////////////////////////////////////////////////////////////////////
// CModuleMgr dialog

class CModuleMgr : public CDialog
{
// Construction
public:
	BOOL GetFileCorporation(char *pszFileFullPath, PCHAR lpszFileCorporation);
	int GetCurrentModuleList(CModuleMgr *pDialog);
	void ShowModuleList();
	CSortListCtrl m_moduleList;
	CModuleMgr(CWnd* pParent = NULL);   // standard constructor
//	CModuleMgr(char *processNameAndPidStr, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CModuleMgr)
	enum { IDD = IDD_NEW_DLL_MGR_DLG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModuleMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CModuleMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickMoudleLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshModList();
	afx_msg void OnModTerminator();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODULEMGR_H__48D5DA75_2903_49EB_A2B9_93CE5BA0F0B6__INCLUDED_)
