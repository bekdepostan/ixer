// ThreadMgr.cpp : implementation file
//

#include "stdafx.h"
#include "ixer.h"
#include "common.h"
#include "ThreadMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString	g_strProcName;	//ѡ��item�Ľ�����
extern ULONG	g_eProc;
ULONG			g_dwTrdId=0;	//ѡ��item���߳�CID
/////////////////////////////////////////////////////////////////////////////
// CThreadMgr dialog


CThreadMgr::CThreadMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CThreadMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CThreadMgr)
	//}}AFX_DATA_INIT
}


void CThreadMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CThreadMgr)
	DDX_Control(pDX, IDC_THREAD_LISTS, m_threadList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CThreadMgr, CDialog)
	//{{AFX_MSG_MAP(CThreadMgr)
	ON_NOTIFY(NM_RCLICK, IDC_THREAD_LISTS, OnRclickThreadLists)
	ON_COMMAND(IDM_REFRESH_TRD_LIST, OnRefreshTrdList)
	ON_COMMAND(IDM_TRD_TERMINATOR, OnTrdTerminator)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CThreadMgr message handlers

BOOL CThreadMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	//������ͷ��Ϣ
	m_threadList.SetHeadings( _T("�߳�ID,  50;ETHREAD,  72;Teb,  72;���ȼ�,  48;�߳����,  72;�߳�ģ��,  85;�л�����,  60;�߳�״̬,  80"));
	//��������Ϣ
	m_threadList.LoadColumnInfo();

	//CListCtrl��չ��ʽ�ı�
	DWORD dwStyle = m_threadList.GetStyle();	//ȡ����ʽ
	dwStyle |=  LVS_EX_GRIDLINES				//������ʽ
		| LVS_EX_FULLROWSELECT				//����ѡ��
		| LVS_EX_HEADERDRAGDROP;			//�б�ͷ�����Ϸ�

	//������չ���
	m_threadList.SetExtendedStyle(dwStyle);

	//��ʼ����ʾ�߳�List
	ShowThreadList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CThreadMgr::ShowThreadList()
{
	unsigned int threadNums=0;					//�߳�����
	char		 proName_Nums[256]="";			//��ʾ�߳��б�Ľ�������ģ����
	
	//��ȡ��ǰModuleList���������ListCtrl������ģ����
	threadNums=GetCurrentThreadList(this);
	
	//��ʽ��ģ�鴰�ڱ����ַ���
	sprintf(proName_Nums, "[%s]-�����߳�(%d)", g_strProcName, threadNums);
	//����ģ�鴰�ڱ���
	m_threadList.GetParent()->SetWindowText(proName_Nums);
}

int CThreadMgr::GetCurrentThreadList(CThreadMgr *pDialog)
{
	ULONG	ifThreadNum=MAX_THREAD_NUM;
	ULONG	thrCount=0;
	PTHREAD_ITEM pThrInfo=NULL;
	ULONG	retBytes;
	BOOL	b_Ret;
	ULONG   trdCount=0;

	DWORD	dw=0;
	char	tid_c[16];				//�߳�tid
	char	eThread_c[16];			//ETHREAD
	char	teb_c[16];				//����teb
	char	selItemProId[16];		//����ID(ѡ��item���ַ�����ʽ"����ID")
	char	startAddress_c[16];		//��ַ
	char	state_c[16];			//״̬
	char	priority_c[16];			//���ȼ�
	char	contextSwitches_c[16];	//�л�����
	int		actualItemNum=0;		//�߳�����
	char	*tmpItemProId=new char[MAXPATHLEN];	//��ʱ����id
	char	strtrdFunMod[MAX_PATH*2]={0};	//�̺߳���ģ��
	ULONG	i=0, j=0;
	ULONG	n=0, m=0;

	CSortListCtrl *m_ThreadList;	//�̶߳Ի������߳��б�ؼ�

	//��ʼ������ָ�� ����Ҫ��������
	if(pDialog)
	{
		m_ThreadList = &pDialog->m_threadList;
		sprintf(selItemProId, "%d", g_dwPId);
	}
	else
		return 0;

	//���ListCtrl�б�
	m_ThreadList->DeleteAllItems();

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return 0;
	}
	//������������̽ṹ�ڴ�
	pThrInfo=(THREAD_ITEM *)malloc(sizeof(THREAD_ITEM)*ifThreadNum*2);
	if (!pThrInfo)
	{
		free(pThrInfo);
		return thrCount;
	}
	
	//�ڴ�����
	ZeroMemory(pThrInfo, sizeof(THREAD_ITEM)*ifThreadNum*2);
	//���߳�ETHREAD���������뻺����
	memcpy( g_ctrlBuff, &g_eProc, sizeof(ULONG) );
	//��ȡ����
	b_Ret=DeviceIoControl(g_hDriver, IOCTL_GET_THREAD_LIST, g_ctrlBuff, sizeof(ULONG), pThrInfo, sizeof(THREAD_ITEM)*ifThreadNum*2, &retBytes, NULL);
	if (b_Ret)
	{
		if (retBytes)
		{
			trdCount=retBytes/sizeof(THREAD_ITEM);
			for (i=0; i<trdCount; i++)
			{
				if (pThrInfo[i].eThread)
				{
					//�߳�tid�ַ���
					sprintf(tid_c, "%d", pThrInfo[i].tid);
					//�߳�ETHREAD�ַ���
					sprintf(eThread_c, "0x%08X", pThrInfo[i].eThread);
					//�߳�teb�ַ���
					sprintf(teb_c, "0x%08X", pThrInfo[i].teb);
					//���ȼ�
					sprintf(priority_c, "%d", pThrInfo[i].priority);
					//��ʼ��ַ
					sprintf(startAddress_c, "0x%08X", pThrInfo[i].win32ThreadStartAddr);
					//�л�����
					sprintf(contextSwitches_c, "%d", pThrInfo[i].contextSwitches);
					//��ȡ�߳�״̬
					GetThreadStateString(pThrInfo[i].state, state_c);
					//��ȡ�̺߳���ģ��
					GetFunctionBelongingModuleName(g_dwPId, pThrInfo[i].win32ThreadStartAddr, strtrdFunMod, sizeof(strtrdFunMod) );
					
					//�����б�ؼ�
					m_ThreadList->AddItem(
											NULL,				//ͼ��������
											tid_c,				//�߳�tid
											eThread_c,			//ETHREAD
											teb_c,				//Teb
											priority_c,			//���ȼ�
											startAddress_c,		//��ʼ��ַ
											strtrdFunMod,		//�߳�ģ��
											contextSwitches_c,	//�л�����
											state_c				//״̬
											);
				}
			}
		}
	}
	else
		showErrBox();
	
	free(pThrInfo);
	return trdCount;
}

void CThreadMgr::GetThreadStateString(UCHAR state, char *state_c)
{
	switch(state)
	{
	case StateInitialized:
		strcpy(state_c,"Initialized");
		break;
	case StateReady:
		strcpy(state_c,"Ready");
		break; 
	case StateRunning:
		strcpy(state_c,"����");
		break; 
	case StateStandby:
		strcpy(state_c,"Standby");
		break; 
	case StateTerminated:
		strcpy(state_c,"��ֹ");
		break; 
	case StateWait:
		strcpy(state_c,"�ȴ�");
		break; 
	case StateTransition:
		strcpy(state_c,"Transition");
		break; 
	case StateUnknown:
		strcpy(state_c,"Unknown");
		break; 
		
	default:
		strcpy(state_c,"Unknown");
		break;
	}
}

void CThreadMgr::OnRclickThreadLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		CString strTrdCID;
		int		selectedItem=0;			//ѡ��item��
		//��ȡ�����ListCtrl�ĵ��λ��
		CPoint pt;
		GetCursorPos(&pt);
		//���ؽ��̵��Ҽ��˵�
		CMenu menu;
		menu.LoadMenu(IDR_THREAD_RPOP);
		//��ȡ��һ���Ӳ˵�
		CMenu *pMenu = menu.GetSubMenu(0);
		//�����˵���׷��ѡ�е�Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		selectedItem = m_threadList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item

		// ѡ��item�ĵ�1�У����߳�CID
		strTrdCID = m_threadList.GetItemText(selectedItem, 0);
		// ת��������ѡ��item���߳�CID
		g_dwTrdId = atol(strTrdCID);					//ת���ַ���ΪLong�����浽ȫ�ֱ���g_dwTrdId
	}
	*pResult = 0;
}

void CThreadMgr::OnRefreshTrdList() 
{
	// TODO: Add your command handler code here
	ShowThreadList();
}

void CThreadMgr::OnTrdTerminator() 
{
	// TODO: Add your command handler code here
	//�����̵߳�CID������
	ULONG	dwTrdCId=g_dwTrdId;
	DWORD	dw;

	ZeroMemory(g_ctrlBuff, MAXBUFFERSIZE);
	*(ULONG *)g_ctrlBuff=dwTrdCId;

	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//�ѡ��߳�CID�����͵������豸
		DeviceIoControl(g_hDriver, IOCTL_THREAD_TERMINATOR, g_ctrlBuff, MAXBUFFERSIZE, g_ctrlBuff, MAXBUFFERSIZE, &dw, 0);
		ShowThreadList();
	}
}
