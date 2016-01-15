// SSDTMgr.cpp : implementation file
//

#include "stdafx.h"
#include "Module.h"
#include "Memory.h"
#include "ixer.h"
#include "SSDTMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CSSDTMgr dialog


CSSDTMgr::CSSDTMgr(CWnd* pParent /*=NULL*/)
	: CDialog(CSSDTMgr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSSDTMgr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSSDTMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSSDTMgr)
	DDX_Control(pDX, IDC_STATIC_SSDT, m_statusSsdt);
	DDX_Control(pDX, IDC_SSDT_LISTS, m_ssdtList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSSDTMgr, CDialog)
	//{{AFX_MSG_MAP(CSSDTMgr)
	ON_NOTIFY(NM_RCLICK, IDC_SSDT_LISTS, OnRclickSsdtLists)
	ON_COMMAND(IDM_REFRESH_SSDT_LIST, OnRefreshSsdtList)
	ON_COMMAND(IDM_SSDT_RESTORE, OnSsdtRestore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////
CSSDTMgr m_ssdtMgrDlg;

/////////////////////////////////////////////////////////////////////////////
// CSSDTMgr message handlers

BOOL CSSDTMgr::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//����ListHead
	m_ssdtList.SetHeadings(_T("���,  50;������,  150;��ǰ��ַ,  90;��������,  85;ԭʼ������ַ,  90;��ǰ��������ģ��,  300"));
	m_ssdtList.LoadColumnInfo();	//��������Ϣ
	//�޸�ListCtrl��չ��ʽ
	DWORD dwStyle = m_ssdtList.GetStyle();		//ȡ��ListCtrl��ʽ
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//�޸�ListCtrl���|WS_VSCROLL|WS_HSCROLL
	m_ssdtList.SetExtendedStyle(dwStyle);		//������
	// TODO: Add extra initialization here
//	m_ssdtList.m_GuiListType=GUILT_SSDTList;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSSDTMgr::ShowSSDTList()
{
	GetCurrentSSDTList(&m_ssdtList);
}

int CSSDTMgr::GetCurrentSSDTList(CSortListCtrl *m_ssdtList)
{
	BOOL		b_Ret;
	ULONG		retBytes;
	ULONG		numOfSrv=0;
	PSSDT_INFO	pSsdtInfo;
	CHAR		Index[6] = "\0";
	CHAR		NewAddr[10] = "\0";
	CHAR		OldAddr[10] = "\0";
	CHAR		hookstype[15] = "\0";
	char		fixSsdtModPath_c[MAXPATHLEN];
	char		ssdtNum_s[128];
	int			hooked=0;

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
		return 0;

	//��ȡssdt����
	b_Ret = DeviceIoControl(g_hDriver,
							(DWORD)IOCTL_GET_SSDT_INFO,
							NULL,
							0,
							&numOfSrv,
							sizeof(DWORD),
							&retBytes,
							NULL);
	if (!b_Ret)
		return 0;

	pSsdtInfo = (PSSDT_INFO)malloc( numOfSrv * sizeof(SSDT_INFO) );
	if ( pSsdtInfo == NULL )
		return 0;

	ZeroMemory( pSsdtInfo, numOfSrv * sizeof(SSDT_INFO) );
		
	//����ssdt��Ϣ����
	b_Ret = DeviceIoControl(g_hDriver,
							(DWORD)IOCTL_GET_SSDT_INFO,
							NULL,
							0,
							pSsdtInfo,
							numOfSrv*sizeof(SSDT_INFO),
							&retBytes,
							NULL);
		if (!b_Ret)
			return 0;
		
		//���ListCtrl�б�
		m_ssdtList->DeleteAllItems();
		
		for (ULONG i = 0; i < numOfSrv; i++)
		{
			sprintf( Index, "%d", i );
			sprintf( NewAddr, "0x%08X", pSsdtInfo[i].currentAddr );
			sprintf( OldAddr, "0x%08X", pSsdtInfo[i].nativeAddr );

			//��ǰ��ַ ������ ԭʼ��ַ����Ϊhooked��
			if (pSsdtInfo[i].currentAddr != pSsdtInfo[i].nativeAddr)
			{
				strcpy(hookstype, " ssdt hook");
				hooked++;
			}
			else
				strcpy(hookstype, "     -     ");

			//����·��
			TranslateFilePathName(pSsdtInfo[i].imagePath, fixSsdtModPath_c);

			m_ssdtList->AddItem(
								NULL,						//ͼ��index
								Index,						//���
								pSsdtInfo[i].funName,		//������
								NewAddr,					//��ǰ��ַ
								hookstype,					//��������
								OldAddr,					//ԭʼ��ַ
								fixSsdtModPath_c);	//��ǰssdt������ַ�����ں�ģ��·��
		}
	
	//��ʽ��ģ�鴰�ڱ����ַ���
	sprintf(ssdtNum_s, "SSDT������ %d,  �ҹ������� %d", numOfSrv, hooked);
	//״̬����
	m_ssdtMgrDlg.SetDlgItemText(IDC_STATIC_SSDT, ssdtNum_s);
	//��ʽ���ַ���
	free( pSsdtInfo );
	return numOfSrv;
}

void CSSDTMgr::OnRclickSsdtLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
	{
		//ѡ��item��
		ssdtSeled=0;
		CPoint pt;
		//��ȡ�����ListCtrl�ĵ��λ��
		GetCursorPos(&pt);
		CMenu menu;
		//���ؽ��̵��Ҽ��˵�
		menu.LoadMenu(IDR_SSDT_RPOP);
		//��ȡ��һ���Ӳ˵�
		CMenu *pMenu = menu.GetSubMenu(0);
		//�����˵���׷��ѡ�е�Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		ssdtSeled = m_ssdtList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item
	}
	*pResult = 0;
}

void CSSDTMgr::OnRefreshSsdtList() 
{
	// TODO: Add your command handler code here
	ShowSSDTList();
}

void CSSDTMgr::OnSsdtRestore() 
{
	// TODO: Add your command handler code here
	//����ssdt������������ԭʼ��ַ������
	CString	strIndex;
	ULONG	dwSrvIndex;			//index
	ULONG	dwCurrentAddr;		//��ǰ��ַ
	ULONG	dwOriginAddr;		//ԭʼ��ַ

	CHAR	*wChr={'\0'};
	CString strCurrentAddr;
	CString strOriginAddr;

	// ѡ��item�ĵ�1�У���ssdt����������
	strIndex = m_ssdtList.GetItemText(ssdtSeled, 0);
	wChr = strIndex.GetBuffer( strIndex.GetLength() );
	//ת��������
	dwSrvIndex = atol(wChr);
	
	// ��3�У�"ssdt��ǰ��ַ"
	strCurrentAddr = m_ssdtList.GetItemText(ssdtSeled, 2);
	// ת��������
	dwCurrentAddr = strtoul( (LPCTSTR)strCurrentAddr, '\0', 16);
	
	// ��5�У�"ssdtԭʼ��ַ"
	strOriginAddr = m_ssdtList.GetItemText(ssdtSeled, 4);
	// ת��������
	dwOriginAddr = strtoul( (LPCTSTR)strOriginAddr, '\0', 16);

	SSDTINFO	sinfo = {0, 0};
	unsigned long int retBytes;

	//��ǰ��ַ==ԭʼ��ַ����ֱ�ӷ���
	if (dwCurrentAddr == dwOriginAddr)
		return;

	//�����豸�����ЧҲ����
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		sinfo.index	  = dwSrvIndex;		//index
		sinfo.funAddr = dwOriginAddr;	//ԭʼ��ַ

		//��ssdt��������������ԭʼ��ַ�����͵������豸
		DeviceIoControl(g_hDriver,
						(DWORD)IOCTL_RESTORE_SSDT,
						&sinfo,
						sizeof(SSDTINFO),
						NULL,
						0,
						&retBytes,
						NULL);
		ShowSSDTList();
	}
}
