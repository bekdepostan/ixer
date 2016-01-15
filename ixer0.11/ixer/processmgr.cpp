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
int		seledProc;				//ѡ�н���item
ULONG	g_eProc;				//ѡ��item�Ľ���eProcess
ULONG	g_dwPId;				//ѡ�н���ID(ѡ��item�Ľ���ID)
CString	g_strProcName;			//ѡ��item�Ľ�����
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
	//����Listͷ
	m_proMgrDlg.m_proList.SetHeadings( _T("ӳ������,  110;����ID,  55;������ID,  65;ӳ��·��,  220;EPROCESS,  75;����״̬,  88;�ļ�����,  142") ); //������ͷ��Ϣ
	m_proMgrDlg.m_proList.LoadColumnInfo();	//��������Ϣ
	//�޸�ListCtrl��չ��ʽ
	DWORD dwStyle = m_proMgrDlg.m_proList.GetStyle();								//ȡ��ListCtrl��ʽ
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//�޸�ListCtrl���|WS_VSCROLL|WS_HSCROLL
	m_proMgrDlg.m_proList.SetExtendedStyle(dwStyle);     //������
	
	//�б����ߵ�ͼ���б�
	m_proMgrDlg.images.Create(16, 16, ILC_COLOR32, 10, 10);
	//����Ĭ��ͼ��Index
//	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_proMgrDlg.nDefaultIconIndex = m_proMgrDlg.images.Add(NULL);	//�հ�icon
	m_proMgrDlg.m_proList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessMgr::ShowProcessList()
{
	//��ȡ��ǰProcessList���������ListCtrl�����ؽ�����
	GetCurrentProcessList(&m_proList);
}

//////////////////////////////////////////////////////////////////////////
//	��ȡ�����б�
int CProcessMgr::GetCurrentProcessList(CSortListCtrl *m_proList)
{
	BOOL	b_Ret;
	ULONG	numOfproc;	//������
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

	//��������Ȩ��
	AdjustPrivileges( SE_DEBUG_NAME );

	//���ListCtrl�б�
	m_proList->DeleteAllItems();
	//���������Ч��Ϊ���򷵻�0
	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
		return 0;

	//ö�ٽ��̲����ظ���
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
			//������������̽ṹ�ڴ�
			pProcInfo=(PROCESS_ITEM *)malloc(sizeof(PROCESS_ITEM)*numOfproc);
			//�ڴ�����
			ZeroMemory(pProcInfo, sizeof(PROCESS_ITEM)*numOfproc);
			//��ȡ����
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
					//��ȡicon����
					iconIndex = GetIconIndex(pProcInfo[i].pszImagePath);

					//����id
					ltoa(pProcInfo[i].ulPid, szPid, 10);					//PIDת�ַ���
					//������idΪ0����"-"
					if (pProcInfo[i].ulPPid)
						ltoa(pProcInfo[i].ulPPid, szPPid, 10);				//PPIDת�ַ���
					else
						strcpy(szPPid, "-" );

					//��ʽ��Ϊ0x��ͷ���ַ���
					sprintf(szEproc, "0x%X", (DWORD)pProcInfo[i].pEproc);

					//��ȡ�ļ���˾��
					m_modMgrDlg.GetFileCorporation(pProcInfo[i].pszImagePath, pProcInfo[i].pszFileCorp);

					//���r3����״̬
					hProc=OpenProcess(PROCESS_ALL_ACCESS, FALSE, pProcInfo[i].ulPid);
					if ( !hProc )
					{
						strcpy( pProcInfo[i].r3Access, "    �ܾ�" );
						r3Access++;

						if (pProcInfo[i].hideFlag)
						{
							strcat(pProcInfo[i].r3Access, " ����");
							hided++;
						}
					}
					else
					{
						strcpy( pProcInfo[i].r3Access, "      -" );

						if (pProcInfo[i].hideFlag)
						{
							strcpy( pProcInfo[i].r3Access, " ����" );
							hided++;
						}

						CloseHandle(hProc);
					}

					//��ӽ����ListCtrl
					m_proList->AddItem( iconIndex,						//icon����
										pProcInfo[i].pszImageName,		//������
										szPid,							//����id
										szPPid,							//������id
										pProcInfo[i].pszImagePath,		//����·��
										szEproc,						//EPROCESS
										pProcInfo[i].r3Access,			//�û������״̬
										pProcInfo[i].pszFileCorp);		//�ļ�����
				}
			}
			free(pProcInfo);
		}
	}
	else
		showErrBox();

	//��ʽ���ַ���
	sprintf(num_s, "���̣� %d,  ���ؽ��̣� %d,  Ӧ�ò�ܾ����ʣ� %d", numOfproc, hided, r3Access);
	//����״̬��
	m_proMgrDlg.m_proList.GetParent()->SetDlgItemText(IDC_STATIC_PROCESS, num_s);
	return numOfproc;
}

int CProcessMgr::GetIconIndex(char *path)
{
	int		nValue;
	BOOL	ret;
	int		nIconIndex;
	//////////////////////////////////////////////////////////////////////////
	//��iconMap��ȡͼ��
	//////////////////////////////////////////////////////////////////////////
	ret = iconMap.Lookup(_T(path), nValue);
	if(ret)	//�Ѵ���
	{
		nIconIndex=nValue;
	}
	else	//û�ҵ��������һ���µ�
	{
		SHFILEINFO sfi;
		ret=SHGetFileInfo(path, 0, &sfi, sizeof(sfi), SHGFI_ICON|SHGFI_SMALLICON|SHGFI_LARGEICON);
		if(ret)
		{
			nIconIndex = images.Add(sfi.hIcon);
			//�����ͼ��·����index����iconMap
			iconMap.SetAt(_T(path), nIconIndex);
		}
		else
		{	//Ĭ��ͼ������
			nIconIndex=nDefaultIconIndex;
		}
	}
	return nIconIndex;
}
			//��Ӧ�Ҽ�ѡ�н���item��Ϣ
void CProcessMgr::OnRclickProcessLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		CString str, strEproc;
		CHAR	*pChr={'\0'};
		seledProc=0;			//ѡ��item
		//��ȡ�����ListCtrl�ĵ��λ��
		CPoint pt;
		GetCursorPos(&pt);
		//���ؽ��̵��Ҽ��˵�
		CMenu menu;
		menu.LoadMenu(IDR_PROC_RPOP);
		//��ȡ��һ���Ӳ˵�
		CMenu *pMenu = menu.GetSubMenu(0);
		//�����˵���׷��ѡ�е�Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		seledProc = m_proList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);	//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item

		/*����*/
		// ѡ��item�ĵ�1�У���������
		g_strProcName = m_proList.GetItemText(seledProc, 0);
		// ѡ��item�ĵ�2�У�������Pid
		str = m_proList.GetItemText(seledProc, 1);
		pChr = str.GetBuffer( str.GetLength() );
		// ת��������
		g_dwPId = atol(pChr);										//ת���ַ���ΪULONG�����浽ȫ�ֱ���g_dwPId

		/*�߳�*/
		// ѡ��item�ĵ�4�У����߳�ETHREAD
		strEproc = m_proList.GetItemText(seledProc, 4);
		// ת��������ѡ��itemģ��Ļ�ַ
		g_eProc = strtoul( (LPCTSTR)strEproc, '\0', 16);			//ת���ַ���ΪULONG�����浽ȫ�ֱ���g_eThread
	}
	*pResult = 0;
}
				//ˢ��
void CProcessMgr::OnRefreshProList() 
{
	// TODO: Add your command handler code here
	ShowProcessList();
}
				//��������
void CProcessMgr::OnProcKiller() 
{
	// TODO: Add your command handler code here
	//���ͽ���ID������
	ULONG	dwPId=g_dwPId;
	DWORD	retBytes;
	
	ZeroMemory(g_ctrlBuff,MAXBUFFERSIZE);
	memcpy( g_ctrlBuff, &dwPId, sizeof(ULONG) );			//�ѡ�����ID�����������뻺����
	
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//�ѡ�����ID�����͵������豸
		DeviceIoControl(g_hDriver, IOCTL_PROCESS_KILLER, g_ctrlBuff, sizeof(DWORD), NULL, 0, &retBytes, 0);
		ShowProcessList();
	}
}
				//����ģ��
void CProcessMgr::OnProModuleList() 
{
	// TODO: Add your command handler code here
	CModuleMgr	*moduleDlg = new CModuleMgr();
	moduleDlg->Create(IDD_NEW_DLL_MGR_DLG, this);
	moduleDlg->CenterWindow();
	moduleDlg->ShowWindow(SW_SHOW);
}
				//�����߳�
void CProcessMgr::OnProThreadList() 
{
	// TODO: Add your command handler code here
	CThreadMgr	*threadDlg = new CThreadMgr();
	threadDlg->Create(IDD_NEW_THREAD_MGR_DLG, this);
	threadDlg->CenterWindow();
	threadDlg->ShowWindow(SW_SHOW);
}
				//���ƽ�����
void CProcessMgr::OnCopyProcName() 
{
	// TODO: Add your command handler code here
	if ( !g_strProcName.IsEmpty() )
		SetClipboardText( (LPSTR)(LPCTSTR)g_strProcName );
}
				//���ƽ���·��
void CProcessMgr::OnCopyProcPath() 
{
	// TODO: Add your command handler code here
	// ѡ��item�ĵ�4�У�������·��
	CString procPath=m_proList.GetItemText(seledProc, 3);
	if ( !procPath.IsEmpty() )
		SetClipboardText( (LPSTR)(LPCTSTR)procPath );
}
				//��λ�����ļ�
void CProcessMgr::OnGpsProcFile() 
{
	// TODO: Add your command handler code here
	// ѡ��item�ĵ�4�У�������·��
	CString procPath=m_proList.GetItemText(seledProc, 3);
	if ( !procPath.IsEmpty() )
		WinExec("explorer.exe /e,/select," + procPath, SW_SHOW);
}
				//����Esc���رճ���ListCtrl��Dlg
BOOL CProcessMgr::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		//����Esc��
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

	//�򿪼��а�
	if ( !OpenClipboard() )
		return FALSE;

	//��ռ��а�����
	if ( !EmptyClipboard() )
	{
		//�رռ��а�
		CloseClipboard();
		return FALSE;
	}

	int strSize=strlen(pText) + 1;	//size + '\0'
	//�ڶ��������ڴ�
	hClip=GlobalAlloc(GMEM_MOVEABLE, strSize);	
	if (!hClip)
	{
		//�رռ��а�
		CloseClipboard();
		return FALSE;
	}

	char *pBuf;
	//��ס���ڴ�飬���ڴ����ת��Ϊ�ַ���ָ��
	pBuf=(char *)GlobalLock(hClip);
	if (!pBuf)
	{
		//�ͷŶ��ڴ��
		GlobalFree(pBuf);
		//�رռ��а�
		CloseClipboard();
		return FALSE;
	}

	//�����ı����ݵ�����Ķ��ڴ����
	strcpy(pBuf, pText);
	//�������ڴ��
	GlobalUnlock(hClip);
	//���ַ����ڴ������а����Դ������
	if ( !SetClipboardData(CF_TEXT, hClip) )
	{
		//�ͷŶ��ڴ��
		GlobalFree(pBuf);
		//�رռ��а�
		CloseClipboard();
		return FALSE;
	}

	//�رռ��а�
	CloseClipboard();
	return TRUE;
}
