// SSDTMgr.cpp : implementation file
//

#include "stdafx.h"
#include "Module.h"
#include "Memory.h"
#include "ixer.h"
#include "SSDTMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSSDTMgr dialog


CSSDTMgr::CSSDTMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CSSDTMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSSDTMgr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSSDTMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSSDTMgr)
	DDX_Control(pDX, IDC_STATIC_SSDT, m_statusSsdt);
	DDX_Control(pDX, IDC_SSDT_LISTS, m_ssdtList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSSDTMgr, CDialog)
	//{{AFX_MSG_MAP(CSSDTMgr)
	ON_NOTIFY(NM_RCLICK, IDC_SSDT_LISTS, OnRclickSsdtLists)
	ON_COMMAND(IDM_REFRESH_SSDT_LIST, OnRefreshSsdtList)
	ON_COMMAND(IDM_SSDT_RESTORE, OnSsdtRestore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CSSDTMgr m_ssdtMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CSSDTMgr message handlers

BOOL CSSDTMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//插入ListHead
	m_ssdtList.SetHeadings(_T("序号,  50;函数名,  150;当前地址,  90;钩子类型,  85;原始函数地址,  90;当前函数所在模块,  300"));
	m_ssdtList.LoadColumnInfo();	//加载列信息
	//修改ListCtrl扩展样式
	DWORD dwStyle = m_ssdtList.GetStyle();		//取得ListCtrl样式
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//修改ListCtrl风格，|WS_VSCROLL|WS_HSCROLL
	m_ssdtList.SetExtendedStyle(dwStyle);		//重设风格
	// TODO: Add extra initialization here
//	m_ssdtList.m_GuiListType=GUILT_SSDTList;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSSDTMgr::ShowSSDTList()
{
	GetCurrentSSDTList(&m_ssdtList);
}

int CSSDTMgr::GetCurrentSSDTList(CSortListCtrl *m_ssdtList)
{
	BOOL		b_Ret;
	ULONG		retBytes;
	ULONG		numOfSrv=0;
	PSSDT_INFO	pSsdtInfo;
	CHAR		Index[6] = "\0";
	CHAR		NewAddr[10] = "\0";
	CHAR		OldAddr[10] = "\0";
	CHAR		hookstype[15] = "\0";
	char		fixSsdtModPath_c[MAXPATHLEN];
	char		ssdtNum_s[128];
	int			hooked=0;

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
		return 0;

	//获取ssdt个数
	b_Ret = DeviceIoControl(g_hDriver,
							(DWORD)IOCTL_GET_SSDT_INFO,
							NULL,
							0,
							&numOfSrv,
							sizeof(DWORD),
							&retBytes,
							NULL);
	if (!b_Ret)
		return 0;

	pSsdtInfo = (PSSDT_INFO)malloc( numOfSrv * sizeof(SSDT_INFO) );
	if ( pSsdtInfo == NULL )
		return 0;

	ZeroMemory( pSsdtInfo, numOfSrv * sizeof(SSDT_INFO) );
		
	//请求ssdt信息数据
	b_Ret = DeviceIoControl(g_hDriver,
							(DWORD)IOCTL_GET_SSDT_INFO,
							NULL,
							0,
							pSsdtInfo,
							numOfSrv*sizeof(SSDT_INFO),
							&retBytes,
							NULL);
		if (!b_Ret)
			return 0;
		
		//清空ListCtrl列表
		m_ssdtList->DeleteAllItems();
		
		for (ULONG i = 0; i < numOfSrv; i++)
		{
			sprintf( Index, "%d", i );
			sprintf( NewAddr, "0x%08X", pSsdtInfo[i].currentAddr );
			sprintf( OldAddr, "0x%08X", pSsdtInfo[i].nativeAddr );

			//当前地址 不等于 原始地址，则为hooked！
			if (pSsdtInfo[i].currentAddr != pSsdtInfo[i].nativeAddr)
			{
				strcpy(hookstype, " ssdt hook");
				hooked++;
			}
			else
				strcpy(hookstype, "     -     ");

			//修正路径
			TranslateFilePathName(pSsdtInfo[i].imagePath, fixSsdtModPath_c);

			m_ssdtList->AddItem(
								NULL,						//图标index
								Index,						//序号
								pSsdtInfo[i].funName,		//函数名
								NewAddr,					//当前地址
								hookstype,					//钩子类型
								OldAddr,					//原始地址
								fixSsdtModPath_c);	//当前ssdt函数地址所在内核模块路径
		}
	
	//格式化模块窗口标题字符串
	sprintf(ssdtNum_s, "SSDT函数： %d,  挂钩函数： %d", numOfSrv, hooked);
	//状态回显
	m_ssdtMgrDlg.SetDlgItemText(IDC_STATIC_SSDT, ssdtNum_s);
	//格式化字符串
	free( pSsdtInfo );
	return numOfSrv;
}

void CSSDTMgr::OnRclickSsdtLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		//选中item行
		ssdtSeled=0;
		CPoint pt;
		//获取鼠标在ListCtrl的点击位置
		GetCursorPos(&pt);
		CMenu menu;
		//加载进程的右键菜单
		menu.LoadMenu(IDR_SSDT_RPOP);
		//获取第一个子菜单
		CMenu *pMenu = menu.GetSubMenu(0);
		//弹出菜单并追踪选中的Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		ssdtSeled = m_ssdtList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1从第1个item开始搜索行,匹配选中item
	}
	*pResult = 0;
}

void CSSDTMgr::OnRefreshSsdtList() 
{
	// TODO: Add your command handler code here
	ShowSSDTList();
}

void CSSDTMgr::OnSsdtRestore() 
{
	// TODO: Add your command handler code here
	//发送ssdt服务函数索引、原始地址给驱动
	CString	strIndex;
	ULONG	dwSrvIndex;			//index
	ULONG	dwCurrentAddr;		//当前地址
	ULONG	dwOriginAddr;		//原始地址

	CHAR	*wChr={'\0'};
	CString strCurrentAddr;
	CString strOriginAddr;

	// 选中item的第1列，即ssdt服务函数索引
	strIndex = m_ssdtList.GetItemText(ssdtSeled, 0);
	wChr = strIndex.GetBuffer( strIndex.GetLength() );
	//转换并保存
	dwSrvIndex = atol(wChr);
	
	// 第3列，"ssdt当前地址"
	strCurrentAddr = m_ssdtList.GetItemText(ssdtSeled, 2);
	// 转换并保存
	dwCurrentAddr = strtoul( (LPCTSTR)strCurrentAddr, '\0', 16);
	
	// 第5列，"ssdt原始地址"
	strOriginAddr = m_ssdtList.GetItemText(ssdtSeled, 4);
	// 转换并保存
	dwOriginAddr = strtoul( (LPCTSTR)strOriginAddr, '\0', 16);

	SSDTINFO	sinfo = {0, 0};
	unsigned long int retBytes;

	//当前地址==原始地址，则直接返回
	if (dwCurrentAddr == dwOriginAddr)
		return;

	//驱动设备句柄无效也返回
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		sinfo.index	  = dwSrvIndex;		//index
		sinfo.funAddr = dwOriginAddr;	//原始地址

		//把ssdt“服务函数索引、原始地址”发送到驱动设备
		DeviceIoControl(g_hDriver,
						(DWORD)IOCTL_RESTORE_SSDT,
						&sinfo,
						sizeof(SSDTINFO),
						NULL,
						0,
						&retBytes,
						NULL);
		ShowSSDTList();
	}
}
