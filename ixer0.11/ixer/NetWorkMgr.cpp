// NetWorkMgr.cpp : implementation file
//

#include "stdafx.h"
#include "processmgr.h"
#include "OS.h"
#include "Memory.h"
#include "ixer.h"
#include "Net.h"
#include "NetWorkMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CProcessMgr m_proMgrDlg;
extern int	iSeledItem;			//选中item行
/////////////////////////////////////////////////////////////////////////////
// CNetWorkMgr dialog


CNetWorkMgr::CNetWorkMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CNetWorkMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNetWorkMgr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CNetWorkMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNetWorkMgr)
	DDX_Control(pDX, IDC_STATIC_NETWORK, m_stusNet);
	DDX_Control(pDX, IDC_NETWORK_LISTS, m_netList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNetWorkMgr, CDialog)
	//{{AFX_MSG_MAP(CNetWorkMgr)
	ON_NOTIFY(NM_RCLICK, IDC_NETWORK_LISTS, OnRclickNetworkLists)
	ON_COMMAND(IDM_REFRESH_NET_LIST, OnRefreshNetList)
	ON_COMMAND(IDM_CONNECT_PRO_KILLER, OnConnectProKiller)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CNetWorkMgr m_netMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CNetWorkMgr message handlers

BOOL CNetWorkMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//插入ListHead
	m_netList.SetHeadings(_T("协议,  50;本地ip端口,  155;远程ip端口,  155;连接状态,  90;进程ID,  55;进程路径,  250"));
	m_netList.LoadColumnInfo();	//加载列信息
	//修改ListCtrl扩展样式
	DWORD dwStyle = m_netList.GetStyle();		//取得ListCtrl样式
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//修改ListCtrl风格，|WS_VSCROLL|WS_HSCROLL
	m_netList.SetExtendedStyle(dwStyle);		//重设风格
	//列表框左边的图标列表  
	m_netList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNetWorkMgr::ShowNetWorkList()
{
	ULONG	numOflink=0;			//ssdt服务函数总数
	char	linkNum_s[128];		//显示ssdt函数总数("字符串形式")
	ULONG	numOfTcp=0;
	ULONG	numOfUdp=0;
	
	//获取当前ModuleList并插入进程ListCtrl，返回模块数
	numOflink=EnumNetWorkLink( &m_netList, &numOfTcp, &numOfUdp );
	
	//格式化模块窗口标题字符串
	sprintf( linkNum_s, "Tcp: %d, Udp: %d", numOfTcp, numOfUdp );	//, Raw：%d
	//状态回显
	m_netMgrDlg.SetDlgItemText(IDC_STATIC_NETWORK, linkNum_s);
}

int CNetWorkMgr::EnumNetWorkLink( CSortListCtrl *m_netList, PDWORD numOfTcp, PDWORD numOfUdp )
{
	PNETLINKINFO	plinfo = NULL;
	int	iconIndex=NULL;
	
	m_netList->DeleteAllItems();
	
	WORD	Version = GetCurrentOSVersion();
	
	if(	Version != VER_VISTA11&&
		Version != VER_VISTASP1&&
		Version != VER_VISTAULT		)
	{
		plinfo = KernelEnumPortInfo( numOfTcp, numOfUdp );
		if(!plinfo)
		{
			return 0;
		}
	}
	else
		return 0;
	
	if(!plinfo)
	{
		return 0;
	}
	
	for(DWORD iCount=0; iCount< *numOfTcp + *numOfUdp; iCount++)
	{		
		char Pid_c[16];
		char SrcPort_c[128];
		char DesPort_c[128];
		sprintf(Pid_c,"%ld", plinfo[iCount].Pid);
		if ( strcmp(plinfo[iCount].Type, "UDP") == 0  &&  plinfo[iCount].DesPort==0 )
			sprintf(DesPort_c,"%s : %s", plinfo[iCount].DesIP, "*");
		else
			sprintf(DesPort_c,"%s：%ld", plinfo[iCount].DesIP, plinfo[iCount].DesPort);
		
		sprintf(SrcPort_c,"%s：%ld", plinfo[iCount].SrcIP, plinfo[iCount].SrcPort);
		
		if (plinfo[iCount].ImagePath[0] != '\0')
			iconIndex = m_proMgrDlg.GetIconIndex( plinfo[iCount].ImagePath );

		m_netList->AddItem(	iconIndex,	//图标号
							plinfo[iCount].Type,
							SrcPort_c,
							DesPort_c,
							plinfo[iCount].State,
							Pid_c,
							plinfo[iCount].ImagePath );
	}
	
	FreeHeapMem(plinfo);
	
	return *numOfTcp + *numOfUdp;
}

void CNetWorkMgr::OnRclickNetworkLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		CPoint pt;
		//获取鼠标在ListCtrl的点击位置
		GetCursorPos(&pt);
		CMenu menu;
		//加载文件的右键菜单
		menu.LoadMenu(IDR_NET_RPOP);
		//获取第一个子菜单
		CMenu *pMenu = menu.GetSubMenu(0);
		//弹出菜单并追踪选中的Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		iSeledItem = m_netList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1从第1个item开始搜索行,匹配选中item
	}
	*pResult = 0;
}

void CNetWorkMgr::OnRefreshNetList() 
{
	// TODO: Add your command handler code here
	ShowNetWorkList();
}

void CNetWorkMgr::OnConnectProKiller() 
{
	// TODO: Add your command handler code here
	CString strPid;
	CHAR	*wStrPid={'\0'};
	DWORD	dw;
	
	// 选中item行的第7列，即联网的进程ID
	strPid = m_netList.GetItemText(iSeledItem, 4);
	wStrPid = strPid.GetBuffer( strPid.GetLength() );
	// 转换并保存
	ULONG	dwProId = atol(wStrPid);
	
	ZeroMemory(g_ctrlBuff,MAXBUFFERSIZE);
	//发送进程ID给驱动
	memcpy( g_ctrlBuff, &dwProId, sizeof(ULONG) );							//把“进程ID”拷贝到输入缓冲区
	
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//把“进程ID”发送到驱动设备
		DeviceIoControl(g_hDriver, IOCTL_PROCESS_KILLER, g_ctrlBuff, MAXBUFFERSIZE, g_ctrlBuff, MAXBUFFERSIZE, &dw, 0);
		ShowNetWorkList();
	}
}
