#if !defined(AFX_FILEMGR_H__16CB4F4F_EFE1_4776_BC2B_80DC582CFA6E__INCLUDED_)
#define AFX_FILEMGR_H__16CB4F4F_EFE1_4776_BC2B_80DC582CFA6E__INCLUDED_

#include "rootkit.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileMgr.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileMgr dialog

class CFileMgr : public CDialog
{
// Construction
public:
	int	iSelected;
	CString g_currPath;
	void SetPath(CString str);
	ULONG index;
	void DeleteAllItems();
	CString m_LocalPath;
	BOOL IsDriveNode(HTREEITEM hItem);
	BOOL IsPathValid(CString &strPathName);
	DWORD FileNum;
	void AddToListView( PDIRECTORY_INFO_EX pfileDir );
	void SetButtonState(HTREEITEM hItem, CString &strPathName);
	UINT AddDirectoryNodes(HTREEITEM hItem, CString &strPathName);
	UINT DeleteChildren(HTREEITEM hItem);
	HTREEITEM GetDriveNode(HTREEITEM hItem);
	BOOL IsMediaValid(CString &strPathName);
	CString GetPathFromItem(HTREEITEM hItem);
	void AddDummyNode(HTREEITEM hItem);
	BOOL AddDrives(CString strDrive, HTREEITEM hParent);
	BOOL HasSubdirectory(CString &strPathName);
	CImageList imgLarge, imgSmall, imgfileTree;
	CFileMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFileMgr)
	enum { IDD = IDD_LFILE_MGR };
	CStatic	m_statusFile;
	CTreeCtrl	m_fileTree;
	CSortListCtrl	m_fileList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFileMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickFileLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshFleList();
	afx_msg void OnItemexpandedFileTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangingFileTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDelFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEMGR_H__16CB4F4F_EFE1_4776_BC2B_80DC582CFA6E__INCLUDED_)
