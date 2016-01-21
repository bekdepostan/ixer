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


//TAB�ؼ�
char	*tabMainView[] = {"����", "����ģ��", "SSDT", "ShadowSSDT", "��Ϣ����", "ע�Ա�", "�ļ�",
									"���j", "����"};
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
	//�������Ի�����ͼTAB��ǩҳ
	for (int iTab=0; iTab<tabMainViewCount; iTab++)
	{
		m_tab.InsertItem(iTab, tabMainView[iTab]);
	}

	//��������
	CWnd *m_WndMainTab = GetDlgItem(IDC_TAB_MAIN_VIEW);
	//�����Ի��򣬲��ҽ�IDC_TAB1�ؼ���Ϊ������
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
	//procMgr DlgLoader���γߴ�
	CRect	proMgrDlgRect, proListRect;
	m_tab.GetClientRect(&proMgrDlgRect);	

	proMgrDlgRect.top		+= 22;			//����
	m_proMgrDlg.MoveWindow(&proMgrDlgRect);

	//drvMgr DlgLoader�ߴ�
	CRect	drvMgrDlgRct;
	m_tab.GetClientRect(&drvMgrDlgRct);		//��ȡTABMainView�ؼ��ͻ�����С
	drvMgrDlgRct.left		+=	3;			//���
	drvMgrDlgRct.right		-=	3;			//�ұ�
	drvMgrDlgRct.top		+= 22;			//����
	drvMgrDlgRct.bottom		-=	3;			//�ײ�����״̬��
	m_DrvMgrDlg.MoveWindow(&drvMgrDlgRct);

	CRect	ssdtMgrDlgRct;
	m_tab.GetClientRect(&ssdtMgrDlgRct);	//��ȡTABMainView�ؼ��ͻ�����С
 	ssdtMgrDlgRct.left		+=	2;			//���
 	ssdtMgrDlgRct.right		-=	2;			//�ұ�
	ssdtMgrDlgRct.top		+= 22;			//����
	ssdtMgrDlgRct.bottom	-=	3;			//�ײ�����״̬��
	m_ssdtMgrDlg.MoveWindow(&ssdtMgrDlgRct);

	m_shadowMgrDlg.MoveWindow(&ssdtMgrDlgRct);

	//fileMgr DlgLoader�ߴ�
	CRect	fileMgrDlgRct;
	m_tab.GetClientRect(&fileMgrDlgRct);	//��ȡTABMainView�ؼ��ͻ�����С
	fileMgrDlgRct.left		+=	2;			//���
	fileMgrDlgRct.right		-=	2;			//�ұ�
	fileMgrDlgRct.top		+= 22;			//����
	fileMgrDlgRct.bottom	-=	4;			//�ײ�����״̬��
	m_fileMgrDlg.MoveWindow(&fileMgrDlgRct);

	//netMgr DlgLoader�ߴ�
	CRect	netMgrDlgRct;
	m_tab.GetClientRect(&netMgrDlgRct);		//��ȡTABMainView�ؼ��ͻ�����С
	netMgrDlgRct.left		+=	2;			//���
	netMgrDlgRct.right		-=	2;			//�ұ�
	netMgrDlgRct.top		+= 22;			//����
	netMgrDlgRct.bottom		-=	4;			//�ײ�����״̬��
	m_netMgrDlg.MoveWindow(&netMgrDlgRct);

	//regMgr DlgLoader�ߴ�
	CRect	regMgrDlgRct;
	m_tab.GetClientRect(&regMgrDlgRct);		//��ȡTABMainView�ؼ��ͻ�����С
	regMgrDlgRct.left		+=	2;			//���
	regMgrDlgRct.right		-=	2;			//�ұ�
	regMgrDlgRct.top		+= 22;			//����
	regMgrDlgRct.bottom		-=	4;			//�ײ�����״̬��
	m_regMgrDlg.MoveWindow(&regMgrDlgRct);

	//////////////////////////////////////////////////////////////////////////
	// ring3hooksPage->ring3hooks TAB DLGLoader->msghookMgrDlg Rect���γߴ�
	CRect	msghookMgrDlgRct;
	m_tab.GetClientRect(&msghookMgrDlgRct);		//��ȡTAB�ؼ��ͻ�����С
	msghookMgrDlgRct.left	+= 2;
	msghookMgrDlgRct.right	-= 3;
	msghookMgrDlgRct.top	+= 22;
	regMgrDlgRct.bottom		-=	4;
	m_msghookMgrDlg.MoveWindow(&msghookMgrDlgRct);

	//netMgr DlgLoader�ߴ�
	CRect	aboutMgrDlgRct;
	m_tab.GetClientRect(&aboutMgrDlgRct);	//��ȡTABMainView�ؼ��ͻ�����С
	aboutMgrDlgRct.left		+=	2;			//���
	aboutMgrDlgRct.right	-=	2;			//�ұ�
	aboutMgrDlgRct.top		+= 22;			//����
	aboutMgrDlgRct.bottom	-=	4;			//�ײ�����״̬��
	m_IxAboutDlg.MoveWindow(&aboutMgrDlgRct);

	//////////////////////////////////////////////////////////////////////////
	//��ʾ�����ش���
	m_proMgrDlg.ShowWindow(true);
	m_DrvMgrDlg.ShowWindow(false);
	m_ssdtMgrDlg.ShowWindow(false);
	m_shadowMgrDlg.ShowWindow(false);
	m_msghookMgrDlg.ShowWindow(false);
	m_regMgrDlg.ShowWindow(false);
	m_fileMgrDlg.ShowWindow(false);
	m_netMgrDlg.ShowWindow(false);
	m_IxAboutDlg.ShowWindow(false);

	//����Ĭ�ϵ�ѡ�
	m_tab.SetCurSel(0);		//���Ի�����ͼ TAB
	//////////////////////////////////////////////////////////////////////////
	char	szfilter[MAX_PATH];
	CHAR	pszDriverImagePath[MAX_PATH];
	//��ȡ����Ŀ¼
	GetModuleFileName( NULL, szfilter, MAX_PATH );
	
	for (int i = strlen(szfilter) - 1; i > -1; i--)
	{
		if (szfilter[i] == '\\')
		{
			//������1��'\'+1������
			ZeroMemory( &szfilter[i+1], strlen(szfilter) - strlen(&szfilter[i+1]) );
			strcpy( pszDriverImagePath, szfilter );
			break;
		}
	}
	strcat( pszDriverImagePath, "ix.sys" );

	//��������Ȩ��
	AdjustPrivileges( SE_LOAD_DRIVER_NAME );
	//��������
	LoadNTDriver( DRVNAME, pszDriverImagePath );
	//��������
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
		sprintf(errcode, "״̬:���������ʧ��! ������:%d", GetLastError() );
		m_proMgrDlg.SetDlgItemText(IDC_STATIC_PROCESS, errcode);
#endif
	}

	//ö�ٽ���
	m_proMgrDlg.ShowProcessList();
	//��Ϊǰ������
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
	//��ӰЧ��
	AnimateWindow(720, AW_HIDE|AW_BLEND);
	//ж����������
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
		//����Tab
	case 0:
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_netMgrDlg.ShowWindow(false);		//����
		m_IxAboutDlg.ShowWindow(false);

		//����Mgr DlgShow
		m_proMgrDlg.ShowWindow(true);
		//ö�ٽ���
		m_proMgrDlg.ShowProcessList();
		break;

		//����ģ��Tab
	case 1:
		m_proMgrDlg.ShowWindow(false);		//����
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_netMgrDlg.ShowWindow(false);		//����
		m_IxAboutDlg.ShowWindow(false);

		//����ģ��Mgr DlgShow
		m_DrvMgrDlg.ShowWindow(true);
		//ö������
		m_DrvMgrDlg.ShowDrvModList();
		break;

		//ShadowSSDT
	case 2:
		m_proMgrDlg.ShowWindow(false);		//����
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_netMgrDlg.ShowWindow(false);		//����
		m_IxAboutDlg.ShowWindow(false);

		//SSDTMgr DlgShow
		m_ssdtMgrDlg.ShowWindow(true);		//SSDT Mgr
		//ö��SSDT
		m_ssdtMgrDlg.ShowSSDTList();
		break;

		//Ring0����Tab->DlgLoader->ShadowSSDT Mgr DlgShow
	case 3:
		m_proMgrDlg.ShowWindow(false);		//����
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_netMgrDlg.ShowWindow(false);		//����
		m_IxAboutDlg.ShowWindow(false);
		
		//SSDTMgr DlgShow
		m_shadowMgrDlg.ShowWindow(true);		//SSDT Mgr
		//ö��SSDT
		m_shadowMgrDlg.ShowShadowSSDTList();
		break;

		//MsgHook DlgShow
	case 4:
		m_proMgrDlg.ShowWindow(false);		//����
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_ssdtMgrDlg.ShowWindow(false);		//Ring0����Tab->DlgLoader->SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//Ring0����Tab->DlgLoader->ShadowSSDT Mgr
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_netMgrDlg.ShowWindow(false);		//����
		m_IxAboutDlg.ShowWindow(false);
		
		//MsgHookMgr DlgShow
		m_msghookMgrDlg.ShowWindow(true);	//Ring3����Tab->MsgHook Mgr;
		m_msghookMgrDlg.ShowMsgHookList();
		break;

		//ע���Tab
	case 5:
		m_proMgrDlg.ShowWindow(false);		//����
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr;
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_netMgrDlg.ShowWindow(false);		//����
		m_IxAboutDlg.ShowWindow(false);

		//ע���Mgr DlgShow
		m_regMgrDlg.ShowWindow(true);		//ע���
		break;

		//�ļ�Tab
	case 6:
		m_proMgrDlg.ShowWindow(false);		//����
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_netMgrDlg.ShowWindow(false);		//����
		m_IxAboutDlg.ShowWindow(false);

		//�ļ�Mgr DlgShow
		m_fileMgrDlg.ShowWindow(true);		//�ļ�
		break;

		//����Tab
	case 7:
		m_proMgrDlg.ShowWindow(false);		//����
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_IxAboutDlg.ShowWindow(false);

		//����Mgr DlgShow
		m_netMgrDlg.ShowWindow(true);
		m_netMgrDlg.ShowNetWorkList();
		break;

		//����Tab
	case 8:
		m_proMgrDlg.ShowWindow(false);		//����
		m_DrvMgrDlg.ShowWindow(false);		//����ģ��
		m_ssdtMgrDlg.ShowWindow(false);		//SSDT Mgr
		m_shadowMgrDlg.ShowWindow(false);	//ShadowSSDT Mgr
		m_msghookMgrDlg.ShowWindow(false);	//MsgHook Mgr
		m_regMgrDlg.ShowWindow(false);		//ע���
		m_fileMgrDlg.ShowWindow(false);		//�ļ�
		m_netMgrDlg.ShowWindow(false);
		
		//����Mgr DlgShow
		m_IxAboutDlg.ShowWindow(true);
		break;

	default:
		m_proMgrDlg.SetDlgItemText(IDC_STATIC_PROCESS, "[X_TRAIL]��TAB default");
		break;
	}
	*pResult = 0;
}

BOOL CixerDlg::LoadNTDriver( char *pszDriverName, char *pszDriverImagePath )
{
	BOOL	bRet = TRUE;

	SC_HANDLE hServiceMgr;		//SCM�������ľ��
	SC_HANDLE hService;			//NT��������ķ�����
	SERVICE_STATUS srvStatus;	//����״̬

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hServiceMgr==NULL )
	{
		m_proMgrDlg.SetDlgItemText(IDC_STATIC_PROCESS, "״̬����SCM������ʧ��!");
		bRet = FALSE;
		goto leave;
	}
	
	hService=OpenService(hServiceMgr, pszDriverName, SERVICE_ALL_ACCESS);
	if (hService) //���ڷ���
	{
				//��ѯ����״̬
		if ( QueryServiceStatus(hService, &srvStatus) )
		{
			if (srvStatus.dwCurrentState != SERVICE_RUNNING)
			{
				if ( !StartService(hService, NULL, NULL) )
				{
					//������ڵ�����ʧ��
					goto leave;
				}
			}
		}
		else
			goto leave;
	}
	else
	{
		//������������Ӧ�ķ���
		hService = CreateService(hServiceMgr,			// SCM���������
								pszDriverName,			// �����������ע����е�����  
								pszDriverName,			// ע������������ DisplayName ֵ  
								SERVICE_ALL_ACCESS,		// ������������ķ���Ȩ��  
								SERVICE_KERNEL_DRIVER,	// ��ʾ���صķ�������������  
								SERVICE_DEMAND_START,	// ע������������ Start ֵ  
								SERVICE_ERROR_IGNORE,	// ע������������ ErrorControl ֵ  
								pszDriverImagePath,		// ע������������ ImagePath ֵ  
								NULL, NULL, NULL, NULL, NULL);
		
		//�жϷ����Ƿ�ʧ��
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

	//�رվ��
leave:
	if (hService)
		CloseServiceHandle(hService);

	if (hServiceMgr)
		CloseServiceHandle(hServiceMgr);

	return bRet;
}

BOOL CixerDlg::StartOrStopService( char *pszSrvName, BOOL boStartOrStop )
{
	SC_HANDLE hServiceMgr = NULL;		//SCM�������ľ��
	SC_HANDLE hService    = NULL;		//NT��������ķ�����
	SERVICE_STATUS srvStatus;			//����״̬

	BOOL bRet = TRUE;

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if( hServiceMgr == NULL )  
	{
		//OpenSCManagerʧ��
		bRet = FALSE;
		goto leave;
	}

	//�򿪷����ȡ���
	hService = OpenService( hServiceMgr, pszSrvName, SERVICE_ALL_ACCESS );  
	if(hService)  
	{
		if (boStartOrStop)
		{
			if( QueryServiceStatus(hService, &srvStatus) )
			{
				//����û���У�����
				if (srvStatus.dwCurrentState != SERVICE_RUNNING)
				{
					if ( !StartService(hService, NULL, NULL) )
					{
						ULONG errcode;
						errcode = GetLastError();
						
						switch(errcode)
						{
							//�豸������
						case ERROR_IO_PENDING:
							break;
							
						default:
							break;
						}
						
						bRet = FALSE;
						//����ʧ��
						goto leave;
					}
				}
			}
		}
		else
		{
			//ֹͣ��������
			if( !ControlService(hService, SERVICE_CONTROL_STOP , &srvStatus ) )  
			{
				bRet = FALSE;
				goto leave;
			} 
		}
	}  
//�رվ��
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
	SC_HANDLE		hSvrMgr;		//SCM���������
	SC_HANDLE		hService;		//NT�������������
	SERVICE_STATUS	svrStatus;

	//��SCM������
	if( ( hSvrMgr=OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) )==NULL )
    {
		return FALSE; //��SCMʧ��.
    }
	//�򿪷�����
	if( ( hService=OpenService(hSvrMgr, szSvrName, SERVICE_ALL_ACCESS) )==NULL )
    {
		CloseServiceHandle(hSvrMgr);
		return FALSE;
    }
	
	//ֹͣ����
	if( ControlService(hService, SERVICE_CONTROL_STOP, &svrStatus) )
    {
		//�ȴ�����״̬��� SERVICE_STOPPED
		while(svrStatus.dwCurrentState == SERVICE_STOP_PENDING && i++ < 100)
		{
			//��ѯ����״̬
			QueryServiceStatus(hService, &svrStatus);
			Sleep(10);
		}
			//������ֹͣ
		if(svrStatus.dwCurrentState == SERVICE_STOPPED)
		{
			//ɾ������
//			StartOrStopService(hService, FALSE);
		}
    }

	//�رշ���SCM���
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
