// IxAboutDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ixer.h"
#include "IxAboutDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIxAboutDlg dialog


CIxAboutDlg::CIxAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIxAboutDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIxAboutDlg)
	//}}AFX_DATA_INIT
}


void CIxAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIxAboutDlg)
	DDX_Control(pDX, IDC_EMIAL, m_Emial);
	DDX_Control(pDX, IDC_MICRO_BLOG, m_MicroLink);
	DDX_Control(pDX, IDC_BLOG, m_BlogLink);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIxAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CIxAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CIxAboutDlg m_IxAboutDlg;
/////////////////////////////////////////////////////////////////////////////
// CIxAboutDlg message handlers

BOOL CIxAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_Emial.SetWindowText("Email：Chinghoi@vip.qq.com");

	CString micro = "http://t.qq.com/ChinghoiTam";
	//设置超链接
	m_MicroLink.SetURL(micro);
	m_MicroLink.ModifyLinkStyle(m_MicroLink.StyleUnderline, FALSE);

	CString bolg = "http://blog.csdn.net/Chinghoi";
	//设置超链接
	m_BlogLink.SetURL(bolg);
	m_BlogLink.ModifyLinkStyle(m_BlogLink.StyleUnderline, FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
