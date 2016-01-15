// DrvModMgr.cpp : implementation file
//

#include "stdafx.h"
#include <winsvc.h>
#include "processMgr.h"
#include "Module.h"
#include "ModuleMgr.h"
#include "Memory.h"
#include "ixer.h"
#include "DrvModMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CModuleMgr	m_modMgrDlg;
extern CProcessMgr	m_proMgrDlg;
int	seledDrv;		//选中item行
/////////////////////////////////////////////////////////////////////////////
// CDrvModMgr dialog


CDrvModMgr::CDrvModMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CDrvModMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDrvModMgr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CDrvModMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDrvModMgr)
	DDX_Control(pDX, IDC_STATIC_DRVMOD, m_statusDrvMod);
	DDX_Control(pDX, IDC_DRVMOD_LISTS, m_DrvModList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDrvModMgr, CDialog)
	//{{AFX_MSG_MAP(CDrvModMgr)
	ON_NOTIFY(NM_RCLICK, IDC_DRVMOD_LISTS, OnRclickDrvmodLists)
	ON_COMMAND(IDM_REFRESH_DRV_LIST, OnRefreshDrvList)
	ON_COMMAND(IDM_COPY_DRVNAME, OnCopyDrvName)
	ON_COMMAND(IDM_COPY_DRVPATH, OnCopyDrvPath)
	ON_COMMAND(IDM_GPS_DRVFILE, OnGpsDrvFile)
	ON_COMMAND(IDM_DELETE_DRVFILE, OnDeleteDrvfile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
//////////////////////////////////////////////////////////////////////////
CDrvModMgr m_DrvMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CDrvModMgr message handlers

BOOL CDrvModMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//插入ListHead
	m_DrvModList.SetHeadings(_T("驱动名,  85;基址,  72;大小,  72;驱动对象,  72;模块路径,  250;服务名,  62;加载顺序,  40;文件厂商,  105"));	//入口点,  72;卸载例程,  72;顺序,  36;
	m_DrvModList.LoadColumnInfo();	//加载列信息
	//修改ListCtrl扩展样式
	DWORD dwStyle;
	dwStyle = m_DrvModList.GetStyle();	//取得ListCtrl样式
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//修改ListCtrl风格，|WS_VSCROLL|WS_HSCROLL
	m_DrvModList.SetExtendedStyle(dwStyle);     //重设风格
	m_DrvModList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDrvModMgr::ShowDrvModList()
{
	//获取DriverList插入进程ListCtrl，并返回进程数
	GetDrvModList(&m_DrvModList);
	return;
}

int CDrvModMgr::GetDrvModList(CSortListCtrl *m_DrvModList)
{
	ULONG	ifDrvNum=MAX_DRIVER_NUM;
	BOOL	b_Ret;
	ULONG	numOfDrv=0;
	ULONG	drvCount=0;
 	ULONG	retBytes;
	ULONG	i=0;

	char	drvModName_c[MAXNAMELEN];		//驱动名
	char	drvSrvName_c[MAXNAMELEN]="";	//服务名
	char	drvModPath_c[MAXPATHLEN];
	char	fixDrvPath_c[MAXPATHLEN];		//路径
	char	baseAddress_c[16];				//基址
	char	sizeOfImage_c[16];				//大小
	char	drvObject_c[16];				//驱动对象
	char	ulLoadOrder_c[16];				//加载顺序

	PDRVMOD_ITEM	pDrvInfo=NULL;
	PDRV_SRVNAME	pDrvSrvName=NULL;
	int		srvNums=0;
	int		hided=0;
	int		suspectImages=0;
	char	drvNum_s[128];
	int		iDefIconDrv;

	//清空ListCtrl列表
	m_DrvModList->DeleteAllItems();

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return 0;
	}

	//申请内存
	pDrvSrvName=(DRV_SRVNAME *)malloc(sizeof(DRV_SRVNAME)*ifDrvNum);
	if ( !pDrvSrvName )
		free(pDrvSrvName);

	//收集准备好驱动服务名
	srvNums=ReadySrvName(pDrvSrvName);

	//按个数申请进程结构内存
	pDrvInfo=(DRVMOD_ITEM *)malloc(sizeof(DRVMOD_ITEM)*ifDrvNum);
	if (!pDrvInfo)
	{
		free(pDrvInfo);
		return drvCount;
	}

	//内存清零
	ZeroMemory(pDrvInfo, sizeof(DRVMOD_ITEM)*ifDrvNum);
	//读取数据
	b_Ret=DeviceIoControl(g_hDriver, IOCTL_ENUM_DRIVER, NULL, 0, pDrvInfo, sizeof(DRVMOD_ITEM)*ifDrvNum, &retBytes, NULL);
	if (b_Ret)
	{
		if (retBytes)
		{
			drvCount=retBytes/sizeof(DRVMOD_ITEM);
			for (i=0; i<drvCount; i++)
			{
				if (pDrvInfo[i].ulBaseAddr)
				{
					//转换模块基址
					sprintf(baseAddress_c, "0x%08X", pDrvInfo[i].ulBaseAddr);
					//转换模块大小
					sprintf(sizeOfImage_c, "0x%08X", pDrvInfo[i].ulImageSize);
					//转换加载顺序
					sprintf(ulLoadOrder_c, "%d", pDrvInfo[i].ulLoadOrder);

					//驱动对象有则转换
					if (pDrvInfo[i].pDrvObject)
						sprintf(drvObject_c, "0x%08X", pDrvInfo[i].pDrvObject);
					else
						strcpy(drvObject_c, "-");

					//驱动名宽字符转多几节
					WideCharToMultiByte(CP_ACP, 0, pDrvInfo[i].pwszDrvName, -1, drvModName_c, sizeof(drvModName_c), NULL, NULL);
					//驱动路径宽字符转多几节
					WideCharToMultiByte(CP_ACP, 0, pDrvInfo[i].pwszDrvPath, -1, drvModPath_c, sizeof(drvModPath_c), NULL, NULL);

					//修正路径
					TranslateFilePathName(drvModPath_c, fixDrvPath_c);
					//获取驱动文件公司
					m_modMgrDlg.GetFileCorporation(fixDrvPath_c, pDrvInfo[i].pszFileCorp);

					//遍历查找驱动服务名
					if (srvNums && pDrvSrvName)
					{
						for (int i=0; i<srvNums; i++)
						{
							//驱动路径与服务路径匹配，则获取服务名
							if (stricmp(fixDrvPath_c, pDrvSrvName[i].szSrvPath) == 0)
							{
								strcpy(drvSrvName_c, pDrvSrvName[i].szSrvName);
								break;
							}
						}
					}

					//获取图标
					iDefIconDrv=m_proMgrDlg.GetIconIndex( fixDrvPath_c );

					//隐藏驱动的显示暂时借用文件厂商栏，自己看着办吧~
					if (pDrvInfo[i].hideFlags)
					{
						strcpy( pDrvInfo[i].pszFileCorp, "隐藏" );
						hided++;
					}

					//加入列表控件(""驱动名,  85;基址,  72;大小,  72;驱动对象,  72;卸载例程,  72;模块路径,  220;顺序,  36;次数,  36;文件厂商,  90")
					m_DrvModList->AddItem(
											iDefIconDrv,				//图标索引号
											drvModName_c,				//模块名
											baseAddress_c,				//基址
											sizeOfImage_c,				//大小
											drvObject_c,				//驱动对象
											fixDrvPath_c,				//模块路径
											drvSrvName_c,				//服务名
											ulLoadOrder_c,				//加载顺序
											pDrvInfo[i].pszFileCorp		//驱动文件公司
											);
					//清空服务名
					ZeroMemory(drvSrvName_c, sizeof(drvSrvName_c));
				}
			}
		}
	}
	else
		showErrBox();

	//格式化字符串
	sprintf(drvNum_s, "驱动： %d,  隐藏驱动： %d,  可疑PE映像： %d", drvCount, hided, suspectImages);
	//设置状态栏
	m_DrvMgrDlg.m_DrvModList.GetParent()->SetDlgItemText(IDC_STATIC_DRVMOD, drvNum_s);

	free(pDrvSrvName);
	free(pDrvInfo);
	return drvCount;
}

int CDrvModMgr::ReadySrvName(PDRV_SRVNAME pDrvSrvName)
{
	SC_HANDLE hScm;			//SCM句柄
	SC_HANDLE hSrv;			//Srv句柄
	ULONG	bytesNeeded=0;	//返回尺寸
	ULONG	srvNum=0;		//服务个数
	LPENUM_SERVICE_STATUS srvStatus;	//状态缓冲区
	LPQUERY_SERVICE_CONFIG pSrvConfig=NULL;	//服务配置

	char	fixSrvPath_c[MAXPATHLEN];		//路径

	//获取SCM句柄
	hScm=OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	
	//获取驱动服务缓冲区大小
	EnumServicesStatus(hScm, SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &srvNum, NULL);
	
	//申请缓冲区
	srvStatus=(LPENUM_SERVICE_STATUS)malloc(bytesNeeded);

	//正式枚举
    if( !EnumServicesStatus(hScm, SERVICE_DRIVER, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUS)srvStatus, bytesNeeded, &bytesNeeded, &srvNum, NULL) )
		return 0;

	//遍历查询所有服务并保存信息
	for (ULONG i=0; i<srvNum; i++)
	{
		//打开服务获取srv句柄
		hSrv=OpenService(hScm, srvStatus[i].lpServiceName, SERVICE_QUERY_CONFIG);
		if(hSrv==NULL)
			continue;

		//由服务句柄查询 服务的 启动类型 PE文件路径及参数
		//获取需要的缓冲区长度
		QueryServiceConfig(hSrv, NULL, 0, &bytesNeeded);
		//申请缓冲区
		pSrvConfig=(LPQUERY_SERVICE_CONFIG)malloc(bytesNeeded);
		//正式查询
		if( !QueryServiceConfig(hSrv,pSrvConfig, bytesNeeded, &bytesNeeded) )
		{
			free(pSrvConfig);
			continue;
		}

		//服务名
		strcpy(pDrvSrvName[i].szSrvName, srvStatus[i].lpServiceName);

		//转换修正路径
		TranslateFilePathName(pSrvConfig->lpBinaryPathName, fixSrvPath_c);
		//服务路径
		strcpy(pDrvSrvName[i].szSrvPath, fixSrvPath_c);
		//清零临时路径变量
		ZeroMemory(fixSrvPath_c, strlen(fixSrvPath_c));
	}

	return srvNum;
}

void CDrvModMgr::OnRclickDrvmodLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CPoint pt;
	//获取鼠标在ListCtrl的点击位置
	GetCursorPos(&pt);
	CMenu menu;
	//加载文件的右键菜单
	menu.LoadMenu(IDR_DRV_RPOP);
	//获取第1个子菜单
	CMenu *pMenu = menu.GetSubMenu(0);
	//弹出菜单并追踪选中的Item
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
	seledDrv = m_DrvModList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1从第1个item开始搜索行,匹配选中item
	*pResult = 0;
}
				//刷新
void CDrvModMgr::OnRefreshDrvList() 
{
	// TODO: Add your command handler code here
	this->ShowDrvModList();
}
				//复制驱动名
void CDrvModMgr::OnCopyDrvName() 
{
	// TODO: Add your command handler code here
	// 选中item的第1列，即驱动名
	CString drvName=m_DrvModList.GetItemText(seledDrv, 0);
	if ( !drvName.IsEmpty() )
		m_proMgrDlg.SetClipboardText( (LPSTR)(LPCTSTR)drvName );
}
				//复制进程路径
void CDrvModMgr::OnCopyDrvPath() 
{
	// TODO: Add your command handler code here
	// 选中item的第4列，即驱动路径
	CString drvPath=m_DrvModList.GetItemText(seledDrv, 4);
	if ( !drvPath.IsEmpty() )
		m_proMgrDlg.SetClipboardText( (LPSTR)(LPCTSTR)drvPath );
}
				//定位驱动文件
void CDrvModMgr::OnGpsDrvFile() 
{
	// TODO: Add your command handler code here
	// 选中item的第4列，即驱动路径
	CString drvPath=m_DrvModList.GetItemText(seledDrv, 4);
	if ( !drvPath.IsEmpty() )
		WinExec("explorer.exe /e,/select," + drvPath, SW_SHOW);
}

void CDrvModMgr::OnDeleteDrvfile() 
{
	// TODO: Add your command handler code here
	CString	name, strPathNT;
	WCHAR	wfilePath[MAX_PATH];
	PCHAR	szCurDirFile;
	BOOL	success;
	DWORD	bytesRead;

	CString drvPath=m_DrvModList.GetItemText(seledDrv, 4);
	if ( !drvPath.IsEmpty() )
	//拼接成nt路径
	strPathNT = "\\??\\" + drvPath;
	
	szCurDirFile = (LPSTR)(LPCSTR)strPathNT;
	
	ZeroMemory( wfilePath, MAX_PATH );
	
	//获取长度
	UINT nStrULength=MultiByteToWideChar( CP_ACP, 0, szCurDirFile,
												-1, wfilePath, 0 );
	//转换成宽字符串
	MultiByteToWideChar( CP_ACP, 0, szCurDirFile, 
							-1, wfilePath, nStrULength );
	
	success = DeviceIoControl ( g_hDriver, 
								IOCTL_DELETE_FILE,
								wfilePath,
								(MAX_PATH + MAXNAMELEN) * sizeof(WCHAR),
								NULL,
								0,
								&bytesRead,
								NULL );
}
