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
extern int	iSeledItem;			//ѡ��item��
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

	//����ListHead
	m_netList.SetHeadings(_T("Э��,  50;����ip�˿�,  155;Զ��ip�˿�,  155;����״̬,  90;����ID,  55;����·��,  250"));
	m_netList.LoadColumnInfo();	//��������Ϣ
	//�޸�ListCtrl��չ��ʽ
	DWORD dwStyle = m_netList.GetStyle();		//ȡ��ListCtrl��ʽ
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//�޸�ListCtrl���|WS_VSCROLL|WS_HSCROLL
	m_netList.SetExtendedStyle(dwStyle);		//������
	//�б����ߵ�ͼ���б�  
	m_netList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNetWorkMgr::ShowNetWorkList()
{
	ULONG	numOflink=0;			//ssdt����������
	char	linkNum_s[128];		//��ʾssdt��������("�ַ�����ʽ")
	ULONG	numOfTcp=0;
	ULONG	numOfUdp=0;
	
	//��ȡ��ǰModuleList���������ListCtrl������ģ����
	numOflink=EnumNetWorkLink( &m_netList, &numOfTcp, &numOfUdp );
	
	//��ʽ��ģ�鴰�ڱ����ַ���
	sprintf( linkNum_s, "Tcp: %d, Udp: %d", numOfTcp, numOfUdp );	//, Raw��%d
	//״̬����
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
			sprintf(DesPort_c,"%s��%ld", plinfo[iCount].DesIP, plinfo[iCount].DesPort);
		
		sprintf(SrcPort_c,"%s��%ld", plinfo[iCount].SrcIP, plinfo[iCount].SrcPort);
		
		if (plinfo[iCount].ImagePath[0] != '\0')
			iconIndex = m_proMgrDlg.GetIconIndex( plinfo[iCount].ImagePath );

		m_netList->AddItem(	iconIndex,	//ͼ���
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
		//��ȡ�����ListCtrl�ĵ��λ��
		GetCursorPos(&pt);
		CMenu menu;
		//�����ļ����Ҽ��˵�
		menu.LoadMenu(IDR_NET_RPOP);
		//��ȡ��һ���Ӳ˵�
		CMenu *pMenu = menu.GetSubMenu(0);
		//�����˵���׷��ѡ�е�Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		iSeledItem = m_netList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item
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
	
	// ѡ��item�еĵ�7�У��������Ľ���ID
	strPid = m_netList.GetItemText(iSeledItem, 4);
	wStrPid = strPid.GetBuffer( strPid.GetLength() );
	// ת��������
	ULONG	dwProId = atol(wStrPid);
	
	ZeroMemory(g_ctrlBuff,MAXBUFFERSIZE);
	//���ͽ���ID������
	memcpy( g_ctrlBuff, &dwProId, sizeof(ULONG) );							//�ѡ�����ID�����������뻺����
	
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//�ѡ�����ID�����͵������豸
		DeviceIoControl(g_hDriver, IOCTL_PROCESS_KILLER, g_ctrlBuff, MAXBUFFERSIZE, g_ctrlBuff, MAXBUFFERSIZE, &dw, 0);
		ShowNetWorkList();
	}
}
