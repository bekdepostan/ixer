#if !defined(AFX_DRVMODMGR_H__2B8D5D42_64CF_4D2B_A4FA_252F7B180BCD__INCLUDED_)
#define AFX_DRVMODMGR_H__2B8D5D42_64CF_4D2B_A4FA_252F7B180BCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DrvModMgr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDrvModMgr dialog

class CDrvModMgr : public CDialog
{
// Construction
public:
	int ReadySrvName(PDRV_SRVNAME pDrvSrvName);
	int GetDrvModList(CSortListCtrl * m_DrvModList);
	void ShowDrvModList();
	CDrvModMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDrvModMgr)
	enum { IDD = IDD_LDRV_MGR };
	CStatic	m_statusDrvMod;
	CSortListCtrl m_DrvModList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrvModMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDrvModMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickDrvmodLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshDrvList();
	afx_msg void OnCopyDrvName();
	afx_msg void OnCopyDrvPath();
	afx_msg void OnGpsDrvFile();
	afx_msg void OnDeleteDrvfile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRVMODMGR_H__2B8D5D42_64CF_4D2B_A4FA_252F7B180BCD__INCLUDED_)
