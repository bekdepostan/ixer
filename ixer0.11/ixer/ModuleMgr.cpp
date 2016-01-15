// ModuleMgr.cpp : implementation file
//

#include "stdafx.h"
#include "Module.h"
#include "ixer.h"
#include "ModuleMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma comment (lib, "version.lib")

ULONG	g_baseAddr=0;				//选中item模块的基址
/////////////////////////////////////////////////////////////////////////////
// CModuleMgr dialog


CModuleMgr::CModuleMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CModuleMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CModuleMgr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CModuleMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CModuleMgr)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_MOUDLE_LISTS, m_moduleList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CModuleMgr, CDialog)
	//{{AFX_MSG_MAP(CModuleMgr)
	ON_NOTIFY(NM_RCLICK, IDC_MOUDLE_LISTS, OnRclickMoudleLists)
	ON_COMMAND(IDM_REFRESH_MOD_LIST, OnRefreshModList)
	ON_COMMAND(IDM_MOD_TERMINATOR, OnModTerminator)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CModuleMgr	m_modMgrDlg;
/////////////////////////////////////////////////////////////////////////////
// CModuleMgr message handlers

BOOL CModuleMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	//设置列头信息
	m_moduleList.SetHeadings( _T("模块路径,  220;基址,  80;大小,  80;文件厂商,  150"));		//入口地址,  100;
	//加载列信息
	m_moduleList.LoadColumnInfo();

	//CListCtrl扩展样式改变
	DWORD dwStyle = m_moduleList.GetStyle();	//取得样式
	dwStyle |=  LVS_EX_GRIDLINES				//网格样式
			| LVS_EX_FULLROWSELECT				//整行选中
			| LVS_EX_HEADERDRAGDROP;			//列表头可以拖放

	//设置扩展风格
	m_moduleList.SetExtendedStyle(dwStyle);

	//初始化显示模块List
	ShowModuleList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModuleMgr::ShowModuleList()
{
	unsigned int moduleNums=0;					//模块总数
	char		 proName_Nums[256]="";			//显示模块列表的进程名、模块数

	//获取当前ModuleList并插入进程ListCtrl，返回模块数
	moduleNums=GetCurrentModuleList(this);

	//格式化模块窗口标题字符串
	sprintf(proName_Nums, "[%s]-进程模块(%d)", g_strProcName, moduleNums);
	//设置模块窗口标题
	m_moduleList.GetParent()->SetWindowText(proName_Nums);
}

int CModuleMgr::GetCurrentModuleList(CModuleMgr *pDialog)
{
	ULONG	ifDllNum=MAX_Dll_NUM;
 	BOOL	b_Ret;
	ULONG	dllCount=0;
 	ULONG	numOfDll=0;
 	ULONG	retBytes;
 	PDLL_ITEM pModInfo=NULL;

	//发送进程ID给驱动
	ULONG	dwProId=g_dwPId;
	char	selItemProId[16];				//进程ID(选中item的字符串形式"进程ID")
	ULONG	i=0;
	char	baseAddress_c[16];				//基址
	char	sizeOfImage_c[16];				//映像大小
	char	dllModPath_c[MAXPATHLEN];
	char	fixDllPath_c[MAXPATHLEN];		//路径
 	char	pszDllCorporation[256]="";
	CSortListCtrl *m_ModuleList;			//模块对话框中模块列表控件

	//初始化几个指针 后面要操作他们
	if(pDialog)
	{
		m_ModuleList = &pDialog->m_moduleList;
		sprintf(selItemProId, "%d", g_dwPId);
	}
	else
		return 0;
	
	//清空ListCtrl列表
	m_ModuleList->DeleteAllItems();
	
	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
		return 0;

	ZeroMemory(g_ctrlBuff, MAXBUFFERSIZE);
	memcpy( g_ctrlBuff, &dwProId, sizeof(ULONG) );	//把“进程ID”拷贝到输入缓冲区

	//按个数申请进程结构内存
	pModInfo=(DLL_ITEM *)malloc(sizeof(DLL_ITEM)*ifDllNum*2);
	if (!pModInfo)
	{
		free(pModInfo);
		return dllCount;
	}

	//内存清零
	ZeroMemory(pModInfo, sizeof(DLL_ITEM)*ifDllNum*2);
	//读取数据
	b_Ret=DeviceIoControl(g_hDriver, IOCTL_GET_Dll_LIST, g_ctrlBuff, sizeof(ULONG), pModInfo, sizeof(DLL_ITEM)*ifDllNum*2, &retBytes, NULL);
	if (b_Ret)
	{
		if (retBytes)
		{
			dllCount=retBytes/sizeof(DLL_ITEM);
			for (i=0; i<dllCount; i++)
			{
				if (pModInfo[i].ulBaseAddr != 0  &&  pModInfo[i].ulBaseAddr < 0x80000000)
				{
					//转换模块基址
					sprintf(baseAddress_c, "0x%08X", pModInfo[i].ulBaseAddr);
					//转换模块大小
					sprintf(sizeOfImage_c, "0x%08X", pModInfo[i].ulImageSize);
					//宽字符转多几节
					WideCharToMultiByte(CP_ACP, 0, pModInfo[i].pwszPath, -1, dllModPath_c, sizeof(dllModPath_c), NULL, NULL);
					//修正转换路径
					TranslateFilePathName(dllModPath_c, fixDllPath_c);
					//获取DLL文件公司
					GetFileCorporation(fixDllPath_c, pszDllCorporation);
					
					//加入列表控件 ("序号,40;模块名,100;基址,100;入口,100;进程pid,100;模块路径,250")
					m_ModuleList->AddItem(
										  NULL,					//图标索引号
										  fixDllPath_c,			//模块路径
										  baseAddress_c,		//基址
										  sizeOfImage_c,		//大小
										  pszDllCorporation		//DLL文件公司
										 );
				}
			}
		}
	}
	else
		showErrBox();

	free(pModInfo);
	return dllCount;
}

BOOL CModuleMgr::GetFileCorporation(char *pszFileFullPath, PCHAR pszFileCorporation)
{
	DWORD	dwXHandle=0;
	DWORD	dwXInfoSize;
	LPBYTE	lpXBuffer;
	UINT	uDataSize = 0;
	LPVOID	lpXData=NULL;
	int		iStringLength=0;
	char	textBuffer[256]={'\0'};
	FILE	*hFile;

	//////////////////////////////////////////////////////////////////////////
	dwXInfoSize=GetFileVersionInfoSize(pszFileFullPath, &dwXHandle);
	if (dwXInfoSize != 0)
	{
		lpXBuffer=new BYTE[dwXInfoSize];				//申请版本信息大小的内存
		memset(lpXBuffer, 0, dwXInfoSize*sizeof(BYTE));	//清零，用于保存版本信息
		//获取文件版本信息
		if (GetFileVersionInfo(pszFileFullPath, dwXHandle, dwXInfoSize, lpXBuffer))
		{
			//文件翻译表信息
			struct LANGANDCODEPAGE 
			{
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			if( !VerQueryValue(lpXBuffer, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &uDataSize) )
				free(lpXBuffer);

			//公司
			wsprintf(textBuffer,TEXT("\\StringFileInfo\\%04x%04x\\CompanyName"),
							lpTranslate[0].wLanguage,
							lpTranslate[0].wCodePage);

			//文件右键的"属性"->"版本"->"公司"
			if( VerQueryValue(lpXBuffer, textBuffer, &lpXData, &uDataSize) )
				strcpy( (CHAR *)pszFileCorporation, LPTSTR(lpXData) );	//拷贝File Corporation
		}
		free(lpXBuffer);
		lpXBuffer=NULL;

		return TRUE;
	}
	else	//获取文件版本信息大小失败，则检测文件是否存在
	{
		//"只读"方式打开文件
		hFile=fopen(pszFileFullPath, "r");
		if ( !hFile )
		{
			//排除"Idle"、"System"2个特殊进程路径
			if (stricmp(pszFileFullPath, "Idle") == 0)
			{
				ZeroMemory( (char *)pszFileCorporation, strlen(pszFileCorporation) );
				return FALSE;
			}
			if (stricmp(pszFileFullPath, "System") == 0)
			{
				ZeroMemory( (char *)pszFileCorporation, strlen(pszFileCorporation) );
				return FALSE;
			}

			strcpy( (CHAR *)pszFileCorporation, "文件不存在" );
			return FALSE;
		}
		else
		{
			ZeroMemory( (char *)pszFileCorporation, strlen(pszFileCorporation) );
			fclose(hFile);
		}
	}

	return FALSE;
}

				//右键
void CModuleMgr::OnRclickMoudleLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
		{
			CString strbaseAddr;
			int		selectedItem=0;			//选中item行
			//获取鼠标在ListCtrl的点击位置
			CPoint pt;
			GetCursorPos(&pt);
			//加载进程的右键菜单
			CMenu menu;
			menu.LoadMenu(IDR_DLL_RPOP);
			//获取第一个子菜单
			CMenu *pMenu = menu.GetSubMenu(0);
			//弹出菜单并追踪选中的Item
			pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
			selectedItem = m_moduleList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1从第1个item开始搜索行,匹配选中item

			// 选中item的第2列，即Dll基址
			strbaseAddr = m_moduleList.GetItemText(selectedItem, 1);
			// 转换并保存选中item模块的基址
			g_baseAddr = strtoul( (LPCTSTR)strbaseAddr, '\0', 16);					//转换字符串为Long，保存到全局变量g_baseAddr													
		}
		*pResult = 0;
}
				//更新模块
void CModuleMgr::OnRefreshModList() 
{
	// TODO: Add your command handler code here
	ShowModuleList();
}

//卸载模块
void CModuleMgr::OnModTerminator() 
{
	// TODO: Add your command handler code here
	//发送模块的PID、基址给驱动
	ULONG	dwProId=g_dwPId;
	ULONG	dwbaseAddr=g_baseAddr;
	DWORD	dw;

	ZeroMemory(g_ctrlBuff, MAXBUFFERSIZE);
	
	//把模块所属“进程ID、基址”拷贝到输入缓冲区
	*(ULONG *)g_ctrlBuff=dwProId;
	*(ULONG *)( (ULONG)g_ctrlBuff + 4 )=dwbaseAddr;

	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//把“进程模块的ID、基址”发送到驱动设备
		DeviceIoControl(g_hDriver, IOCTL_DLL_TERMINATOR, g_ctrlBuff, MAXBUFFERSIZE, g_ctrlBuff, MAXBUFFERSIZE, &dw, 0);
		ShowModuleList();
	}
}



