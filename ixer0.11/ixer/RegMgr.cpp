// RegMgr.cpp : implementation file
//

#include "stdafx.h"
#include <Sddl.h>
#include "Reg.h"
#include "processmgr.h"
#include "ixer.h"
#include "RegMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CixerApp		theApp;
extern CProcessMgr	m_proMgrDlg;
int	iSeledRegItem;			//选中item行

REGMGR	regMgr;

#define MYCOMPUTER "我的电脑"
char currUserSid[256];
/////////////////////////////////////////////////////////////////////////////
// CRegMgr dialog


CRegMgr::CRegMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CRegMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRegMgr)
	//}}AFX_DATA_INIT
}


void CRegMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegMgr)
	DDX_Control(pDX, IDC_REG_LISTS, m_regList);
	DDX_Control(pDX, IDC_REG_TREE, m_regTree);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRegMgr, CDialog)
	//{{AFX_MSG_MAP(CRegMgr)
	ON_NOTIFY(TVN_SELCHANGING, IDC_REG_TREE, OnSelchangingRegTree)
	ON_NOTIFY(NM_RCLICK, IDC_REG_LISTS, OnRclickRegLists)
	ON_COMMAND(IDM_REG_DELETE, OnRegDelete)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_REG_TREE, OnItemexpandingRegTree)
	ON_COMMAND(IDM_REFRESH_REG_LIST, OnRefreshRegList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CRegMgr m_regMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CRegMgr message handlers

BOOL CRegMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//////////////////////////////////////////////////////////////////////////
	//fileMgr->fileTree icon
	treeImages.Create(16, 16, ILC_COLOR8|ILC_MASK, 2, 1);
	//添加icon Map
	treeImages.Add( theApp.LoadIcon(ICO_RCOMPUTER) );
	treeImages.Add( theApp.LoadIcon(ICO_RCLSDFLD) );
	treeImages.Add( theApp.LoadIcon(ICO_ROPENFLD) );
	//file树imageList
	m_regTree.SetImageList(&treeImages, TVSIL_NORMAL);

	//////////////////////////////////////////////////////////////////////////
	//fileMgr->listCtrl icon
	images.Create(16, 16, ILC_COLOR8|ILC_MASK, 2, 1);
	//添加icon Map
	images.Add( theApp.LoadIcon(ICO_SZ) );
	images.Add( theApp.LoadIcon(ICO_DWORD) );
	//listCtrl控件imageList
	m_regList.SetImageList (&images, LVSIL_SMALL);

	//修改listCtrl扩展样式
	DWORD dwStyle = m_regList.GetStyle();				//取得ListCtrl样式
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//修改ListCtrl风格，|WS_VSCROLL|WS_HSCROLL
	m_regList.SetExtendedStyle(dwStyle);		//设置扩展风格
	//插入List头
 	m_regList.SetHeadings( _T("名称, 200;类型, 100;数据, 300") );
 	m_regList.LoadColumnInfo();					//加载列信息

	// TODO: Add extra initialization here
	InitRootNode();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRegMgr::InitRootNode()
{
	HTREEITEM	hItem;
	HTREEITEM	hti;
	TVITEM		tvi;
	
	/* 我的电脑 */
	hItem = m_regTree.InsertItem( "我的电脑", 0, 0, TVI_ROOT, TVI_LAST );

	/* HKEY_CLASSES_ROOT */
	hti = m_regTree.InsertItem( "HKEY_CLASSES_ROOT", 1, 2, hItem, TVI_LAST );
	tvi.mask = TVIF_CHILDREN;
	tvi.hItem = hti;
	tvi.cChildren = 1;
	m_regTree.SetItem(&tvi);

	/* HKEY_CURRENT_USER */
	hti = m_regTree.InsertItem( "HKEY_CURRENT_USER", 1, 2, hItem, TVI_LAST );
	tvi.mask = TVIF_CHILDREN;
	tvi.hItem = hti;
	tvi.cChildren = 1;
	m_regTree.SetItem(&tvi);
	
	/* HKEY_LOCAL_MACHINE */
	hti = m_regTree.InsertItem( "HKEY_LOCAL_MACHINE", 1, 2, hItem, TVI_LAST );
	tvi.mask = TVIF_CHILDREN;
	tvi.hItem = hti;
	tvi.cChildren = 1;
	m_regTree.SetItem(&tvi);
	
	/* HKEY_USERS */
	hti = m_regTree.InsertItem( "HKEY_USERS", 1, 2, hItem, TVI_LAST );
	tvi.mask = TVIF_CHILDREN;
	tvi.hItem = hti;
	tvi.cChildren = 1;
	m_regTree.SetItem(&tvi);
	
	/* HKEY_CURRENT_CONFIG */
	hti = m_regTree.InsertItem( "HKEY_CURRENT_CONFIG", 1, 2, hItem, TVI_LAST );
	tvi.mask = TVIF_CHILDREN;
	tvi.hItem = hti;
	tvi.cChildren = 1;
	m_regTree.SetItem(&tvi);

	hti = m_regTree.GetRootItem();
	m_regTree.SelectItem(hti);
	m_regTree.Expand(hti, TVE_EXPAND);

	//获取当前用户sid
	GetCurrentUserRegPath( currUserSid );
}
				//展开reg树
void CRegMgr::OnItemexpandingRegTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here

	regMgr.m_ClickNode = pNMTreeView->itemNew.hItem;

	//获得要展开的item和其路径
     CString strPathName = GetPathFromItem ( regMgr.m_ClickNode );

	*pResult = FALSE;
	
	if(strPathName == MYCOMPUTER || strPathName == m_LocalPath)
	{
		return;
	}
	
	CWaitCursor wait;
	
	if (pNMTreeView->action == 0)
	{
		return;
	}
	
	if(strPathName != MYCOMPUTER)
	{
		regMgr.fullPath = strPathName;
		//更新item这个节点的子节点
		DeleteChildren ( regMgr.m_ClickNode );
		if ( !AddRegistryNodes( regMgr.m_ClickNode, strPathName) )
		{
			*pResult = TRUE;
		}
	}
	
	m_LocalPath = strPathName;
	
	*pResult = 0;
}

				//选中树的响应
void CRegMgr::OnSelchangingRegTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

UINT CRegMgr::AddRegistryNodes( HTREEITEM hItem, CString &strPathName )
{
	CString		strPath;
	CString		strPathNT;
	CString		str1;
	CString		str2;
	WCHAR		wPath[MAX_PATH + 1];
	PCHAR		szCurDir;
	DWORD		bytesRead;
	BOOL		success;
	ULONG		SubKey[3] = {0};
	CHAR		KeyName[255] = {0};

	//路径名后边没'\'，则附加'\'
    if ( strPathName.Right (1) != "\\" )
	{
        strPathName += "\\";
	}
	
	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return 0;
	}

	str1 = strPathName.Left( strPathName.Find("\\") );
	str2 = strPathName.Mid( strPathName.Find("\\") );

	//等待鼠标图标
	CWaitCursor wait;
	m_regList.DeleteAllItems();

	if (strcmp( str1, "HKEY_CLASSES_ROOT" ) == 0)
	{
		str1 = "Machine\\SOFTWARE\\Classes";
	}
	else if (strcmp( str1, "HKEY_CURRENT_USER" ) == 0)
	{
		char currUserPath[256]="User\\";

		// HKEY_CURRENT_USER路径: "HKEY_USERS\\[当前登录用户SID]"
		strcat( currUserPath, currUserSid );
		str1 = currUserPath;
	}
	else if (strcmp( str1, "HKEY_LOCAL_MACHINE" ) == 0)
	{
		str1 = "Machine";
	}
	else if (strcmp( str1, "HKEY_USERS" ) == 0)
	{
		str1 = "User";
	}
	else if (strcmp( str1, "HKEY_CURRENT_CONFIG" ) == 0)
	{
		str1 = "Machine\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current";
	}

	/* 去除右边'\'字符标记 */
	str2.Delete( str2.GetLength()-1, 1 );
	strPath = str1 + str2;

	strPath = "\\Registry\\" + strPath;
	szCurDir = (LPSTR)(LPCSTR)strPath;

	memset( &wPath[1], 0, MAX_PATH * sizeof(WCHAR) ); 
	UINT nStrULength = MultiByteToWideChar( CP_ACP, 0, szCurDir, -1, &wPath[1], 0 );

	MultiByteToWideChar( CP_ACP, 0, szCurDir, -1, &wPath[1], nStrULength );

	success = DeviceIoControl(
								g_hDriver, 
								IOCTL_GET_REGKEY_SIZE,
								&wPath[1],
								MAX_PATH * sizeof(WCHAR),
								&SubKey,
								3 * sizeof(ULONG),
								&bytesRead,
								NULL);
	if (!success)
	{
		showErrBox();
		return 0;
	}

	if ( SubKey[0] > 0 )
	{
		PREGISTRY_KEY pRegKey = (PREGISTRY_KEY)malloc( SubKey[0] * sizeof(REGISTRY_KEY) );

		/* 截断? */
		wPath[0] = (USHORT)SubKey[0];

		success = DeviceIoControl(
									g_hDriver, 
									IOCTL_GET_REGKEY,
									wPath,
									(MAX_PATH + 1) * sizeof(WCHAR),
									pRegKey,
									SubKey[0] * sizeof(REGISTRY_KEY),
									&bytesRead,
									NULL);
		if (!success)
		{
			return 0;
		}

		for ( ULONG i = 0; i < SubKey[0]; i++ )
		{
			memset( KeyName, 0, 255 );
			UINT nStrLength = WideCharToMultiByte( CP_ACP, 0, pRegKey[i].Name, 
														-1, KeyName, 0, NULL, NULL );

			WideCharToMultiByte( CP_ACP, 0, pRegKey[i].Name, 
									-1, KeyName, nStrLength, NULL, NULL );

			if ( hItem != NULL )
			{
				HTREEITEM	hChild;
				
				hChild = m_regTree.InsertItem( KeyName, 1, 2, hItem, TVI_LAST );
									
				TVITEM tvi;
				tvi.mask = TVIF_CHILDREN;
				tvi.hItem = hChild;
				tvi.cChildren = 1;
				
				m_regTree.SetItem(&tvi);
			}
		}

		free( pRegKey );
	}

		/* regList */
		m_regList.AddItem( 0, "(默认)", "REG_SZ", "(数值未设置)" );

		if ( SubKey[1] > 0 )
		{
			PREGISTRY_VALUE pRegValue = (PREGISTRY_VALUE)malloc( SubKey[1] * sizeof(REGISTRY_VALUE) + SubKey[2] );

			wPath[0] = (USHORT)SubKey[1];

			success = DeviceIoControl(
										g_hDriver, 
										IOCTL_GET_REGVALUE,
										wPath,
										(MAX_PATH + 1) * sizeof(WCHAR),
										pRegValue,
										SubKey[1] * sizeof(REGISTRY_VALUE) + SubKey[2],
										&bytesRead,
										NULL);
			if (!success)
			{
				showErrBox();
				return SubKey[1];
			}

			PREGISTRY_VALUE pRV = pRegValue;

			for ( ULONG i = 0; i < SubKey[1]; i++ )
			{
				WCHAR	wstr[255] = {0};

				memcpy( wstr, pRV->Name, pRV->NameLength );

				memset( KeyName, 0, 255 );
				UINT nStrLength = WideCharToMultiByte( CP_ACP, 0, wstr, 
								-1, KeyName, 0, NULL, NULL );

				WideCharToMultiByte( CP_ACP, 0, wstr, 
						-1, KeyName, nStrLength, NULL, NULL );

				UINT	uimage;
				CHAR	szType[40] = {0};
				LPSTR	lpValue;

				switch ( pRV->Type )
				{
				case REG_BINARY:
					uimage = 1;
					strcpy( szType, "REG_BINARY" );
					break;
				case REG_DWORD:
					uimage = 1;
					strcpy( szType, "REG_DWORD" );
					break;
				case REG_FULL_RESOURCE_DESCRIPTOR:
					uimage = 1;
					strcpy( szType, "REG_FULL_RESOURCE_DESCRIPTOR" );
					break;
				case REG_RESOURCE_LIST:
					uimage = 1;
					strcpy( szType, "REG_RESOURCE_LIST" );
					break;
				case REG_RESOURCE_REQUIREMENTS_LIST:
					uimage = 1;
					strcpy( szType, "REG_RESOURCE_REQUIREMENTS_LIST" );
					break;
				case REG_NONE:
					uimage = 1;
					strcpy( szType, "REG_NONE" );
					break;
				case REG_SZ:
					uimage = 0;
					strcpy( szType, "REG_SZ" );
					break;
				case REG_MULTI_SZ:
					uimage = 0;
					strcpy( szType, "REG_MULTI_SZ" );
					break;
				case REG_EXPAND_SZ:
					uimage = 0;
					strcpy( szType, "REG_EXPAND_SZ" );
					break;
				default:
					uimage = 1;
					sprintf( szType, "0x%02x", pRV->Type );
					break;
				}

				if ( pRV->Type == REG_DWORD )
				{
					lpValue = (LPSTR)malloc( 128 );
					memset( lpValue, 0, 128 );
					sprintf( lpValue, "0x%08x(%d)", *((PULONG)((ULONG)pRV + pRV->DataOffset)), *((PULONG)((ULONG)pRV + pRV->DataOffset)) );
				}
				else if ( uimage == 0 )	//REG_xxSZ
				{
					LPWSTR lpWstr;

					lpWstr = (LPWSTR)malloc( pRV->DataLength + sizeof(WCHAR) );
					lpValue = (LPSTR)malloc( pRV->DataLength + 1 );
					memset( lpWstr, 0, pRV->DataLength + sizeof(WCHAR) );
					memset( lpValue, 0, pRV->DataLength + 1 );

					memcpy( lpWstr, (PVOID)((ULONG)pRV + pRV->DataOffset), pRV->DataLength );

					for ( ULONG i = 0; i < (pRV->DataLength / 2); i++ )
					{
						if ( lpWstr[i] == '\0' )
						{
							lpWstr[i] = L' ';
						}
					}

					UINT nStrLength = WideCharToMultiByte( CP_ACP, 0, lpWstr, 
															-1, lpValue, 0, NULL, NULL );

					WideCharToMultiByte( CP_ACP, 0, lpWstr, 
											-1, lpValue, nStrLength, NULL, NULL );

					free( lpWstr );
				}
				else	//REG_BINARY
				{
					lpValue = (LPSTR)malloc( (pRV->DataLength * 3) + 1 );
					memset( lpValue, 0, (pRV->DataLength * 2) + 1 );

					for ( ULONG i = 0; i < pRV->DataLength; i++ )
					{
						CHAR tmp[10] = {0};
						sprintf( tmp, "%02x ", ((PUCHAR)((ULONG)pRV + pRV->DataOffset))[i] );
						strcat( lpValue, tmp );
					}
				}

				if ( KeyName[0] == '\0' )
				{
					m_regList.SetItem( 0, 0, LVIF_IMAGE, NULL, uimage, 0, 0, NULL );
					m_regList.SetItemText( 0, 1, szType );

					if ( pRV->DataLength == 0 )
					{
						if ( uimage == 0 )
						{
							m_regList.SetItemText( 0, 2, "(数值未设置)" );
						}
						else if ( uimage == 1 )
						{
							m_regList.SetItemText( 0, 2, "(长度为零的二进制位值)" );
						}
					}
					else
					{
						m_regList.SetItemText( 0, 2, lpValue );
					}
				}
				else
				{
					m_regList.AddItem( uimage, KeyName, szType, lpValue );
				}

				free( lpValue );

				pRV = (PREGISTRY_VALUE)((ULONG)pRV + sizeof(REGISTRY_VALUE) + pRV->NameLength + pRV->DataLength - 1);
			}

			free( pRegValue );
		}

	return 0;
}
					//
CString CRegMgr::GetPathFromItem( HTREEITEM hItem )
{
    CString strPathName;
    while (hItem != NULL) 
	{
		CString string = m_regTree.GetItemText (hItem);
        if ((string.Right (1) != "\\") && !strPathName.IsEmpty ())
		{
			string += "\\";
		}
		strPathName = string + strPathName;
        hItem = m_regTree.GetParentItem (hItem);
    }
	
	if(strPathName.Left(8) == MYCOMPUTER && strPathName.GetLength() > 8)
	{
		strPathName = strPathName.Mid(9);
	}
	
	return strPathName;
}
				//
UINT CRegMgr::DeleteChildren(HTREEITEM hItem)
{
    UINT nCount = 0;
    HTREEITEM hChild = m_regTree.GetChildItem (hItem);
	
    while (hChild != NULL) 
	{
        HTREEITEM hNextItem = m_regTree.GetNextSiblingItem (hChild);
        m_regTree.DeleteItem (hChild);
        hChild = hNextItem;
        nCount++;	
    }

    return nCount;
}
				//右键
void CRegMgr::OnRclickRegLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CPoint pt;
	//获取鼠标在ListCtrl的点击位置
	GetCursorPos(&pt);
	CMenu menu;
	//加载文件的右键菜单
	menu.LoadMenu(IDR_REG_RPOP);
	//获取第一个子菜单
	CMenu *pMenu = menu.GetSubMenu(0);
	//弹出菜单并追踪选中的Item
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
	iSeledRegItem = m_regList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1从第1个item开始搜索行,匹配选中item

	*pResult = 0;
}
				//刷新
void CRegMgr::OnRefreshRegList() 
{
	// TODO: Add your command handler code here
	AddRegistryNodes( NULL, regMgr.fullPath );
}

				//删除键值
void CRegMgr::OnRegDelete() 
{
	// TODO: Add your command handler code here;
	CString		strPath;
	CString		str1;
	CString		str2;
	PCHAR		szCurDir;
	BOOL		success;
	ULONG		bytesRead;
	VALUE_KEY	ValueKey;

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return;
	}

	CString strName = m_regList.GetItemText(iSeledRegItem, 0);

	if (strName != "")
	{
		if ( strName.Find("默认") > 0 )
			return;

		if ( regMgr.fullPath.IsEmpty() )
			return;
	}
	
	//路径名后边没'\'，则附加'\'
    if (regMgr.fullPath.Right (1) != "\\")
	{
        regMgr.fullPath += "\\";
	}

	str1 = regMgr.fullPath.Left( regMgr.fullPath.Find("\\") );
	str2 = regMgr.fullPath.Mid( regMgr.fullPath.Find("\\") );

	//等待鼠标图标
	CWaitCursor wait;
	
	if (strcmp( str1, "HKEY_CLASSES_ROOT" ) == 0)
	{
		str1 = "Machine\\SOFTWARE\\Classes";
	}
	else if (strcmp( str1, "HKEY_CURRENT_USER" ) == 0)
	{
		char currUserPath[256]="User\\";
		
		// HKEY_CURRENT_USER路径: "HKEY_USERS\\[当前登录用户SID]"
		strcat( currUserPath, currUserSid );
		str1 = currUserPath;
	}
	else if (strcmp( str1, "HKEY_LOCAL_MACHINE" ) == 0)
	{
		str1 = "Machine";
	}
	else if (strcmp( str1, "HKEY_USERS" ) == 0)
	{
		str1 = "User";
	}
	else if (strcmp( str1, "HKEY_CURRENT_CONFIG" ) == 0)
	{
		str1 = "Machine\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current";
	}
	
	/* 去除右边'\'字符标记 */
	str2.Delete( str2.GetLength()-1, 1 );
	strPath = str1 + str2;
	
	strPath = "\\Registry\\" + strPath;
	szCurDir = (LPSTR)(LPCSTR)strPath;
	
	memset( &ValueKey, 0, sizeof(VALUE_KEY) );
	UINT nStrULength = MultiByteToWideChar( CP_ACP, 0, szCurDir, -1, ValueKey.Key, 0 );
	
	MultiByteToWideChar( CP_ACP, 0, szCurDir, -1, ValueKey.Key, nStrULength );

	szCurDir = (LPSTR)(LPCSTR)strName;

	nStrULength = MultiByteToWideChar( CP_ACP, 0, szCurDir, 
										-1, ValueKey.Value, 0 );
	
	MultiByteToWideChar( CP_ACP, 0, szCurDir, 
							-1, ValueKey.Value, nStrULength );
	
	success = DeviceIoControl(
								g_hDriver, 
								IOCTL_DELETE_REGVALUE,
								&ValueKey,
								sizeof(VALUE_KEY),
								NULL,
								0,
								&bytesRead,
								NULL  );
	if (success)
	{
		m_regList.DeleteItem(iSeledRegItem);
	}
}

void CRegMgr::TranslateRegPath(CString regPath, PWCHAR krnlPath)
{
	CString		strkeyRoot, strsubPath, strRegPath;
	PCHAR		pwsPath;

	//路径名后边没'\'，则附加'\'标记
	if (regPath.Right (1) != "\\")
		regPath += "\\";

	strkeyRoot = regPath.Left( regPath.Find("\\") );
	strsubPath = regPath.Mid( regPath.Find("\\") );

	if (strcmp( regPath, "HKEY_CLASSES_ROOT" ) == 0)
	{
		strkeyRoot = "Machine\\SOFTWARE\\Classes";
	}
	else if (strcmp( strkeyRoot, "HKEY_CURRENT_USER" ) == 0)
	{
		char currUserPath[256]="User\\";
		
		// HKEY_CURRENT_USER路径: "HKEY_USERS\\[当前登录用户SID]"
		strcat( currUserPath, currUserSid );
		strkeyRoot = currUserPath;
	}
	else if (strcmp( strkeyRoot, "HKEY_LOCAL_MACHINE" ) == 0)
	{
		strkeyRoot = "Machine";
	}
	else if (strcmp( strkeyRoot, "HKEY_USERS" ) == 0)
	{
		strkeyRoot = "User";
	}
	else if (strcmp( strkeyRoot, "HKEY_CURRENT_CONFIG" ) == 0)
	{
		strkeyRoot = "Machine\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current";
	}
	
	strRegPath = strkeyRoot + strsubPath;
	
	strRegPath = "\\Registry\\" + strRegPath;
	pwsPath = (LPSTR)(LPCSTR)strRegPath;
	
	UINT nStrULength = MultiByteToWideChar( CP_ACP, 0, pwsPath, -1, krnlPath, 0 );

	MultiByteToWideChar( CP_ACP, 0, pwsPath, -1, krnlPath, nStrULength );
}

BOOL CRegMgr::GetCurrentUserRegPath( char *strCurrUserPath )
{
    ULONG	ulRet;
	HKEY	hKey, hSubKey;
	ULONG   numValues, maxLenValueName, maxLenValueData, lpType, lenValue;
	char	ValueName[1024];				//存储得到的键值名
	BYTE	*lpData=new  BYTE[1024];		//存储得到的键值数据
	ULONG	lenData;						//存储键值的数据长度
	char	subKeyName[256];
	ULONG	numSubKey=0;
	ULONG	sizeSubKey;
	PSID	pstrSid=NULL;

	ZeroMemory( strCurrUserPath, sizeof(strCurrUserPath) );

	ulRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList", NULL, KEY_READ, &hKey);
	if ( ulRet != ERROR_SUCCESS )
		return FALSE;

	ulRet = RegQueryInfoKey( hKey, NULL, NULL, 0, &numSubKey, NULL, NULL, NULL, &maxLenValueName, &maxLenValueData, NULL, NULL );
	if ( ulRet != ERROR_SUCCESS )
		return FALSE;

	/* 遍历sid键 */
	for (int i=0; ulRet==ERROR_SUCCESS, i<(int)numSubKey; i++)
	{
		subKeyName[0] = '\0';				//清空子键
		sizeSubKey=MAX_LEN_KEYNAME;			//初始化缓冲区大小

		ulRet = RegEnumKeyEx( hKey, i, subKeyName, &sizeSubKey, NULL, NULL, NULL, NULL );
		/* 过滤掉 特征码->小于8位 or 超过最大尺寸 的sid */
		if ( sizeSubKey<= strlen( "S-1-5-20" ) || sizeSubKey >= SECURITY_MAX_SID_SIZE)
			continue;

		/* 打开sid键 */
		ulRet = RegOpenKeyEx( hKey, subKeyName, NULL, KEY_READ, &hSubKey );
		if ( ulRet != ERROR_SUCCESS )
			return FALSE;

		/* 查询sid键 */
		ulRet = RegQueryInfoKey( hSubKey, NULL, NULL, 0, &numSubKey, NULL, NULL, &numValues, &maxLenValueName, &maxLenValueData, NULL, NULL );
		if ( ulRet != ERROR_SUCCESS )
			return FALSE;

		for (int keyVal=0; ulRet==ERROR_SUCCESS, keyVal<(int)numValues; keyVal++)
		{
			lenValue=1024;
			lenData=1024;

			//清空键名、键值
			ZeroMemory( ValueName, sizeof(ValueName) );
			ZeroMemory( lpData, sizeof(lpData) );

			/* 查询sid键值 */
			ulRet = RegEnumValue( hSubKey, keyVal, ValueName, &lenValue, NULL, &lpType, lpData, &lenData );

			//判断键名为"RefCount"、类型为REG_DWORD、且"RefCount"键值lpData大于0
			if ( strcmp( ValueName, "RefCount" )==0 && lpType==REG_DWORD && lpData>0 )
			{
				//再次判断是否是有效的sid
				if ( ConvertStringSidToSid( subKeyName, &pstrSid ) && IsValidSid(pstrSid) )
				{
					//获取sid成功，拷贝
					strcpy( strCurrUserPath, subKeyName );
					break;
				}
			}
		}
	}

	delete   lpData;
    return TRUE;
}
