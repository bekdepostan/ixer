// AddNewKeyItem.cpp : implementation file
//

#include "stdafx.h"
#include "Reg.h"
#include "RegMgr.h"
#include "ixer.h"
#include "AddNewKeyItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern REGMGR	regMgr;
extern CRegMgr	m_regMgrDlg;
extern char currUserSid[256];
/////////////////////////////////////////////////////////////////////////////
// CAddNewKeyItem dialog


CAddNewKeyItem::CAddNewKeyItem(CWnd* pParent /*=NULL*/)
	: CDialog(CAddNewKeyItem::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAddNewKeyItem)
	m_newItem = _T("");
	//}}AFX_DATA_INIT
}


void CAddNewKeyItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddNewKeyItem)
	DDX_Text(pDX, IDC_EDIT_ITEMNAME, m_newItem);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAddNewKeyItem, CDialog)
	//{{AFX_MSG_MAP(CAddNewKeyItem)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddNewKeyItem message handlers

BOOL CAddNewKeyItem::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	if (m_bWinText)
	{
		this->SetWindowText("添加值");
		SetDlgItemText(IDC_STATIC_NAME, "值名称：");
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAddNewKeyItem::OnOK() 
{
	// TODO: Add extra validation here
	UpdateData(TRUE);

	CDialog::OnOK();
}
