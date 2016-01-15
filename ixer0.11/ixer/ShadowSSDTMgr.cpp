// ShadowSSDTMgr.cpp : implementation file
//

#include "stdafx.h"
#include "SSDT.h"
#include "Memory.h"
#include "OS.h"
#include "Module.h"
#include "ixer.h"
#include "common.h"
#include "ShadowSSDTMgr.h"
#include "SSDTMgr.h"
#include <Dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "Imagehlp.lib")


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



extern "C" {
	PLOADED_IMAGE
		IMAGEAPI
		ImageLoad(
		PCSTR DllName,
		PCSTR DllPath
		);

	BOOL 
		IMAGEAPI 
		ImageUnload(
		PLOADED_IMAGE LoadedImage  
		);
}


extern	HANDLE	g_ixerhProc;

DWORD		g_W32pServiceTable = 0;
SHADOWFUNC	g_ShadowFunc[667] = {0};
/////////////////////////////////////////////////////////////////////////////
// CShadowSSDTMgr dialog


CShadowSSDTMgr::CShadowSSDTMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CShadowSSDTMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CShadowSSDTMgr)
	//}}AFX_DATA_INIT
//	pShadowData=GetShadowSSDTNativeAddr(&numOfNativeAddr);
}


void CShadowSSDTMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CShadowSSDTMgr)
	DDX_Control(pDX, IDC_STATIC_SHADOWSSDT, m_statusShadow);
	DDX_Control(pDX, IDC_SHADOWSSDT_LISTS, m_shadowList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CShadowSSDTMgr, CDialog)
	//{{AFX_MSG_MAP(CShadowSSDTMgr)
	ON_NOTIFY(NM_RCLICK, IDC_SHADOWSSDT_LISTS, OnRclickShadowssdtLists)
	ON_COMMAND(IDM_REFRESH_SHADOW_LIST, OnRefreshShadowList)
	ON_COMMAND(IDM_SHADOW_RESTORE, OnShadowRestore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CShadowSSDTMgr m_shadowMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CShadowSSDTMgr message handlers

BOOL CShadowSSDTMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//插入ListHead
	m_shadowList.SetHeadings(_T("序号,  50;函数名,  150;当前地址,  90;钩子类型,  85;原始函数地址,  90;当前函数所在模块,  300"));
	m_shadowList.LoadColumnInfo();	//加载列信息
	//修改ListCtrl扩展样式
	DWORD dwStyle = m_shadowList.GetStyle();		//取得ListCtrl样式
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//修改ListCtrl风格，|WS_VSCROLL|WS_HSCROLL
	m_shadowList.SetExtendedStyle(dwStyle);		//重设风格
	// TODO: Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CShadowSSDTMgr::ShowShadowSSDTList()
{
	GetCurrentShadowSSDTList(&m_shadowList);
}

int CShadowSSDTMgr::GetCurrentShadowSSDTList(CSortListCtrl *m_shadowList)
{
	ULONG				numOfShadow=0;
	unsigned long int	bytesReturned;		//返回字节数
	PSHADOW_INFO	pShadowInfo=NULL;
	BOOL		b_Ret;
	bool		bFlag=false;
	DWORD		numOfSrv=0;			//实际返回的服务函数个数
	DWORD		iCount	=0;
	char		fixShadowModPath_c[MAXPATHLEN];
	char		shadowNum_s[128];
	int			hooked=0;

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return 0;
	}

	//获取ShadowSSDT信息大小
	b_Ret = DeviceIoControl(g_hDriver,
							(DWORD)IOCTL_GET_SHADOWSSDT_INFO,
							NULL,
							0,
							&numOfSrv,
							sizeof(DWORD),
							&bytesReturned,
							NULL);
	if (!b_Ret)
		return 0;

	//申请堆内存
	pShadowInfo=(PSHADOW_INFO)AllocHeapMem( numOfSrv*sizeof(SHADOW_INFO) );

	//通过symbol获取ShadowSSDT的原始函数地址、函数名
	b_Ret = GetShadowSSDTNativeAddrBySymbol( );
	
	//请求ShadowSSDT信息数据
	b_Ret = DeviceIoControl(g_hDriver,
							(DWORD)IOCTL_GET_SHADOWSSDT_INFO,
							NULL,
							0,
							pShadowInfo,
							numOfSrv*sizeof(SHADOW_INFO),
							&bytesReturned,
							NULL);


	char	index_c[16];
	char	currentAddr_c[16];
	char	nativeAddr_c[16];
	char	hookstype[15] = "\0";

	//清空ListCtrl列表
	m_shadowList->DeleteAllItems();
	
	for (ULONG i = 0; i < numOfSrv; i++)
	{
		sprintf( index_c, "%d", i );
		sprintf( currentAddr_c, "0x%08X", pShadowInfo[i].currentAddr );
		sprintf( nativeAddr_c, "0x%08X", g_ShadowFunc[i].nativeAddr );

		//当前地址 不等于 原始地址，则为hooked！
		if (pShadowInfo[i].currentAddr != g_ShadowFunc[i].nativeAddr && g_ShadowFunc[i].nativeAddr != 0)
		{
			strcpy(hookstype, " ssdt hook");
			hooked++;
		}
		else
			strcpy(hookstype, "     -     ");

		//修正路径
		TranslateFilePathName( pShadowInfo[i].imagePath, fixShadowModPath_c );
		
		m_shadowList->AddItem(
								NULL,						//图标index
								index_c,					//序号
								g_ShadowFunc[i].funcName,	//函数名
								currentAddr_c,				//当前地址
								hookstype,					//钩子类型
								nativeAddr_c,				//原始地址
								fixShadowModPath_c);		//当前ssdt函数地址所在内核模块路径
		}

	//格式化模块窗口标题字符串
	sprintf(shadowNum_s, "ShadowSSDT函数： %d,  挂钩函数： %d", numOfSrv, hooked);	//钩子函数：%d
	//状态回显
	m_shadowMgrDlg.SetDlgItemText(IDC_STATIC_SHADOWSSDT, shadowNum_s);
	FreeHeapMem(pShadowInfo);
	return	numOfSrv;
}

void CShadowSSDTMgr::OnRclickShadowssdtLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		//选中item行
		shadowSeled=0;
		CPoint pt;
		//获取鼠标在ListCtrl的点击位置
		GetCursorPos(&pt);
		CMenu menu;
		//加载进程的右键菜单
		menu.LoadMenu(IDR_SHADOW_RPOP);
		//获取第一个子菜单
		CMenu *pMenu = menu.GetSubMenu(0);
		//弹出菜单并追踪选中的Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		shadowSeled = m_shadowList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		// -1从第1个item开始搜索行,匹配选中item
	}
	*pResult = 0;
}

void CShadowSSDTMgr::OnRefreshShadowList() 
{
	// TODO: Add your command handler code here
	ShowShadowSSDTList();
}

void CShadowSSDTMgr::OnShadowRestore() 
{
	// TODO: Add your command handler code here
	//发送ShadowSsdt服务函数索引、原始地址给驱动
	CString	strIndex;
	CString strOriginAddr;
	CString strCurrentAddr;
	
	CHAR	*wChr={'\0'};
	ULONG	dwSrvIndex;			//index
	ULONG	dwCurrentAddr;		//当前地址
	ULONG	dwOriginAddr;		//原始地址
	
	// 选中item的第1列，即ssdt服务函数索引
	strIndex = m_shadowList.GetItemText(shadowSeled, 0);
	wChr = strIndex.GetBuffer( strIndex.GetLength() );
	//转换并保存选中item的ssdt服务函数索引
	dwSrvIndex = atol(wChr);
	
	// 选中item的第3列，即"ssdt当前地址"
	strCurrentAddr = m_shadowList.GetItemText(shadowSeled, 2);
	// 转换并保存
	dwCurrentAddr = strtoul( (LPCTSTR)strCurrentAddr, '\0', 16);
	
	// 选中item的第5列，即"ssdt原始地址"
	strOriginAddr = m_shadowList.GetItemText(shadowSeled, 4);
	// 转换并保存
	dwOriginAddr = strtoul( (LPCTSTR)strOriginAddr, '\0', 16);

	//当前地址==原始地址，则直接返回
	if (dwCurrentAddr == dwOriginAddr)
		return;
	
	//驱动设备句柄无效也返回
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;

	SSDTINFO	sinfo = {0, 0};
	unsigned long int retBytes;

	sinfo.index	  = dwSrvIndex;		//index
	sinfo.funAddr = dwOriginAddr;	//原始地址
	
	//把ssdt“服务函数索引、原始地址”发送到驱动设备
	DeviceIoControl( g_hDriver,
					(DWORD)IOCTL_RESTORE_SHADOW,
					&sinfo,
					sizeof(SSDTINFO),
					NULL,
					0,
					&retBytes,
					NULL);

	ShowShadowSSDTList();
}

BOOL CShadowSSDTMgr::GetShadowSSDTNativeAddrBySymbol()
{
	BOOL			boRetn;
	PDWORD			pW32pServiceTable = 0;
	DWORD			dwload = 0;
	PLOADED_IMAGE	ploadImage = {0};
	char			imgPath[MAX_PATH];

	// 初始化
	if ( !InitSymbolsHandler() )
	{
		return FALSE;
	}

	// 获取系统目录"C:\WINDOWS\system32"
	GetSystemDirectory( imgPath, MAX_PATH );
	// 加载模块
	ploadImage = ImageLoad( "win32k.sys", imgPath );

	// 加载符号"C:\WINDOWS\system32\win32k.sys"
	strcat( imgPath, "\\win32k.sys" );
	dwload=SymLoadModule ( g_ixerhProc, ploadImage->hFile,
									imgPath, "win32k.pdb",
									0, ploadImage->SizeOfImage );

	boRetn=SymEnumSymbols( g_ixerhProc, dwload, NULL, (PSYM_ENUMERATESYMBOLS_CALLBACK)EnumSymRoutine, NULL );
	if (g_W32pServiceTable)
	{
		pW32pServiceTable = (PDWORD)( g_W32pServiceTable - dwload + (DWORD)ploadImage->MappedAddress );
		boRetn=SymEnumSymbols( g_ixerhProc, dwload, NULL, (PSYM_ENUMERATESYMBOLS_CALLBACK)EnumSymRoutine, pW32pServiceTable );
	}

	ImageUnload(ploadImage);
	SymCleanup(g_ixerhProc);
	return boRetn;
}
