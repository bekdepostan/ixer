#if !defined(AFX_REGMGR_H__743D9584_B723_4B38_844E_CE6DC0BDC764__INCLUDED_)
#define AFX_REGMGR_H__743D9584_B723_4B38_844E_CE6DC0BDC764__INCLUDED_

//#include "EditSzValue.h"	// Added by ClassView
#include "rootkit.h"	// Added by ClassView
//#include "EditDwValue.h"	// Added by ClassView
//#include "AddNewKeyItem.h"	// Added by ClassView
//#include "RegSearch.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RegMgr.h : header file
//

#define MAX_LEN_KEYNAME 255
/////////////////////////////////////////////////////////////////////////////
// CRegMgr dialog

class CRegMgr : public CDialog
{
// Construction
public:
	void TranslateRegPath(CString regPath, PWCHAR krnlPath);
	CString g_currPath;
	UINT DeleteChildren(HTREEITEM hItem);
	CString m_LocalPath;
	CString GetPathFromItem( HTREEITEM hItem );
	UINT AddRegistryNodes(HTREEITEM hItem, CString &strPathName);
	BOOL GetCurrentUserRegPath( char *strCurrUserPath );
	HTREEITEM m_LastItem;
	CImageList images;
	CImageList treeImages;
// 	CRegSearch m_regSearchDlg;
// 	CAddNewKeyItem m_addItemDlg;
// 	CEditSzValue m_editSzValDlg;
	CString sztm;
//	CEditDwValue m_editDwValDlg;

	struct  _VALUE
	{
		HKEY	m_hKey;
		ULONG	iValType;
		CString szType;
        CString szName;
		CString szValue;
		CString keyPath;
	} regItem;

	BOOL bLoading, bLoading_HkeyClassesRoot_After;
	int	 maxPosition;
//	HTREEITEM hRegParent;
	BOOL nodecliek;
	void InitRootNode();
	CImageList imeregTree;
	CRegMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRegMgr)
	enum { IDD = IDD_LREG_MGR };
	CTreeCtrl	m_regTree;
	CSortListCtrl	m_regList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRegMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangingRegTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickRegLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRegDelete();
	afx_msg void OnItemexpandingRegTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshRegList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REGMGR_H__743D9584_B723_4B38_844E_CE6DC0BDC764__INCLUDED_)
