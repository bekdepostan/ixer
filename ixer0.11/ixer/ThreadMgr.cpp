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

extern CString	g_strProcName;	//选中item的进程名
extern ULONG	g_eProc;
ULONG			g_dwTrdId=0;	//选中item的线程CID
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
	//设置列头信息
	m_threadList.SetHeadings( _T("线程ID,  50;ETHREAD,  72;Teb,  72;优先级,  48;线程入口,  72;线程模块,  85;切换次数,  60;线程状态,  80"));
	//加载列信息
	m_threadList.LoadColumnInfo();

	//CListCtrl扩展样式改变
	DWORD dwStyle = m_threadList.GetStyle();	//取得样式
	dwStyle |=  LVS_EX_GRIDLINES				//网格样式
		| LVS_EX_FULLROWSELECT				//整行选中
		| LVS_EX_HEADERDRAGDROP;			//列表头可以拖放

	//设置扩展风格
	m_threadList.SetExtendedStyle(dwStyle);

	//初始化显示线程List
	ShowThreadList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CThreadMgr::ShowThreadList()
{
	unsigned int threadNums=0;					//线程总数
	char		 proName_Nums[256]="";			//显示线程列表的进程名、模块数
	
	//获取当前ModuleList并插入进程ListCtrl，返回模块数
	threadNums=GetCurrentThreadList(this);
	
	//格式化模块窗口标题字符串
	sprintf(proName_Nums, "[%s]-进程线程(%d)", g_strProcName, threadNums);
	//设置模块窗口标题
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
	char	tid_c[16];				//线程tid
	char	eThread_c[16];			//ETHREAD
	char	teb_c[16];				//进程teb
	char	selItemProId[16];		//进程ID(选中item的字符串形式"进程ID")
	char	startAddress_c[16];		//基址
	char	state_c[16];			//状态
	char	priority_c[16];			//优先级
	char	contextSwitches_c[16];	//切换次数
	int		actualItemNum=0;		//线程总数
	char	*tmpItemProId=new char[MAXPATHLEN];	//临时进程id
	char	strtrdFunMod[MAX_PATH*2]={0};	//线程函数模块
	ULONG	i=0, j=0;
	ULONG	n=0, m=0;

	CSortListCtrl *m_ThreadList;	//线程对话框中线程列表控件

	//初始化几个指针 后面要操作他们
	if(pDialog)
	{
		m_ThreadList = &pDialog->m_threadList;
		sprintf(selItemProId, "%d", g_dwPId);
	}
	else
		return 0;

	//清空ListCtrl列表
	m_ThreadList->DeleteAllItems();

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return 0;
	}
	//按个数申请进程结构内存
	pThrInfo=(THREAD_ITEM *)malloc(sizeof(THREAD_ITEM)*ifThreadNum*2);
	if (!pThrInfo)
	{
		free(pThrInfo);
		return thrCount;
	}
	
	//内存清零
	ZeroMemory(pThrInfo, sizeof(THREAD_ITEM)*ifThreadNum*2);
	//把线程ETHREAD拷贝到输入缓冲区
	memcpy( g_ctrlBuff, &g_eProc, sizeof(ULONG) );
	//读取数据
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
					//线程tid字符串
					sprintf(tid_c, "%d", pThrInfo[i].tid);
					//线程ETHREAD字符串
					sprintf(eThread_c, "0x%08X", pThrInfo[i].eThread);
					//线程teb字符串
					sprintf(teb_c, "0x%08X", pThrInfo[i].teb);
					//优先级
					sprintf(priority_c, "%d", pThrInfo[i].priority);
					//起始地址
					sprintf(startAddress_c, "0x%08X", pThrInfo[i].win32ThreadStartAddr);
					//切换次数
					sprintf(contextSwitches_c, "%d", pThrInfo[i].contextSwitches);
					//获取线程状态
					GetThreadStateString(pThrInfo[i].state, state_c);
					//获取线程函数模块
					GetFunctionBelongingModuleName(g_dwPId, pThrInfo[i].win32ThreadStartAddr, strtrdFunMod, sizeof(strtrdFunMod) );
					
					//加入列表控件
					m_ThreadList->AddItem(
											NULL,				//图标索引号
											tid_c,				//线程tid
											eThread_c,			//ETHREAD
											teb_c,				//Teb
											priority_c,			//优先级
											startAddress_c,		//起始地址
											strtrdFunMod,		//线程模块
											contextSwitches_c,	//切换次数
											state_c				//状态
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
		strcpy(state_c,"运行");
		break; 
	case StateStandby:
		strcpy(state_c,"Standby");
		break; 
	case StateTerminated:
		strcpy(state_c,"终止");
		break; 
	case StateWait:
		strcpy(state_c,"等待");
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
		int		selectedItem=0;			//选中item行
		//获取鼠标在ListCtrl的点击位置
		CPoint pt;
		GetCursorPos(&pt);
		//加载进程的右键菜单
		CMenu menu;
		menu.LoadMenu(IDR_THREAD_RPOP);
		//获取第一个子菜单
		CMenu *pMenu = menu.GetSubMenu(0);
		//弹出菜单并追踪选中的Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		selectedItem = m_threadList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1从第1个item开始搜索行,匹配选中item

		// 选中item的第1列，即线程CID
		strTrdCID = m_threadList.GetItemText(selectedItem, 0);
		// 转换并保存选中item的线程CID
		g_dwTrdId = atol(strTrdCID);					//转换字符串为Long，保存到全局变量g_dwTrdId
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
	//发送线程的CID给驱动
	ULONG	dwTrdCId=g_dwTrdId;
	DWORD	dw;

	ZeroMemory(g_ctrlBuff, MAXBUFFERSIZE);
	*(ULONG *)g_ctrlBuff=dwTrdCId;

	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//把“线程CID”发送到驱动设备
		DeviceIoControl(g_hDriver, IOCTL_THREAD_TERMINATOR, g_ctrlBuff, MAXBUFFERSIZE, g_ctrlBuff, MAXBUFFERSIZE, &dw, 0);
		ShowThreadList();
	}
}
