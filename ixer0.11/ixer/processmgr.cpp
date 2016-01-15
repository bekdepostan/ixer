// processMgr.cpp : implementation file
//

#include "stdafx.h"
#include "ixer.h"
#include "ixerDlg.h"
#include "moduleMgr.h"
#include "threadMgr.h"
#include "common.h"
#include "processMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUMCOLUMNS	4
#define MAX_PROCESS 256

extern CixerApp theApp;
extern CModuleMgr	m_modMgrDlg;
int		seledProc;				//选中进程item
ULONG	g_eProc;				//选中item的进程eProcess
ULONG	g_dwPId;				//选中进程ID(选中item的进程ID)
CString	g_strProcName;			//选中item的进程名
/////////////////////////////////////////////////////////////////////////////
// CProcessMgr dialog


CProcessMgr::CProcessMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcessMgr)
	//}}AFX_DATA_INIT
}


void CProcessMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessMgr)
	DDX_Control(pDX, IDC_STATIC_PROCESS, m_statusProcess);
	DDX_Control(pDX, IDC_PROCESS_LISTS, m_proList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessMgr, CDialog)
	//{{AFX_MSG_MAP(CProcessMgr)
	ON_NOTIFY(NM_RCLICK, IDC_PROCESS_LISTS, OnRclickProcessLists)
	ON_COMMAND(IDM_REFRESH_PRO_LIST, OnRefreshProList)
	ON_COMMAND(IDM_PRO_MODULE_LIST, OnProModuleList)
	ON_COMMAND(IDM_PRO_THREAD_LIST, OnProThreadList)
	ON_COMMAND(IDM_PROC_KILLER, OnProcKiller)
	ON_COMMAND(IDM_COPY_PROCNAME, OnCopyProcName)
	ON_COMMAND(IDM_COPY_PROCPATH, OnCopyProcPath)
	ON_COMMAND(IDM_GPS_PROCFILE, OnGpsProcFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CProcessMgr m_proMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CProcessMgr message handlers

BOOL CProcessMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	//插入List头
	m_proMgrDlg.m_proList.SetHeadings( _T("映像名称,  110;进程ID,  55;父进程ID,  65;映像路径,  220;EPROCESS,  75;访问状态,  88;文件厂商,  142") ); //设置列头信息
	m_proMgrDlg.m_proList.LoadColumnInfo();	//加载列信息
	//修改ListCtrl扩展样式
	DWORD dwStyle = m_proMgrDlg.m_proList.GetStyle();								//取得ListCtrl样式
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//修改ListCtrl风格，|WS_VSCROLL|WS_HSCROLL
	m_proMgrDlg.m_proList.SetExtendedStyle(dwStyle);     //重设风格
	
	//列表框左边的图标列表
	m_proMgrDlg.images.Create(16, 16, ILC_COLOR32, 10, 10);
	//设置默认图标Index
//	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_proMgrDlg.nDefaultIconIndex = m_proMgrDlg.images.Add(NULL);	//空白icon
	m_proMgrDlg.m_proList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessMgr::ShowProcessList()
{
	//获取当前ProcessList并插入进程ListCtrl，返回进程数
	GetCurrentProcessList(&m_proList);
}

//////////////////////////////////////////////////////////////////////////
//	获取进程列表
int CProcessMgr::GetCurrentProcessList(CSortListCtrl *m_proList)
{
	BOOL	b_Ret;
	ULONG	numOfproc;	//进程数
	ULONG	retBytes;
	PPROCESS_ITEM pProcInfo=NULL;
	int		iconIndex;
	char	szPid[16];
	char	szPPid[16];
	char	szEproc[16];

	CThreadMgr m_TrdMgr;
	HANDLE	hProc;	
	int		hided=0;
	int		r3Access=0;
	char	num_s[128];

	//提升进程权限
	AdjustPrivileges( SE_DEBUG_NAME );

	//清空ListCtrl列表
	m_proList->DeleteAllItems();
	//驱动句柄无效、为空则返回0
	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
		return 0;

	//枚举进程并返回个数
	b_Ret=DeviceIoControl( g_hDriver,
							IOCTL_ENUM_PROCESS,
							NULL,
							0,
							&numOfproc,
							sizeof(ULONG),
							&retBytes,
							NULL );
	if (b_Ret)
	{
		if (numOfproc)
		{
			//按个数申请进程结构内存
			pProcInfo=(PROCESS_ITEM *)malloc(sizeof(PROCESS_ITEM)*numOfproc);
			//内存清零
			ZeroMemory(pProcInfo, sizeof(PROCESS_ITEM)*numOfproc);
			//读取数据
			b_Ret=DeviceIoControl( g_hDriver,
									IOCTL_GET_PROCESS_LIST,
									NULL,
									0,
									pProcInfo,
									sizeof(PROCESS_ITEM)*numOfproc,
									&retBytes,
									NULL );
			if (b_Ret)
			{
				for (ULONG i=0; i<numOfproc; i++)
				{
					//获取icon索引
					iconIndex = GetIconIndex(pProcInfo[i].pszImagePath);

					//进程id
					ltoa(pProcInfo[i].ulPid, szPid, 10);					//PID转字符串
					//父进程id为0则置"-"
					if (pProcInfo[i].ulPPid)
						ltoa(pProcInfo[i].ulPPid, szPPid, 10);				//PPID转字符串
					else
						strcpy(szPPid, "-" );

					//格式化为0x开头的字符串
					sprintf(szEproc, "0x%X", (DWORD)pProcInfo[i].pEproc);

					//获取文件公司名
					m_modMgrDlg.GetFileCorporation(pProcInfo[i].pszImagePath, pProcInfo[i].pszFileCorp);

					//检测r3访问状态
					hProc=OpenProcess(PROCESS_ALL_ACCESS, FALSE, pProcInfo[i].ulPid);
					if ( !hProc )
					{
						strcpy( pProcInfo[i].r3Access, "    拒绝" );
						r3Access++;

						if (pProcInfo[i].hideFlag)
						{
							strcat(pProcInfo[i].r3Access, " 隐藏");
							hided++;
						}
					}
					else
					{
						strcpy( pProcInfo[i].r3Access, "      -" );

						if (pProcInfo[i].hideFlag)
						{
							strcpy( pProcInfo[i].r3Access, " 隐藏" );
							hided++;
						}

						CloseHandle(hProc);
					}

					//添加进程项到ListCtrl
					m_proList->AddItem( iconIndex,						//icon索引
										pProcInfo[i].pszImageName,		//进程名
										szPid,							//进程id
										szPPid,							//父进程id
										pProcInfo[i].pszImagePath,		//进程路径
										szEproc,						//EPROCESS
										pProcInfo[i].r3Access,			//用户层访问状态
										pProcInfo[i].pszFileCorp);		//文件厂商
				}
			}
			free(pProcInfo);
		}
	}
	else
		showErrBox();

	//格式化字符串
	sprintf(num_s, "进程： %d,  隐藏进程： %d,  应用层拒绝访问： %d", numOfproc, hided, r3Access);
	//设置状态栏
	m_proMgrDlg.m_proList.GetParent()->SetDlgItemText(IDC_STATIC_PROCESS, num_s);
	return numOfproc;
}

int CProcessMgr::GetIconIndex(char *path)
{
	int		nValue;
	BOOL	ret;
	int		nIconIndex;
	//////////////////////////////////////////////////////////////////////////
	//从iconMap获取图标
	//////////////////////////////////////////////////////////////////////////
	ret = iconMap.Lookup(_T(path), nValue);
	if(ret)	//已存在
	{
		nIconIndex=nValue;
	}
	else	//没找到，则添加一个新的
	{
		SHFILEINFO sfi;
		ret=SHGetFileInfo(path, 0, &sfi, sizeof(sfi), SHGFI_ICON|SHGFI_SMALLICON|SHGFI_LARGEICON);
		if(ret)
		{
			nIconIndex = images.Add(sfi.hIcon);
			//把这个图标路径和index加入iconMap
			iconMap.SetAt(_T(path), nIconIndex);
		}
		else
		{	//默认图标索引
			nIconIndex=nDefaultIconIndex;
		}
	}
	return nIconIndex;
}
			//响应右键选中进程item消息
void CProcessMgr::OnRclickProcessLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		CString str, strEproc;
		CHAR	*pChr={'\0'};
		seledProc=0;			//选中item
		//获取鼠标在ListCtrl的点击位置
		CPoint pt;
		GetCursorPos(&pt);
		//加载进程的右键菜单
		CMenu menu;
		menu.LoadMenu(IDR_PROC_RPOP);
		//获取第一个子菜单
		CMenu *pMenu = menu.GetSubMenu(0);
		//弹出菜单并追踪选中的Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		seledProc = m_proList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);	//-1从第1个item开始搜索行,匹配选中item

		/*进程*/
		// 选中item的第1列，即进程名
		g_strProcName = m_proList.GetItemText(seledProc, 0);
		// 选中item的第2列，即进程Pid
		str = m_proList.GetItemText(seledProc, 1);
		pChr = str.GetBuffer( str.GetLength() );
		// 转换并保存
		g_dwPId = atol(pChr);										//转换字符串为ULONG，保存到全局变量g_dwPId

		/*线程*/
		// 选中item的第4列，即线程ETHREAD
		strEproc = m_proList.GetItemText(seledProc, 4);
		// 转换并保存选中item模块的基址
		g_eProc = strtoul( (LPCTSTR)strEproc, '\0', 16);			//转换字符串为ULONG，保存到全局变量g_eThread
	}
	*pResult = 0;
}
				//刷新
void CProcessMgr::OnRefreshProList() 
{
	// TODO: Add your command handler code here
	ShowProcessList();
}
				//结束进程
void CProcessMgr::OnProcKiller() 
{
	// TODO: Add your command handler code here
	//发送进程ID给驱动
	ULONG	dwPId=g_dwPId;
	DWORD	retBytes;
	
	ZeroMemory(g_ctrlBuff,MAXBUFFERSIZE);
	memcpy( g_ctrlBuff, &dwPId, sizeof(ULONG) );			//把“进程ID”拷贝到输入缓冲区
	
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//把“进程ID”发送到驱动设备
		DeviceIoControl(g_hDriver, IOCTL_PROCESS_KILLER, g_ctrlBuff, sizeof(DWORD), NULL, 0, &retBytes, 0);
		ShowProcessList();
	}
}
				//进程模块
void CProcessMgr::OnProModuleList() 
{
	// TODO: Add your command handler code here
	CModuleMgr	*moduleDlg = new CModuleMgr();
	moduleDlg->Create(IDD_NEW_DLL_MGR_DLG, this);
	moduleDlg->CenterWindow();
	moduleDlg->ShowWindow(SW_SHOW);
}
				//进程线程
void CProcessMgr::OnProThreadList() 
{
	// TODO: Add your command handler code here
	CThreadMgr	*threadDlg = new CThreadMgr();
	threadDlg->Create(IDD_NEW_THREAD_MGR_DLG, this);
	threadDlg->CenterWindow();
	threadDlg->ShowWindow(SW_SHOW);
}
				//复制进程名
void CProcessMgr::OnCopyProcName() 
{
	// TODO: Add your command handler code here
	if ( !g_strProcName.IsEmpty() )
		SetClipboardText( (LPSTR)(LPCTSTR)g_strProcName );
}
				//复制进程路径
void CProcessMgr::OnCopyProcPath() 
{
	// TODO: Add your command handler code here
	// 选中item的第4列，即进程路径
	CString procPath=m_proList.GetItemText(seledProc, 3);
	if ( !procPath.IsEmpty() )
		SetClipboardText( (LPSTR)(LPCTSTR)procPath );
}
				//定位进程文件
void CProcessMgr::OnGpsProcFile() 
{
	// TODO: Add your command handler code here
	// 选中item的第4列，即进程路径
	CString procPath=m_proList.GetItemText(seledProc, 3);
	if ( !procPath.IsEmpty() )
		WinExec("explorer.exe /e,/select," + procPath, SW_SHOW);
}
				//屏蔽Esc键关闭承载ListCtrl的Dlg
BOOL CProcessMgr::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		//过滤Esc键
		if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

BOOL CProcessMgr::SetClipboardText(char *pText)
{
	HGLOBAL hClip;

	if (pText[0] == '\0')
		return TRUE;

	//打开剪切板
	if ( !OpenClipboard() )
		return FALSE;

	//清空剪切板内容
	if ( !EmptyClipboard() )
	{
		//关闭剪切板
		CloseClipboard();
		return FALSE;
	}

	int strSize=strlen(pText) + 1;	//size + '\0'
	//在堆上申请内存
	hClip=GlobalAlloc(GMEM_MOVEABLE, strSize);	
	if (!hClip)
	{
		//关闭剪切板
		CloseClipboard();
		return FALSE;
	}

	char *pBuf;
	//锁住堆内存块，并内存块句柄转换为字符型指针
	pBuf=(char *)GlobalLock(hClip);
	if (!pBuf)
	{
		//释放堆内存块
		GlobalFree(pBuf);
		//关闭剪切板
		CloseClipboard();
		return FALSE;
	}

	//拷贝文本内容到分配的堆内存块中
	strcpy(pBuf, pText);
	//解锁堆内存块
	GlobalUnlock(hClip);
	//将字符串内存放入剪切板的资源管理中
	if ( !SetClipboardData(CF_TEXT, hClip) )
	{
		//释放堆内存块
		GlobalFree(pBuf);
		//关闭剪切板
		CloseClipboard();
		return FALSE;
	}

	//关闭剪切板
	CloseClipboard();
	return TRUE;
}
