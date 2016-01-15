#if !defined(AFX_SHADOWSSDTMGR_H__F4860922_3111_4E53_8AA9_AA6D10531D98__INCLUDED_)
#define AFX_SHADOWSSDTMGR_H__F4860922_3111_4E53_8AA9_AA6D10531D98__INCLUDED_

#include "rootkit.h"	// Added by ClassView
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShadowSSDTMgr.h : header file
//

// extern "C" {
// 	PLOADED_IMAGE
// 				IMAGEAPI
// 				ImageLoad(
// 							PCSTR DllName,
// 							PCSTR DllPath );
// 	BOOL 
// 		IMAGEAPI 
// 		ImageUnload( PLOADED_IMAGE LoadedImage );
// }

//
//���ڴ�ŵõ���ShadowSSDT�ĺ����ͺ�����
//
typedef struct _SHADOWFUNC
{
	CHAR		funcName[MAXNAMELEN];	//������
	ULONG		nativeAddr;				//ԭʼ��ַ
}SHADOWFUNC, *PSHADOWFUNC;
/////////////////////////////////////////////////////////////////////////////
// CShadowSSDTMgr dialog

class CShadowSSDTMgr : public CDialog
{
// Construction
public:
	int shadowSeled;
	BOOL	GetShadowSSDTNativeAddrBySymbol();
	DWORD	numOfNativeAddr;
//	PSHADOW_INFO	pShadowInfo;

	int GetCurrentShadowSSDTList(CSortListCtrl *m_shadowList);
	void ShowShadowSSDTList();
	CShadowSSDTMgr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CShadowSSDTMgr)
	enum { IDD = IDD_LSHADOW_MGR };
	CStatic	m_statusShadow;
	CSortListCtrl	m_shadowList;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShadowSSDTMgr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CShadowSSDTMgr)
	virtual BOOL OnInitDialog();
	afx_msg void OnRclickShadowssdtLists(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRefreshShadowList();
	afx_msg void OnShadowRestore();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHADOWSSDTMGR_H__F4860922_3111_4E53_8AA9_AA6D10531D98__INCLUDED_)
