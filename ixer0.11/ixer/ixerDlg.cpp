// ixerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "processMgr.h"
#include "DrvModMgr.h"
#include "FileMgr.h"
#include "SSDTMgr.h"
#include "ShadowSSDTMgr.h"
#include "MsgHookMgr.h"
#include "API.h"
#include "WindowsVersion.h"
#include "NetWorkMgr.h"
#include "RegMgr.h"
#include "common.h"
#include "IxAboutDlg.h"
#include "ixer.h"
#include "ixerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define DRVNAME			"ix"

extern CProcessMgr		m_proMgrDlg;
extern CDrvModMgr		m_DrvMgrDlg;
extern CFileMgr			m_fileMgrDlg;
extern CSSDTMgr			m_ssdtMgrDlg;
extern CShadowSSDTMgr	m_shadowMgrDlg;
extern CMsgHookMgr		m_msghookMgrDlg;
extern CNetWorkMgr		m_netMgrDlg;
extern CRegMgr			m_regMgrDlg;
extern CIxAboutDlg		m_IxAboutDlg;


//TAB控件
char	*tabMainView[] = {"进程", "驱动模块", "SSDT", "ShadowSSDT", "消息钩子", "注冊表", "文件",
									"网絡", "关于"};
int		tabMainViewCount = ( sizeof(tabMainView)/4 );
/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CixerDlg dialog

CixerDlg::CixerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CixerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CixerDlg)
		// NOTE: the ClassWizard will add member initialization here
//	g_hDriver=INVALID_HANDLE_VALUE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CixerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CixerDlg)
	DDX_Control(pDX, IDC_TAB_MAIN_VIEW, m_tab);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CixerDlg, CDialog)
	//{{AFX_MSG_MAP(CixerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN_VIEW, OnSelchangeTabMainView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CixerDlg message handlers


BOOL CixerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	//插入主对话框视图TAB标签页
	for (int iTab=0; iTab<tabMainViewCount; iTab++)
	{
		m_tab.InsertItem(iTab, tabMainView[iTab]);
	}

	//创建并绑定
	CWnd *m_WndMainTab = GetDlgItem(IDC_TAB_MAIN_VIEW);
	//关联对话框，并且将IDC_TAB1控件设为父窗口
	m_proMgrDlg.Create(IDD_LPROC_MGR, m_WndMainTab);
	m_DrvMgrDlg.Create(IDD_LDRV_MGR, m_WndMainTab);
	m_ssdtMgrDlg.Create(IDD_LSSDT_MGR, m_WndMainTab);
	m_shadowMgrDlg.Create(IDD_LSHADOW_MGR, m_WndMainTab);
	m_msghookMgrDlg.Create(IDD_LMSGHOOK_MGR, m_WndMainTab);
	m_fileMgrDlg.Create(IDD_LFILE_MGR, m_WndMainTab);
	m_netMgrDlg.Create(IDD_LNETWORK_MGR, m_WndMainTab);
	m_regMgrDlg.Create(IDD_LREG_MGR, m_WndMainTab);
	m_IxAboutDlg.Create(IDD_LABOUT_DLG, m_WndMainTab);

	//////////////////////////////////////////////////////////////////////////
	//procMgr DlgLoader矩形尺寸
	CRect	proMgrDlgRect, proListRect;
	m_tab.GetClientRect(&proMgrDlgRect);	

	proMgrDlgRect.top		+= 22;			//顶部
	m_proMgrDlg.MoveWindow(&proMgrDlgRect);

	//drvMgr DlgLoader尺寸
	CRect	drvMgrDlgRct;
	m_tab.GetClientRect(&drvMgrDlgRct);		//获取TABMainView控件客户区大小
	drvMgrDlgRct.left		+=	3;			//左边
	drvMgrDlgRct.right		-=	3;			//右边
	drvMgrDlgRct.top		+= 22;			//顶部
	drvMgrDlgRct.bottom		-=	3;			//底部，留状态栏
	m_DrvMgrDlg.MoveWindow(&drvMgrDlgRct);

	CRect	ssdtMgrDlgRct;
	m_tab.GetClientRect(&ssdtMgrDlgRct);	//获取TABMainView控件客户区大小
 	ssdtMgrDlgRct.left		+=	2;			//左边
 	ssdtMgrDlgRct.right		-=	2;			//右边
	ssdtMgrDlgRct.top		+= 22;			//顶部
	ssdtMgrDlgRct.bottom	-=	3;			//底部，留状态栏
	m_ssdtMgrDlg.MoveWindow(&ssdtMgrDlgRct);

	m_shadowMgrDlg.MoveWindow(&ssdtMgrDlgRct);

	//fileMgr DlgLoader尺寸
	CRect	fileMgrDlgRct;
	m_tab.GetClientRect(&fileMgrDlgRct);	//获取TABMainView控件客户区大小
	fileMgrDlgRct.left		+=	2;			//左边
	fileMgrDlgRct.right		-=	2;			//右边
	fileMgrDlgRct.top		+= 22;			//顶部
	fileMgrDlgRct.bottom	-=	4;			//底部，留状态栏
	m_fileMgrDlg.MoveWindow(&fileMgrDlgRct);

	//netMgr DlgLoader尺寸
	CRect	netMgrDlgRct;
	m_tab.GetClientRect(&netMgrDlgRct);		//获取TABMainView控件客户区大小
	netMgrDlgRct.left		+=	2;			//左边
	netMgrDlgRct.right		-=	2;			//右边
	netMgrDlgRct.top		+= 22;			//顶部
	netMgrDlgRct.bottom		-=	4;			//底部，留状态栏
	m_netMgrDlg.MoveWindow(&netMgrDlgRct);

	//regMgr DlgLoader尺寸
	CRect	regMgrDlgRct;
	m_tab.GetClientRect(&regMgrDlgRct);		//获取TABMainView控件客户区大小
	regMgrDlgRct.left		+=	2;			//左边
	regMgrDlgRct.right		-=	2;			//右边
	regMgrDlgRct.top		+= 22;			//顶部
	regMgrDlgRct.bottom		-=	4;			//底部，留状态栏
	m_regMgrDlg.MoveWindow(&regMgrDlgRct);

	//////////////////////////////////////////////////////////////////////////
	// ring3hooksPage->ring3hooks TAB DLGLoader->msghookMgrDlg Rect矩形尺寸
	CRect	msghookMgrDlgRct;
	m_tab.GetClientRect(&msghookMgrDlgRct);		//获取TAB控件客户区大小
	msghookMgrDlgRct.left	+= 2;
	msghookMgrDlgRct.right	-= 3;
	msghookMgrDlgRct.top	+= 22;
	regMgrDlgRct.bottom		-=	4;
	m_msghookMgrDlg.MoveWindow(&msghookMgrDlgRct);

	//netMgr DlgLoader尺寸
	CRect	aboutMgrDlgRct;
	m_tab.GetClientRect(&aboutMgrDlgRct);	//获取TABMainView控件客户区大小
	aboutMgrDlgRct.left		+=	2;			//左边
	aboutMgrDlgRct.right	-=	2;			//右边
	aboutMgrDlgRct.top		+= 22;			//顶部
	aboutMgrDlgRct.bottom	-=	4;			//底部，留状态栏
	m_IxAboutDlg.MoveWindow(&aboutMgrDlgRct);

	//////////////////////////////////////////////////////////////////////////
	//显示、隐藏窗口
	m_proMgrDlg.ShowWindow(true);
	m_DrvMgrDlg.ShowWindow(false);
	m_ssdtMgrDlg.ShowWindow(false);
	m_shadowMgrDlg.ShowWindow(false);
	m_msghookMgrDlg.ShowWindow(false);
	m_regMgrDlg.ShowWindow(false);
	m_fileMgrDlg.ShowWindow(false);
	m_netMgrDlg.ShowWindow(false);
	m_IxAboutDlg.ShowWindow(false);

	//设置默认的选项卡
	m_tab.SetCurSel(0);		//主对话框视图 TAB
	//////////////////////////////////////////////////////////////////////////
	char	szfilter[MAX_PATH];
	CHAR	pszDriverImagePath[MAX_PATH];
	//获取进程目录
	GetModuleFileName( NULL, szfilter, MAX_PATH );
	
	for (int i = strlen(szfilter) - 1; i > -1; i--)
	{
		if (szfilter[i] == '\\')
		{
			//倒数第1个'\'+1索引处
			ZeroMemory( &szfilter[i+1], strlen(szfilter) - strlen(&szfilter[i+1]) );
			strcpy( pszDriverImagePath, szfilter );
			break;
		}
	}
	strcat( pszDriverImagePath, "ix.sys" );

	//提升进程权限
	AdjustPrivileges( SE_LOAD_DRIVER_NAME );
	//加载驱动
	LoadNTDriver( DRVNAME, pszDriverImagePath );
	//开启服务
	StartOrStopService( DRVNAME, TRUE );
	//Open Device
	g_hDriver=CreateFile("\\\\.\\IXER",
						  GENERIC_READ|GENERIC_WRITE,
						  0,
						  0,
						  OPEN_EXISTING,
						  FILE_ATTRIBUTE_SYSTEM,
						  0);
    if (g_hDriver==INVALID_HANDLE_VALUE)
    {

#ifdef _DEBUG
		char	errcode[512];
		sprintf(errcode, "状态:打开驱动句柄失败! 错误码:%d", GetLastError() );
		m_proMgrDlg.SetDlgItemText(IDC_STATIC_PROCESS, errcode);
#endif
	}

	//枚举进程
	m_proMgrDlg.ShowProcessList();
	//设为前景窗口
	AfxGetMainWnd()->SetForegroundWindow();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CixerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CixerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CixerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CixerDlg::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	//阴影效果
	AnimateWindow(GetSafeHwnd(), 720, AW_HIDE|AW_BLEND);
	//卸载驱动服务
	Deleteservice(DRVNAME);
	CDialog::OnClose();
}

void CixerDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
}

void CixerDlg::OnSelchangeTabMainView(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	switch (m_tab.GetCurSel())
	{
		//进程Tab
	case 0:
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_netMgrDlg.ShowWindow(false);		//网络
		m_IxAboutDlg.ShowWindow(false);

		//进程Mgr DlgShow
		m_proMgrDlg.ShowWindow(true);
		//枚举进程
		m_proMgrDlg.ShowProcessList();
		break;

		//驱动模块Tab
	case 1:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_netMgrDlg.ShowWindow(false);		//网络
		m_IxAboutDlg.ShowWindow(false);

		//驱动模块Mgr DlgShow
		m_DrvMgrDlg.ShowWindow(true);
		//枚举驱动
		m_DrvMgrDlg.ShowDrvModList();
		break;

		//ShadowSSDT
	case 2:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_netMgrDlg.ShowWindow(false);		//网络
		m_IxAboutDlg.ShowWindow(false);

		//SSDTMgr DlgShow
		m_ssdtMgrDlg.ShowWindow(true);		//SSDT Mgr
		//枚举SSDT
		m_ssdtMgrDlg.ShowSSDTList();
		break;

		//Ring0钩子Tab->DlgLoader->ShadowSSDT Mgr DlgShow
	case 3:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_netMgrDlg.ShowWindow(false);		//网络
		m_IxAboutDlg.ShowWindow(false);
		
		//SSDTMgr DlgShow
		m_shadowMgrDlg.ShowWindow(true);		//SSDT Mgr
		//枚举SSDT
		m_shadowMgrDlg.ShowShadowSSDTList();
		break;

		//MsgHook DlgShow
	case 4:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_ssdtMgrDlg.ShowWindow(false);		//Ring0钩子Tab->DlgLoader->SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//Ring0钩子Tab->DlgLoader->ShadowSSDT Mgr
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_netMgrDlg.ShowWindow(false);		//网络
		m_IxAboutDlg.ShowWindow(false);
		
		//MsgHookMgr DlgShow
		m_msghookMgrDlg.ShowWindow(true);	//Ring3钩子Tab->MsgHook Mgr;
		m_msghookMgrDlg.ShowMsgHookList();
		break;

		//注册表Tab
	case 5:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_netMgrDlg.ShowWindow(false);		//网络
		m_IxAboutDlg.ShowWindow(false);

		//注册表Mgr DlgShow
		m_regMgrDlg.ShowWindow(true);		//注册表
		break;

		//文件Tab
	case 6:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_netMgrDlg.ShowWindow(false);		//网络
		m_IxAboutDlg.ShowWindow(false);

		//文件Mgr DlgShow
		m_fileMgrDlg.ShowWindow(true);		//文件
		break;

		//网络Tab
	case 7:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_IxAboutDlg.ShowWindow(false);

		//网络Mgr DlgShow
		m_netMgrDlg.ShowWindow(true);
		m_netMgrDlg.ShowNetWorkList();
		break;

		//关于Tab
	case 8:
		m_proMgrDlg.ShowWindow(false);		//进程
		m_DrvMgrDlg.ShowWindow(false);		//驱动模块
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//注册表
		m_fileMgrDlg.ShowWindow(false);		//文件
		m_netMgrDlg.ShowWindow(false);
		
		//关于Mgr DlgShow
		m_IxAboutDlg.ShowWindow(true);
		break;

	default:
		m_proMgrDlg.SetDlgItemText(IDC_STATIC_PROCESS, "[X_TRAIL]：TAB default");
		break;
	}
	*pResult = 0;
}

BOOL CixerDlg::LoadNTDriver( char *pszDriverName, char *pszDriverImagePath )
{
	BOOL	bRet = TRUE;

	SC_HANDLE hServiceMgr;		//SCM管理器的句柄
	SC_HANDLE hService;			//NT驱动程序的服务句柄
	SERVICE_STATUS srvStatus;	//服务状态

	//打开服务控制管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hServiceMgr==NULL )
	{
		m_proMgrDlg.SetDlgItemText(IDC_STATIC_PROCESS, "状态：打开SCM管理器失败!");
		bRet = FALSE;
		goto leave;
	}
	
	hService=OpenService(hServiceMgr, pszDriverName, SERVICE_ALL_ACCESS);
	if (hService) //存在服务
	{
				//查询服务状态
		if ( QueryServiceStatus(hService, &srvStatus) )
		{
			if (srvStatus.dwCurrentState != SERVICE_RUNNING)
			{
				if ( !StartService(hService, NULL, NULL) )
				{
					//服务存在但启动失败
					goto leave;
				}
			}
		}
		else
			goto leave;
	}
	else
	{
		//创建驱动所对应的服务
		hService = CreateService(hServiceMgr,			// SCM管理器句柄
								pszDriverName,			// 驱动程序的在注册表中的名字  
								pszDriverName,			// 注册表驱动程序的 DisplayName 值  
								SERVICE_ALL_ACCESS,		// 加载驱动程序的访问权限  
								SERVICE_KERNEL_DRIVER,	// 表示加载的服务是驱动程序  
								SERVICE_DEMAND_START,	// 注册表驱动程序的 Start 值  
								SERVICE_ERROR_IGNORE,	// 注册表驱动程序的 ErrorControl 值  
								pszDriverImagePath,		// 注册表驱动程序的 ImagePath 值  
								NULL, NULL, NULL, NULL, NULL);
		
		//判断服务是否失败
		if( hService==NULL )  
		{
			ULONG	errcode;
			errcode = GetLastError();
			if( errcode != ERROR_IO_PENDING && errcode != ERROR_SERVICE_EXISTS )  
			{
				bRet = FALSE;
				goto leave;
			}
		}
	}

	bRet = TRUE;

	//关闭句柄
leave:
	if (hService)
		CloseServiceHandle(hService);

	if (hServiceMgr)
		CloseServiceHandle(hServiceMgr);

	return bRet;
}

BOOL CixerDlg::StartOrStopService( char *pszSrvName, BOOL boStartOrStop )
{
	SC_HANDLE hServiceMgr = NULL;		//SCM管理器的句柄
	SC_HANDLE hService    = NULL;		//NT驱动程序的服务句柄
	SERVICE_STATUS srvStatus;			//服务状态

	BOOL bRet = TRUE;

	//打开服务控制管理器
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hServiceMgr == NULL )  
	{
		//OpenSCManager失败
		bRet = FALSE;
		goto leave;
	}

	//打开服务获取句柄
	hService = OpenService( hServiceMgr, pszSrvName, SERVICE_ALL_ACCESS );  
	if(hService)  
	{
		if (boStartOrStop)
		{
			if( QueryServiceStatus(hService, &srvStatus) )
			{
				//服务没运行，则开启
				if (srvStatus.dwCurrentState != SERVICE_RUNNING)
				{
					if ( !StartService(hService, NULL, NULL) )
					{
						ULONG errcode;
						errcode = GetLastError();
						
						switch(errcode)
						{
							//设备被挂起
						case ERROR_IO_PENDING:
							break;
							
						default:
							break;
						}
						
						bRet = FALSE;
						//启动失败
						goto leave;
					}
				}
			}
		}
		else
		{
			//停止驱动程序
			if( !ControlService(hService, SERVICE_CONTROL_STOP , &srvStatus ) )  
			{
				bRet = FALSE;
				goto leave;
			} 
		}
	}  
//关闭句柄
leave:
	if(hService)
	{
		CloseServiceHandle(hService);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	
	return bRet;
}

BOOL CixerDlg::Deleteservice(CHAR *szSvrName)
{
	int i=0;
	SC_HANDLE		hSvrMgr;		//SCM管理器句柄
	SC_HANDLE		hService;		//NT驱动程序服务句柄
	SERVICE_STATUS	svrStatus;

	//打开SCM管理器
	if( ( hSvrMgr=OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) )==NULL )
    {
		return FALSE; //打开SCM失败.
    }
	//打开服务句柄
	if( ( hService=OpenService(hSvrMgr, szSvrName, SERVICE_ALL_ACCESS) )==NULL )
    {
		CloseServiceHandle(hSvrMgr);
		return FALSE;
    }
	
	//停止服务
	if( ControlService(hService, SERVICE_CONTROL_STOP, &svrStatus) )
    {
		//等待服务状态变成 SERVICE_STOPPED
		while(svrStatus.dwCurrentState == SERVICE_STOP_PENDING && i++ < 100)
		{
			//查询服务状态
			QueryServiceStatus(hService, &svrStatus);
			Sleep(10);
		}
			//服务已停止
		if(svrStatus.dwCurrentState == SERVICE_STOPPED)
		{
			//删除服务
//			StartOrStopService(hService, FALSE);
		}
    }

	//关闭服务、SCM句柄
	CloseServiceHandle(hService);
	CloseServiceHandle(hSvrMgr);
	return TRUE;
}

BOOL CixerDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message ==WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
