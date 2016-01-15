#if !defined(AFX_PROCESSMGR_H__47FCF5C1_8328_4C7B_B923_FB0590241DD7__INCLUDED_)
#define AFX_PROCESSMGR_H__47FCF5C1_8328_4C7B_B923_FB0590241DD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// processMgr.h : header file
//
//#include "UIClass/ControlPos/ControlPos.h"

//½ø³ÌListCtrl¿Ø¼þ
/////////////////////////////////////////////////////////////////////////////
// CProcessMgr dialog

class CProcessMgr : public CDialog
{
// Construction
public:
	BOOL SetClipboardText(char *pText);
	CImageList images;
	CMap<CString, LPCTSTR, int, int> iconMap;
	int nDefaultIconIndex;
	int GetIconIndex(char *path);
	int GetCurrentProcessList(CSortListCtrl * m_proList);
	void ShowProcessList();
	CProcessMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProcessMgr)
	enum { IDD = IDD_LPROC_MGR };
	CStatic	m_statusProcess;
	CSortListCtrl	m_proList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessMgr)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcessMgr)
	afx_msg void OnRclickProcessLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshProList();
	afx_msg void OnProModuleList();
	afx_msg void OnProThreadList();
	virtual BOOL OnInitDialog();
	afx_msg void OnProcKiller();
	afx_msg void OnCopyProcName();
	afx_msg void OnCopyProcPath();
	afx_msg void OnGpsProcFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCESS_DLG_H__47FCF5C1_8328_4C7B_B923_FB0590241DD7__INCLUDED_)
