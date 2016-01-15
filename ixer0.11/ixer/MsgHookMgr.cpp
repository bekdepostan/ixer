// MsgHookMgr.cpp : implementation file
//

#include "stdafx.h"
#include "ixer.h"
#include "common.h"
#include "processmgr.h"
#include "MsghookMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CProcessMgr m_proMgrDlg;
extern int	iSeledItem;			//选中item行
/////////////////////////////////////////////////////////////////////////////
// CMsgHookMgr dialog


CMsgHookMgr::CMsgHookMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CMsgHookMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMsgHookMgr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMsgHookMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgHookMgr)
	DDX_Control(pDX, IDC_STATIC_MSGHOOK, m_statusMsghook);
	DDX_Control(pDX, IDC_MSG_HOOK_LISTS, m_msghookList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMsgHookMgr, CDialog)
	//{{AFX_MSG_MAP(CMsgHookMgr)
	ON_NOTIFY(NM_RCLICK, IDC_MSG_HOOK_LISTS, OnRclickMsgHookLists)
	ON_COMMAND(IDM_MSG_UNHOOK, OnMsgUnhook)
	ON_COMMAND(IDM_REFRESH_MSGHOOK_LIST, OnRefreshMsghookList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CMsgHookMgr m_msghookMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CMsgHookMgr message handlers

BOOL CMsgHookMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//插入ListHead
	m_msghookList.SetHeadings(_T("句柄,  70;类型,  95;函数地址,  90;模块名,  100;线程ID,  50;进程ID,  50;进程路径, 300"));
	m_msghookList.LoadColumnInfo();	//加载列信息
	//修改ListCtrl扩展样式
	DWORD dwStyle = m_msghookList.GetStyle();		//取得ListCtrl样式
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//修改ListCtrl风格，|WS_VSCROLL|WS_HSCROLL
	m_msghookList.SetExtendedStyle(dwStyle);		//重设风格
	//列表框左边的图标列表  
	m_msghookList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMsgHookMgr::ShowMsgHookList()
{
	unsigned int	numOfMsgHook=0;		//ssdt服务函数总数
	char	msghookNum_s[256]="";		//显示ssdt函数总数("字符串形式")
	
	//获取当前ModuleList并插入进程ListCtrl，返回模块数
	numOfMsgHook=EnumMessageHooks(&m_msghookList);
	
	//格式化模块窗口标题字符串
	sprintf(msghookNum_s, "消息钩子：%d", numOfMsgHook);	//钩子函数：%d
	//状态回显
	m_msghookMgrDlg.SetDlgItemText(IDC_STATIC_MSGHOOK, msghookNum_s);
}

int CMsgHookMgr::EnumMessageHooks(CSortListCtrl *m_msghookList)
{
	BOOL		success;
	DWORD		bytesRead;
	PHOOK_INFO	HookInfo;
	CHAR		Handle[10] = "\0";
	CHAR		Type[32] = "\0";
	CHAR		Proc[10] = "\0";
	CHAR		Pid[10] = "\0";
	CHAR		Tid[10] = "\0";
	CHAR		ModuleName[256] = "\0";

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
		return 0;

	HookInfo = (PHOOK_INFO)malloc( MAX_HOOK * sizeof(HOOK_INFO) );
	if (HookInfo == NULL)
	{
		return 0;
	}

	memset( HookInfo, 0, MAX_HOOK * sizeof(HOOK_INFO) );

	success = DeviceIoControl(
								g_hDriver, 
								IOCTL_GET_MSGHOOK_INFO,
								NULL,
								0,
								HookInfo,
								MAX_HOOK * sizeof(HOOK_INFO),
								&bytesRead,
								NULL );
	if (!success)
	{
		return 0;
	}

	DWORD i = 0;
	ULONG k = 0;
	ULONG msghkCount=0;
	//清空ListCtrl列表
	m_msghookList->DeleteAllItems();

	bool bBreakWhile;
	while ( HookInfo[i].Handle != 0 )
	{
		bBreakWhile = false;
		for (ULONG j=0; j< i; j++)
		{
			//过滤已存在相同的句柄
			if (HookInfo[j].Handle == HookInfo[i].Handle)
			{
				bBreakWhile=true;
				break;
			}
		}

		k = i;
		i++;
		if (bBreakWhile)
		{
			continue;
		}

		//清空变量并记录
		memset( Type, 0, 32 );
		memset( Handle, 0, 10 );
		memset( Proc, 0, 10 );
		memset( Pid, 0, 10 );
		memset( Tid, 0, 10 );
		memset( ModuleName, 0, 255 );
		
		switch ( HookInfo[k].iHook )
		{
		case -1:
			strcpy( Type, "WH_MSGFILTER" );
			break;
		case 0:
			strcpy( Type, "WH_JOURNALRECORD" );
			break;
		case 1:
			strcpy( Type, "WH_JOURNALPLAYBACK" );
			break;
		case 2:
			strcpy( Type, "WH_KEYBOARD" );
			break;
		case 3:
			strcpy( Type, "WH_GETMESSAGE" );
			break;
		case 4:
			strcpy( Type, "WH_CALLWNDPROC" );
			break;
		case 5:
			strcpy( Type, "WH_CBT" );
			break;
		case 6:
			strcpy( Type, "WH_SYSMSGFILTER" );
			break;
		case 7:
			strcpy( Type, "WH_MOUSE" );
			break;
		case 8:
			strcpy( Type, "WH_HARDWARE" );
			break;
		case 9:
			strcpy( Type, "WH_DEBUG" );
			break;
		case 10:
			strcpy( Type, "WH_SHELL" );
			break;
		case 11:
			strcpy( Type, "WH_FOREGROUNDIDLE" );
			break;
		case 12:
			strcpy( Type, "WH_CALLWNDPROCRET" );
			break;
		case 13:
			strcpy( Type, "WH_KEYBOARD_LL" );
			break;
		case 14:
			strcpy( Type, "WH_MOUSE_LL" );
			break;

		default:
			strcpy(Type,"未知");
			break;
		}
		sprintf( Handle, "0x%08X", HookInfo[k].Handle );
		sprintf( Proc, "0x%08X", HookInfo[k].FuncOffset );
		sprintf( Pid, "%d", HookInfo[k].pid );
		sprintf( Tid, "%d", HookInfo[k].tid );

		//获取函数地址所在的模块名
		GetFunctionBelongingModuleName( HookInfo[k].pid, (PVOID)HookInfo[k].FuncOffset, ModuleName, 255 );
		//若模块名为空，则用模块所在进程名
		if (ModuleName[0] == '\0')
			strcpy( ModuleName, HookInfo[k].ProcessName );

		//插入ListCtrl
		m_msghookList->AddItem( NULL,
								Handle,
								Type,
								Proc,
								ModuleName,
								Tid,
								Pid,
								HookInfo[k].ProcessPath );
		msghkCount++;
	}

	free( HookInfo );
	return msghkCount;
}

void CMsgHookMgr::OnRclickMsgHookLists(NMHDR* pNMHDR, LRESULT* pResult) 
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
		menu.LoadMenu(IDR_MSGHOOK_RPOP);
		//获取第一个子菜单
		CMenu *pMenu = menu.GetSubMenu(0);
		//弹出菜单并追踪选中的Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		iSeledItem = m_msghookList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1从第1个item开始搜索行,匹配选中item
	}
	*pResult = 0;
}

void CMsgHookMgr::OnRefreshMsghookList() 
{
	// TODO: Add your command handler code here
	ShowMsgHookList();
}

void CMsgHookMgr::OnMsgUnhook() 
{
	// TODO: Add your command handler code here
	char	strhHk[16]={0};
	ULONG	hHk;

	// 选中item的第1列，即文件名
	m_msghookList.GetItemText(iSeledItem, 0, strhHk, 16);
	hHk = strtoul(strhHk + 2, '\0', 16);

	//卸载消息钩子
	UnhookWindowsHookEx( (HHOOK)hHk );
	//刷新
	ShowMsgHookList();
}
