// FileMgr.cpp : implementation file
//

#include "stdafx.h"
#include "File.h"
#include "ixer.h"
#include "processMgr.h"
#include "NtUnDoc.h"
#include "FileMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ILI_MYCOMP    0
#define ILI_DRIVE     1
#define ILI_CDDRV     2
#define ILI_FLOPPYDRV 3
#define ILI_OPENFLD   4
#define ILI_CLSDFLD   5
#define ILI_TEXTFILE  6

#define MYCOMPUTER "�ҵĵ���"


extern CProcessMgr	m_proMgrDlg;
extern CixerApp		theApp;
int		iSeledItem=0;			//ѡ��item��
/////////////////////////////////////////////////////////////////////////////
// CFileMgr dialog


CFileMgr::CFileMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CFileMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileMgr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CFileMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileMgr)
	DDX_Control(pDX, IDC_STATIC_FILE, m_statusFile);
	DDX_Control(pDX, IDC_FILE_TREE, m_fileTree);
	DDX_Control(pDX, IDC_FILE_LISTS, m_fileList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileMgr, CDialog)
	//{{AFX_MSG_MAP(CFileMgr)
	ON_NOTIFY(NM_RCLICK, IDC_FILE_LISTS, OnRclickFileLists)
	ON_COMMAND(IDM_REFRESH_FLE_LIST, OnRefreshFleList)
	ON_NOTIFY(TVN_ITEMEXPANDED, IDC_FILE_TREE, OnItemexpandedFileTree)
	ON_NOTIFY(TVN_SELCHANGING, IDC_FILE_TREE, OnSelchangingFileTree)
	ON_COMMAND(IDM_DEL_FILE, OnDelFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CFileMgr m_fileMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CFileMgr message handlers

BOOL CFileMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// TODO: Add extra initialization here
	//�޸�listCtrl��ʽ
	m_fileList.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);
	//���imageList
	m_fileList.SetImageList (&m_proMgrDlg.images, LVSIL_SMALL);
	//�ٴ��޸�ListCtrl��չ��ʽ
	DWORD dwStyle;
	dwStyle = m_fileList.GetStyle();									//ȡ��ListCtrl��ʽ
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//�޸�ListCtrl���|WS_VSCROLL|WS_HSCROLL
	m_fileList.SetExtendedStyle(dwStyle);		//������
	//����Listͷ
	m_fileList.SetHeadings( _T("�ļ���,100;ռ�ÿռ�,60;����ʱ��,100;����޸�,100;������,100;�ļ�����,100") ); //������ͷ��Ϣ
	m_fileList.LoadColumnInfo();							//��������Ϣ

	//////////////////////////////////////////////////////////////////////////
	//fileTree icon
	imgfileTree.Create(16, 16, ILC_COLOR8|ILC_MASK, 9, 9);
	imgfileTree.Add( theApp.LoadIcon(ICO_MYCOMPUTER) );
	imgfileTree.Add( theApp.LoadIcon(ICO_DRIVE) );
	imgfileTree.Add( theApp.LoadIcon(ICO_CDDRV) );
	imgfileTree.Add( theApp.LoadIcon(ICO_FLOPPYDRV) );
	imgfileTree.Add( theApp.LoadIcon(ICO_OPENFLD) );
	imgfileTree.Add( theApp.LoadIcon(ICO_CLSDFLD) );

	HTREEITEM hFileParent = m_fileTree.InsertItem(MYCOMPUTER, ILI_MYCOMP, ILI_MYCOMP);
	//���imageList
	m_fileTree.SetImageList(&imgfileTree, TVSIL_NORMAL);

	CString cTmp;
	int		nPos	= 0;
	UINT	nCount	= 0;
	CString	strDrive= "?:\\";
	//��Ӹ��ڵ�
    DWORD dwDriveList = ::GetLogicalDrives ();	
		
    while (dwDriveList) 
	{
        if (dwDriveList & 1) 
		{
			cTmp = strDrive;
            strDrive.SetAt(0, 0x41 + nPos);

            if ( AddDrives(strDrive, hFileParent) )
                nCount++;
        }
		
        dwDriveList >>= 1;
        nPos++;
    }

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFileMgr::AddDrives(CString strDrive, HTREEITEM hParent)
{
	HTREEITEM hItem;
    UINT nType = ::GetDriveType ((LPCTSTR) strDrive);
    UINT nDrive = (UINT) strDrive[0] - 0x41;
	
    switch (nType)
	{
    case DRIVE_REMOVABLE:
		strDrive=strDrive;
		hItem = m_fileTree.InsertItem(strDrive, ILI_FLOPPYDRV, ILI_FLOPPYDRV, hParent);
		if ( HasSubdirectory (strDrive) )
		{
			AddDummyNode(hItem);
		}
        break;

    case DRIVE_FIXED:
		strDrive=strDrive;
		hItem = m_fileTree.InsertItem(strDrive,  ILI_DRIVE, ILI_DRIVE, hParent);
		if ( HasSubdirectory (strDrive) )
		{
			AddDummyNode(hItem);
		}
        break;
		
    case DRIVE_REMOTE:
        hItem = m_fileTree.InsertItem(strDrive, ILI_DRIVE, ILI_DRIVE, hParent);
		if ( HasSubdirectory (strDrive) )
		{
			AddDummyNode(hItem);
		}
        break;
		
    case DRIVE_CDROM:
		strDrive=strDrive;
        hItem = m_fileTree.InsertItem(strDrive, ILI_CDDRV, ILI_CDDRV, hParent);
		if ( HasSubdirectory (strDrive) )
		{
			AddDummyNode(hItem);
		}
        break;
		
    case DRIVE_RAMDISK:
        hItem = m_fileTree.InsertItem(strDrive, ILI_CDDRV, ILI_CDDRV, hParent);
		if ( HasSubdirectory (strDrive) )
		{
			AddDummyNode(hItem);
		}
        break;

    default:
        return FALSE;
	}

	return TRUE;
}

//�ж�һ���ļ������Ƿ������ļ���
BOOL CFileMgr::HasSubdirectory(CString &strPathName)
{
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    BOOL bResult = FALSE;
	
    CString strFileSpec = strPathName;
    if (strFileSpec.Right (1) != "\\")
        strFileSpec += "\\";
    strFileSpec += "*.*";
	
    if ( (hFind = ::FindFirstFile ((LPCTSTR) strFileSpec, &fd)) !=
						INVALID_HANDLE_VALUE ) 
	{
        do 
		{
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
			{
                CString strFileName = (LPCTSTR) &fd.cFileName;
                if ((strFileName != ".") && (strFileName != ".."))
				{
                    bResult = TRUE;
				}
            }
        } while (::FindNextFile (hFind, &fd) && !bResult);
        ::FindClose (hFind);
    }

    return bResult;
}

//��hItem�������һ���յ�item
void CFileMgr::AddDummyNode(HTREEITEM hItem)
{
	m_fileTree.InsertItem ("", 0, 0, hItem);
}

				//ѡ��ı����Ӧ
void CFileMgr::OnSelchangingFileTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	
    CString strPathName = GetPathFromItem(hItem);
	
	*pResult = FALSE;
	
	if(strPathName == MYCOMPUTER)
	{
		return;
	}
	
	if(strPathName==m_LocalPath)
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
		//����item����ڵ���ӽڵ�
		DeleteChildren (hItem);
		if (!AddDirectoryNodes (hItem, strPathName))
		{
			*pResult = TRUE;
		}
	}
	
	m_LocalPath = strPathName;
	*pResult = 0;
}
				//չ���������Ӧ
void CFileMgr::OnItemexpandedFileTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;

	//���Ҫչ����item����·��
    CString strPathName = GetPathFromItem (hItem);
	
	//�ж�·�����ڵ��������Ƿ���Ч
	if ( !IsMediaValid (strPathName) ) 
	{
        HTREEITEM hRoot = GetDriveNode (hItem);
        m_fileTree.Expand (hRoot, TVE_COLLAPSE);
        DeleteChildren (hRoot);
        AddDummyNode (hRoot);
        *pResult = TRUE;
        return;
	} 
	
    //�ж�·���Ƿ���Ч
    if ( !IsPathValid (strPathName) ) 
	{
        if(strPathName != MYCOMPUTER && strPathName != "")
		{
			m_fileTree.DeleteItem (hItem);
			*pResult = TRUE;
			return;
		}
    }
	
	//�ȴ����
	CWaitCursor wait;
    // If the item is expanding, delete the dummy item attached to it
    // and add folder items. If the item is collapsing instead, delete
    // its folder items and add a dummy item if appropriate.
    if (pNMTreeView->action == TVE_EXPAND) 
	{
		if(strPathName != MYCOMPUTER)
		{
			DeleteChildren (hItem);
			if (!AddDirectoryNodes (hItem, strPathName))
			{
				*pResult = TRUE;
			}
		}
    }
    else 
	{
		if(strPathName != MYCOMPUTER)
		{
			DeleteChildren (hItem);
			if (IsDriveNode (hItem))
			{
				AddDummyNode (hItem);
			}
			else
			{
				SetButtonState (hItem, strPathName);
			}
		}
    }
	
	m_LocalPath = strPathName;
	*pResult = 0;
}

CString CFileMgr::GetPathFromItem(HTREEITEM hItem)
{
    CString strPathName;
    while (hItem != NULL) 
	{
		CString string = m_fileTree.GetItemText (hItem);
        if ((string.Right (1) != "\\") && !strPathName.IsEmpty ())
		{
			string += "\\";
		}
		strPathName = string + strPathName;
        hItem = m_fileTree.GetParentItem (hItem);
    }
	
	if(strPathName.Left(8) == MYCOMPUTER && strPathName.GetLength() > 8)
	{
		strPathName = strPathName.Mid(9);
	}
	
	return strPathName;
}

BOOL CFileMgr::IsMediaValid(CString &strPathName)
{
    // Return TRUE if the drive doesn't support removable media.
    UINT nDriveType = GetDriveType ((LPCTSTR) strPathName);
    if ((nDriveType != DRIVE_REMOVABLE) && (nDriveType != DRIVE_CDROM))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

HTREEITEM CFileMgr::GetDriveNode(HTREEITEM hItem)
{
    HTREEITEM hParent;
    do 
	{
        hParent = m_fileTree.GetParentItem (hItem);
        if (hParent != NULL)
		{
            hItem = hParent;
		}
    } while (hParent != NULL);
    return hItem;
}

UINT CFileMgr::DeleteChildren(HTREEITEM hItem)
{
    UINT nCount = 0;
    HTREEITEM hChild = m_fileTree.GetChildItem (hItem);
	
    while (hChild != NULL) 
	{
        HTREEITEM hNextItem = m_fileTree.GetNextSiblingItem (hChild);
        m_fileTree.DeleteItem (hChild);
        hChild = hNextItem;
        nCount++;	
    }
    return nCount;
}

//�ж�һ��item�Ƿ���������item
BOOL CFileMgr::IsDriveNode(HTREEITEM hItem)
{
	return (m_fileTree.GetParentItem (hItem) == NULL) ? TRUE : FALSE;
}

// �ж�һ��·���Ƿ���Ч
BOOL CFileMgr::IsPathValid(CString &strPathName)
{
    if (strPathName.GetLength () == 3)
	{
        return TRUE;
	}
	
    HANDLE hFind;
    WIN32_FIND_DATA fd;
    BOOL bResult = FALSE;
	
    if ((hFind = ::FindFirstFile ((LPCTSTR) strPathName, &fd)) !=
        INVALID_HANDLE_VALUE) 
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			bResult = TRUE;
		}
        ::CloseHandle (hFind);
    }
	
    return bResult;
}

UINT CFileMgr::AddDirectoryNodes(HTREEITEM hItem, CString &strPathName)
{
	DWORD    bytesReturned;
	ULONG   num;
	PDIRECTORY_INFO   temp;
	DIRECTORY_INFO_EX  b;
    CString str,str1,strFileSpec = strPathName;

	//·�������û'\'���򸽼�'\'
    if (strFileSpec.Right (1) != "\\")
	{
        strFileSpec += "\\";
	}

	char  a[MAX_PATH];
	str1=strFileSpec;
	strcpy(a, strFileSpec.GetBuffer( strFileSpec.GetLength() ));
    strFileSpec += "*.*";

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		//MessageBox("����δ����","����",MB_OK|MB_ICONERROR);
		return 0;
	}

	//��ȡ���ļ����µ��ļ��и�����������num�� 
	BOOL bRc=DeviceIoControl(g_hDriver,
							(DWORD)IOCTL_GET_DIR_NUM,
							a,
							sizeof(a),
							&num,
							sizeof(ULONG),
							&bytesReturned,
							NULL);
	if(num==0 || (!bRc))
	{
		return 0;
	}

	//�����ڴ�
	temp=(PDIRECTORY_INFO)calloc(num, sizeof(DIRECTORY_INFO));
	if(temp==NULL)
	{
		return 0;
	}
	//��ȡ�ļ����б���Ϣ����������temp��������
	bRc=DeviceIoControl(g_hDriver,
						(DWORD)IOCTL_GET_DIR_INFO,
						a,
						sizeof(a),
						temp,
						num*sizeof(DIRECTORY_INFO),
						&bytesReturned,
						NULL);	
	if(!bRc)
	{
		delete temp;
		return 0;
	}
	
	//�ȴ����ͼ��
	CWaitCursor wait;
	//������������е�item
	m_fileMgrDlg.DeleteAllItems();
	//����·��
	m_fileMgrDlg.SetPath(str1);
	//��ʼ����
    for(ULONG i=0; i<num; i++)
	{
		//�ļ�·��
		str.Format("%s",temp[i].FileName);
		str=str1 + str;

		//������ļ���
		if(temp[i].FileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			//���´���NULL�����ø���filetree״̬
			if (hItem==NULL)
			{
				//��������ѭ��
				continue;
			}

			//����Ŀ¼�ṹ������Ŀ¼����Ϊ:'.'��'..'����
			if(strcmp(temp[i].FileName,"."))
			{
				if(strcmp(temp[i].FileName,".."))
				{
					//����filetree�ؼ�
					HTREEITEM hChild=m_fileTree.InsertItem (
											(LPCTSTR)temp[i].FileName,	//&fd.cFileName,
												ILI_CLSDFLD , ILI_OPENFLD , hItem , TVI_SORT );
					
					CString strNewPathName = strPathName;
					if (strNewPathName.Right (1) != "\\")
						strNewPathName += "\\";
					
					strNewPathName += (LPCTSTR) temp[i].FileName; //fd.cFileName;
					//����filetree״̬
					SetButtonState (hChild, strNewPathName);	
				}
			}
		}
		else	//���ļ�����
		{
			b.DirectoryInfo=temp[i];
			b.path=str1;
			AddToListView(&b);
		}
    } 
	delete temp;
    return num;
}

void CFileMgr::DeleteAllItems()
{
    m_fileList.DeleteAllItems();
	index=0;
	FileNum=0;
}

void CFileMgr::SetPath(CString str)
{
    g_currPath=str;
}

void CFileMgr::SetButtonState(HTREEITEM hItem, CString &strPathName)
{
    if (HasSubdirectory (strPathName))
	{
        AddDummyNode (hItem);
	}
}

void CFileMgr::AddToListView( PDIRECTORY_INFO_EX pfileDir )
{
	char fileAttributeStr[256]={0};//�ļ������ַ���
	CString str; //�ļ�·�� �����õ��ļ�ͼ��
	char createTimeStr[16];//����ʱ���ַ���
	char lastWriteTimeStr[16];//����޸�ʱ��
	char lastAccessTimeStr[16];//������ʱ��
	char allocationSizeStr[16];//ռ�ÿռ�
	
	//�ļ�����������
	FileNum++;
	
	//��ȡ�ļ�·�� �����õ��ļ�ͼ��
	str.Format("%s",pfileDir->DirectoryInfo.FileName);
	str=pfileDir->path+str;
	
	//��ȡ�ļ�����ʱ���ַ���
	sprintf(createTimeStr,"%d-%d-%d",pfileDir->DirectoryInfo.CreationTime.Year,pfileDir->DirectoryInfo.CreationTime.Month,pfileDir->DirectoryInfo.CreationTime.Day);
	
	//��ȡ�ļ�ռ�ÿռ��ַ���
	if(pfileDir->DirectoryInfo.AllocationSize.QuadPart>1024*1024*1024) 
		sprintf(allocationSizeStr, "%0.2fG",(float)(pfileDir->DirectoryInfo.AllocationSize.QuadPart)/(float)(1024*1024*1024)); 
	else if(pfileDir->DirectoryInfo.AllocationSize.QuadPart>1024*1024)
		sprintf(allocationSizeStr, "%0.2fM",(float)(pfileDir->DirectoryInfo.AllocationSize.QuadPart)/(float)(1024*1024));
	else if(pfileDir->DirectoryInfo.AllocationSize.QuadPart>1024)
		sprintf(allocationSizeStr, "%0.2fK",(float)(pfileDir->DirectoryInfo.AllocationSize.QuadPart)/(float)(1024));
	else 
		sprintf(allocationSizeStr,"%ldB",pfileDir->DirectoryInfo.AllocationSize.QuadPart);
	
	//��ȡ�ļ�����޸�ʱ���ַ���
	sprintf(lastWriteTimeStr,"%d-%d-%d",pfileDir->DirectoryInfo.LastWriteTime.Year,pfileDir->DirectoryInfo.LastWriteTime.Month,pfileDir->DirectoryInfo.LastWriteTime.Day);
	
	//��ȡ�ļ�������ʱ���ַ���
	sprintf(lastAccessTimeStr,"%d-%d-%d",pfileDir->DirectoryInfo.LastAccessTime.Year,pfileDir->DirectoryInfo.LastAccessTime.Month,pfileDir->DirectoryInfo.LastAccessTime.Day);
	
	//��ȡ�ļ������ַ���
	if(pfileDir->DirectoryInfo.FileAttributes & FILE_ATTRIBUTE_READONLY)
		strcat(fileAttributeStr,"ֻ�� ");
	if(pfileDir->DirectoryInfo.FileAttributes & FILE_ATTRIBUTE_HIDDEN)
		strcat(fileAttributeStr,"���� ");
	if(pfileDir->DirectoryInfo.FileAttributes & FILE_ATTRIBUTE_SYSTEM)
		strcat(fileAttributeStr,"ϵͳ ");
	if(pfileDir->DirectoryInfo.FileAttributes & FILE_ATTRIBUTE_ARCHIVE)
		strcat(fileAttributeStr,"�浵 ");
	if(pfileDir->DirectoryInfo.FileAttributes & FILE_ATTRIBUTE_NORMAL)
		strcat(fileAttributeStr,"���� ");
	if(pfileDir->DirectoryInfo.FileAttributes & FILE_ATTRIBUTE_TEMPORARY)
		strcat(fileAttributeStr,"��ʱ ");
	if(pfileDir->DirectoryInfo.FileAttributes & FILE_ATTRIBUTE_COMPRESSED)
		strcat(fileAttributeStr,"ѹ�� ");  
	
	//��ȡͼ��
	int nIconIndex=m_proMgrDlg.GetIconIndex( str.GetBuffer(str.GetLength()) ); 
	
	//�����б�ؼ�
	m_fileList.AddItem( nIconIndex,							//ͼ���
						pfileDir->DirectoryInfo.FileName,	//�ļ���
						allocationSizeStr,					//ռ�ÿռ�
						createTimeStr,						//����ʱ��
						lastWriteTimeStr,					//����޸�ʱ��
						lastAccessTimeStr,					//������ʱ��
						fileAttributeStr );					//����
}

void CFileMgr::OnRclickFileLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		//��ȡ�����ListCtrl�ĵ��λ��
		CPoint pt;
		GetCursorPos(&pt);
		//�����ļ����Ҽ��˵�
		CMenu menu;
		menu.LoadMenu(IDR_FILE_RPOP);
		//��ȡ��һ���Ӳ˵�
		CMenu *pMenu = menu.GetSubMenu(0);
		//�����˵���׷��ѡ�е�Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		iSeledItem = m_fileList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item
	}
	*pResult = 0;
}

void CFileMgr::OnRefreshFleList() 
{
	// TODO: Add your command handler code here
	AddDirectoryNodes( NULL, g_currPath);
}

			//ɾ���ļ�
void CFileMgr::OnDelFile() 
{
	// TODO: Add your command handler code here
	CString	name, strPathNT;
	char	fileName[MAX_PATH];
	WCHAR	wfilePath[MAX_PATH];
	PCHAR	szCurDirFile;
	BOOL	success;
	DWORD	bytesRead;
	
	// ѡ��item�ĵ�1�У����ļ���
	m_fileList.GetItemText(iSeledItem, 0, fileName, 255);
	name.Format("%s", fileName);
	//����·��
	name = g_currPath + fileName;
	//ƴ�ӳ�nt·��
	strPathNT = "\\??\\" + name;
	
	szCurDirFile = (LPSTR)(LPCSTR)strPathNT;
	
	ZeroMemory( wfilePath, MAX_PATH );
	
	//��ȡ����
	UINT nStrULength=MultiByteToWideChar( CP_ACP, 0, szCurDirFile,
		-1, wfilePath, 0 );
	//ת���ɿ��ַ���
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
	if (!success)
	{
		return;
	}
	else
	{
		m_fileList.DeleteItem(iSeledItem);
	}
	
}
