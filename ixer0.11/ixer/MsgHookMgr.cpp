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
extern int	iSeledItem;			//ѡ��item��
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

	//����ListHead
	m_msghookList.SetHeadings(_T("���,  70;����,  95;������ַ,  90;ģ����,  100;�߳�ID,  50;����ID,  50;����·��, 300"));
	m_msghookList.LoadColumnInfo();	//��������Ϣ
	//�޸�ListCtrl��չ��ʽ
	DWORD dwStyle = m_msghookList.GetStyle();		//ȡ��ListCtrl��ʽ
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//�޸�ListCtrl���|WS_VSCROLL|WS_HSCROLL
	m_msghookList.SetExtendedStyle(dwStyle);		//������
	//�б����ߵ�ͼ���б�  
	m_msghookList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMsgHookMgr::ShowMsgHookList()
{
	unsigned int	numOfMsgHook=0;		//ssdt����������
	char	msghookNum_s[256]="";		//��ʾssdt��������("�ַ�����ʽ")
	
	//��ȡ��ǰModuleList���������ListCtrl������ģ����
	numOfMsgHook=EnumMessageHooks(&m_msghookList);
	
	//��ʽ��ģ�鴰�ڱ����ַ���
	sprintf(msghookNum_s, "��Ϣ���ӣ�%d", numOfMsgHook);	//���Ӻ�����%d
	//״̬����
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
	//���ListCtrl�б�
	m_msghookList->DeleteAllItems();

	bool bBreakWhile;
	while ( HookInfo[i].Handle != 0 )
	{
		bBreakWhile = false;
		for (ULONG j=0; j< i; j++)
		{
			//�����Ѵ�����ͬ�ľ��
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

		//��ձ�������¼
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
			strcpy(Type,"δ֪");
			break;
		}
		sprintf( Handle, "0x%08X", HookInfo[k].Handle );
		sprintf( Proc, "0x%08X", HookInfo[k].FuncOffset );
		sprintf( Pid, "%d", HookInfo[k].pid );
		sprintf( Tid, "%d", HookInfo[k].tid );

		//��ȡ������ַ���ڵ�ģ����
		GetFunctionBelongingModuleName( HookInfo[k].pid, (PVOID)HookInfo[k].FuncOffset, ModuleName, 255 );
		//��ģ����Ϊ�գ�����ģ�����ڽ�����
		if (ModuleName[0] == '\0')
			strcpy( ModuleName, HookInfo[k].ProcessName );

		//����ListCtrl
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
		//��ȡ�����ListCtrl�ĵ��λ��
		GetCursorPos(&pt);
		CMenu menu;
		//�����ļ����Ҽ��˵�
		menu.LoadMenu(IDR_MSGHOOK_RPOP);
		//��ȡ��һ���Ӳ˵�
		CMenu *pMenu = menu.GetSubMenu(0);
		//�����˵���׷��ѡ�е�Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		iSeledItem = m_msghookList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item
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

	// ѡ��item�ĵ�1�У����ļ���
	m_msghookList.GetItemText(iSeledItem, 0, strhHk, 16);
	hHk = strtoul(strhHk + 2, '\0', 16);

	//ж����Ϣ����
	UnhookWindowsHookEx( (HHOOK)hHk );
	//ˢ��
	ShowMsgHookList();
}
