// ixerDlg.h : header file
//

#if !defined(AFX_IXERDLG_H__D7BBA616_580E_41AA_8F08_B63D1252223F__INCLUDED_)
#define AFX_IXERDLG_H__D7BBA616_580E_41AA_8F08_B63D1252223F__INCLUDED_

#include "processMgr.h"	// Added by ClassView
#include <winsvc.h>
//#include "UIClass/ControlPos/ControlPos.h"
#include "rootkit.h"	// Added by ClassView

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CixerDlg dialog

class CixerDlg : public CDialog
{
// Construction
public:
	CRect mainTabOldRec;
	char * GetRandomString(char *sTargetBuff, int  nMinCharNum, int  nMaxCharNum, BOOL useDigit, BOOL useLowercase, BOOL useUppercase, BOOL useSpecialChar, char *customChars);
	BOOL Deleteservice(CHAR *szSvrName);
	BOOL StartOrStopService( char *pszSrvName, BOOL boStartOrStop );
	BOOL LoadNTDriver( char *pszDriverName, char *pszDriverImagePath );
	
	CixerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CixerDlg)
	enum { IDD = IDD_IXER_DIALOG };
	CTabCtrl	m_tab;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CixerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CixerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeTabMainView(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IXERDLG_H__D7BBA616_580E_41AA_8F08_B63D1252223F__INCLUDED_)
