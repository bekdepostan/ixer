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

	//����ListHead
	m_shadowList.SetHeadings(_T("���,  50;������,  150;��ǰ��ַ,  90;��������,  85;ԭʼ������ַ,  90;��ǰ��������ģ��,  300"));
	m_shadowList.LoadColumnInfo();	//��������Ϣ
	//�޸�ListCtrl��չ��ʽ
	DWORD dwStyle = m_shadowList.GetStyle();		//ȡ��ListCtrl��ʽ
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//�޸�ListCtrl���|WS_VSCROLL|WS_HSCROLL
	m_shadowList.SetExtendedStyle(dwStyle);		//������
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
	unsigned long int	bytesReturned;		//�����ֽ���
	PSHADOW_INFO	pShadowInfo=NULL;
	BOOL		b_Ret;
	bool		bFlag=false;
	DWORD		numOfSrv=0;			//ʵ�ʷ��صķ���������
	DWORD		iCount	=0;
	char		fixShadowModPath_c[MAXPATHLEN];
	char		shadowNum_s[128];
	int			hooked=0;

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return 0;
	}

	//��ȡShadowSSDT��Ϣ��С
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

	//������ڴ�
	pShadowInfo=(PSHADOW_INFO)AllocHeapMem( numOfSrv*sizeof(SHADOW_INFO) );

	//ͨ��symbol��ȡShadowSSDT��ԭʼ������ַ��������
	b_Ret = GetShadowSSDTNativeAddrBySymbol( );
	
	//����ShadowSSDT��Ϣ����
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

	//���ListCtrl�б�
	m_shadowList->DeleteAllItems();
	
	for (ULONG i = 0; i < numOfSrv; i++)
	{
		sprintf( index_c, "%d", i );
		sprintf( currentAddr_c, "0x%08X", pShadowInfo[i].currentAddr );
		sprintf( nativeAddr_c, "0x%08X", g_ShadowFunc[i].nativeAddr );

		//��ǰ��ַ ������ ԭʼ��ַ����Ϊhooked��
		if (pShadowInfo[i].currentAddr != g_ShadowFunc[i].nativeAddr && g_ShadowFunc[i].nativeAddr != 0)
		{
			strcpy(hookstype, " ssdt hook");
			hooked++;
		}
		else
			strcpy(hookstype, "     -     ");

		//����·��
		TranslateFilePathName( pShadowInfo[i].imagePath, fixShadowModPath_c );
		
		m_shadowList->AddItem(
								NULL,						//ͼ��index
								index_c,					//���
								g_ShadowFunc[i].funcName,	//������
								currentAddr_c,				//��ǰ��ַ
								hookstype,					//��������
								nativeAddr_c,				//ԭʼ��ַ
								fixShadowModPath_c);		//��ǰssdt������ַ�����ں�ģ��·��
		}

	//��ʽ��ģ�鴰�ڱ����ַ���
	sprintf(shadowNum_s, "ShadowSSDT������ %d,  �ҹ������� %d", numOfSrv, hooked);	//���Ӻ�����%d
	//״̬����
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
		//ѡ��item��
		shadowSeled=0;
		CPoint pt;
		//��ȡ�����ListCtrl�ĵ��λ��
		GetCursorPos(&pt);
		CMenu menu;
		//���ؽ��̵��Ҽ��˵�
		menu.LoadMenu(IDR_SHADOW_RPOP);
		//��ȡ��һ���Ӳ˵�
		CMenu *pMenu = menu.GetSubMenu(0);
		//�����˵���׷��ѡ�е�Item
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
		shadowSeled = m_shadowList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		// -1�ӵ�1��item��ʼ������,ƥ��ѡ��item
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
	//����ShadowSsdt������������ԭʼ��ַ������
	CString	strIndex;
	CString strOriginAddr;
	CString strCurrentAddr;
	
	CHAR	*wChr={'\0'};
	ULONG	dwSrvIndex;			//index
	ULONG	dwCurrentAddr;		//��ǰ��ַ
	ULONG	dwOriginAddr;		//ԭʼ��ַ
	
	// ѡ��item�ĵ�1�У���ssdt����������
	strIndex = m_shadowList.GetItemText(shadowSeled, 0);
	wChr = strIndex.GetBuffer( strIndex.GetLength() );
	//ת��������ѡ��item��ssdt����������
	dwSrvIndex = atol(wChr);
	
	// ѡ��item�ĵ�3�У���"ssdt��ǰ��ַ"
	strCurrentAddr = m_shadowList.GetItemText(shadowSeled, 2);
	// ת��������
	dwCurrentAddr = strtoul( (LPCTSTR)strCurrentAddr, '\0', 16);
	
	// ѡ��item�ĵ�5�У���"ssdtԭʼ��ַ"
	strOriginAddr = m_shadowList.GetItemText(shadowSeled, 4);
	// ת��������
	dwOriginAddr = strtoul( (LPCTSTR)strOriginAddr, '\0', 16);

	//��ǰ��ַ==ԭʼ��ַ����ֱ�ӷ���
	if (dwCurrentAddr == dwOriginAddr)
		return;
	
	//�����豸�����ЧҲ����
	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;

	SSDTINFO	sinfo = {0, 0};
	unsigned long int retBytes;

	sinfo.index	  = dwSrvIndex;		//index
	sinfo.funAddr = dwOriginAddr;	//ԭʼ��ַ
	
	//��ssdt��������������ԭʼ��ַ�����͵������豸
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

	// ��ʼ��
	if ( !InitSymbolsHandler() )
	{
		return FALSE;
	}

	// ��ȡϵͳĿ¼"C:\WINDOWS\system32"
	GetSystemDirectory( imgPath, MAX_PATH );
	// ����ģ��
	ploadImage = ImageLoad( "win32k.sys", imgPath );

	// ���ط���"C:\WINDOWS\system32\win32k.sys"
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
