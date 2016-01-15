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
int	seledDrv;		//ѡ��item��
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

	//����ListHead
	m_DrvModList.SetHeadings(_T("������,  85;��ַ,  72;��С,  72;��������,  72;ģ��·��,  250;������,  62;����˳��,  40;�ļ�����,  105"));	//��ڵ�,  72;ж������,  72;˳��,  36;
	m_DrvModList.LoadColumnInfo();	//��������Ϣ
	//�޸�ListCtrl��չ��ʽ
	DWORD dwStyle;
	dwStyle = m_DrvModList.GetStyle();	//ȡ��ListCtrl��ʽ
	dwStyle |=  LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP ;	//�޸�ListCtrl���|WS_VSCROLL|WS_HSCROLL
	m_DrvModList.SetExtendedStyle(dwStyle);     //������
	m_DrvModList.SetImageList(&m_proMgrDlg.images, LVSIL_SMALL);
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDrvModMgr::ShowDrvModList()
{
	//��ȡDriverList�������ListCtrl�������ؽ�����
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

	char	drvModName_c[MAXNAMELEN];		//������
	char	drvSrvName_c[MAXNAMELEN]="";	//������
	char	drvModPath_c[MAXPATHLEN];
	char	fixDrvPath_c[MAXPATHLEN];		//·��
	char	baseAddress_c[16];				//��ַ
	char	sizeOfImage_c[16];				//��С
	char	drvObject_c[16];				//��������
	char	ulLoadOrder_c[16];				//����˳��

	PDRVMOD_ITEM	pDrvInfo=NULL;
	PDRV_SRVNAME	pDrvSrvName=NULL;
	int		srvNums=0;
	int		hided=0;
	int		suspectImages=0;
	char	drvNum_s[128];
	int		iDefIconDrv;

	//���ListCtrl�б�
	m_DrvModList->DeleteAllItems();

	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
	{
		return 0;
	}

	//�����ڴ�
	pDrvSrvName=(DRV_SRVNAME *)malloc(sizeof(DRV_SRVNAME)*ifDrvNum);
	if ( !pDrvSrvName )
		free(pDrvSrvName);

	//�ռ�׼��������������
	srvNums=ReadySrvName(pDrvSrvName);

	//������������̽ṹ�ڴ�
	pDrvInfo=(DRVMOD_ITEM *)malloc(sizeof(DRVMOD_ITEM)*ifDrvNum);
	if (!pDrvInfo)
	{
		free(pDrvInfo);
		return drvCount;
	}

	//�ڴ�����
	ZeroMemory(pDrvInfo, sizeof(DRVMOD_ITEM)*ifDrvNum);
	//��ȡ����
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
					//ת��ģ���ַ
					sprintf(baseAddress_c, "0x%08X", pDrvInfo[i].ulBaseAddr);
					//ת��ģ���С
					sprintf(sizeOfImage_c, "0x%08X", pDrvInfo[i].ulImageSize);
					//ת������˳��
					sprintf(ulLoadOrder_c, "%d", pDrvInfo[i].ulLoadOrder);

					//������������ת��
					if (pDrvInfo[i].pDrvObject)
						sprintf(drvObject_c, "0x%08X", pDrvInfo[i].pDrvObject);
					else
						strcpy(drvObject_c, "-");

					//���������ַ�ת�༸��
					WideCharToMultiByte(CP_ACP, 0, pDrvInfo[i].pwszDrvName, -1, drvModName_c, sizeof(drvModName_c), NULL, NULL);
					//����·�����ַ�ת�༸��
					WideCharToMultiByte(CP_ACP, 0, pDrvInfo[i].pwszDrvPath, -1, drvModPath_c, sizeof(drvModPath_c), NULL, NULL);

					//����·��
					TranslateFilePathName(drvModPath_c, fixDrvPath_c);
					//��ȡ�����ļ���˾
					m_modMgrDlg.GetFileCorporation(fixDrvPath_c, pDrvInfo[i].pszFileCorp);

					//������������������
					if (srvNums && pDrvSrvName)
					{
						for (int i=0; i<srvNums; i++)
						{
							//����·�������·��ƥ�䣬���ȡ������
							if (stricmp(fixDrvPath_c, pDrvSrvName[i].szSrvPath) == 0)
							{
								strcpy(drvSrvName_c, pDrvSrvName[i].szSrvName);
								break;
							}
						}
					}

					//��ȡͼ��
					iDefIconDrv=m_proMgrDlg.GetIconIndex( fixDrvPath_c );

					//������������ʾ��ʱ�����ļ����������Լ����Ű��~
					if (pDrvInfo[i].hideFlags)
					{
						strcpy( pDrvInfo[i].pszFileCorp, "����" );
						hided++;
					}

					//�����б�ؼ�(""������,  85;��ַ,  72;��С,  72;��������,  72;ж������,  72;ģ��·��,  220;˳��,  36;����,  36;�ļ�����,  90")
					m_DrvModList->AddItem(
											iDefIconDrv,				//ͼ��������
											drvModName_c,				//ģ����
											baseAddress_c,				//��ַ
											sizeOfImage_c,				//��С
											drvObject_c,				//��������
											fixDrvPath_c,				//ģ��·��
											drvSrvName_c,				//������
											ulLoadOrder_c,				//����˳��
											pDrvInfo[i].pszFileCorp		//�����ļ���˾
											);
					//��շ�����
					ZeroMemory(drvSrvName_c, sizeof(drvSrvName_c));
				}
			}
		}
	}
	else
		showErrBox();

	//��ʽ���ַ���
	sprintf(drvNum_s, "������ %d,  ���������� %d,  ����PEӳ�� %d", drvCount, hided, suspectImages);
	//����״̬��
	m_DrvMgrDlg.m_DrvModList.GetParent()->SetDlgItemText(IDC_STATIC_DRVMOD, drvNum_s);

	free(pDrvSrvName);
	free(pDrvInfo);
	return drvCount;
}

int CDrvModMgr::ReadySrvName(PDRV_SRVNAME pDrvSrvName)
{
	SC_HANDLE hScm;			//SCM���
	SC_HANDLE hSrv;			//Srv���
	ULONG	bytesNeeded=0;	//���سߴ�
	ULONG	srvNum=0;		//�������
	LPENUM_SERVICE_STATUS srvStatus;	//״̬������
	LPQUERY_SERVICE_CONFIG pSrvConfig=NULL;	//��������

	char	fixSrvPath_c[MAXPATHLEN];		//·��

	//��ȡSCM���
	hScm=OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	
	//��ȡ�������񻺳�����С
	EnumServicesStatus(hScm, SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &srvNum, NULL);
	
	//���뻺����
	srvStatus=(LPENUM_SERVICE_STATUS)malloc(bytesNeeded);

	//��ʽö��
    if( !EnumServicesStatus(hScm, SERVICE_DRIVER, SERVICE_STATE_ALL, (LPENUM_SERVICE_STATUS)srvStatus, bytesNeeded, &bytesNeeded, &srvNum, NULL) )
		return 0;

	//������ѯ���з��񲢱�����Ϣ
	for (ULONG i=0; i<srvNum; i++)
	{
		//�򿪷����ȡsrv���
		hSrv=OpenService(hScm, srvStatus[i].lpServiceName, SERVICE_QUERY_CONFIG);
		if(hSrv==NULL)
			continue;

		//�ɷ�������ѯ ����� �������� PE�ļ�·��������
		//��ȡ��Ҫ�Ļ���������
		QueryServiceConfig(hSrv, NULL, 0, &bytesNeeded);
		//���뻺����
		pSrvConfig=(LPQUERY_SERVICE_CONFIG)malloc(bytesNeeded);
		//��ʽ��ѯ
		if( !QueryServiceConfig(hSrv,pSrvConfig, bytesNeeded, &bytesNeeded) )
		{
			free(pSrvConfig);
			continue;
		}

		//������
		strcpy(pDrvSrvName[i].szSrvName, srvStatus[i].lpServiceName);

		//ת������·��
		TranslateFilePathName(pSrvConfig->lpBinaryPathName, fixSrvPath_c);
		//����·��
		strcpy(pDrvSrvName[i].szSrvPath, fixSrvPath_c);
		//������ʱ·������
		ZeroMemory(fixSrvPath_c, strlen(fixSrvPath_c));
	}

	return srvNum;
}

void CDrvModMgr::OnRclickDrvmodLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	CPoint pt;
	//��ȡ�����ListCtrl�ĵ��λ��
	GetCursorPos(&pt);
	CMenu menu;
	//�����ļ����Ҽ��˵�
	menu.LoadMenu(IDR_DRV_RPOP);
	//��ȡ��1���Ӳ˵�
	CMenu *pMenu = menu.GetSubMenu(0);
	//�����˵���׷��ѡ�е�Item
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
	seledDrv = m_DrvModList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item
	*pResult = 0;
}
				//ˢ��
void CDrvModMgr::OnRefreshDrvList() 
{
	// TODO: Add your command handler code here
	this->ShowDrvModList();
}
				//����������
void CDrvModMgr::OnCopyDrvName() 
{
	// TODO: Add your command handler code here
	// ѡ��item�ĵ�1�У���������
	CString drvName=m_DrvModList.GetItemText(seledDrv, 0);
	if ( !drvName.IsEmpty() )
		m_proMgrDlg.SetClipboardText( (LPSTR)(LPCTSTR)drvName );
}
				//���ƽ���·��
void CDrvModMgr::OnCopyDrvPath() 
{
	// TODO: Add your command handler code here
	// ѡ��item�ĵ�4�У�������·��
	CString drvPath=m_DrvModList.GetItemText(seledDrv, 4);
	if ( !drvPath.IsEmpty() )
		m_proMgrDlg.SetClipboardText( (LPSTR)(LPCTSTR)drvPath );
}
				//��λ�����ļ�
void CDrvModMgr::OnGpsDrvFile() 
{
	// TODO: Add your command handler code here
	// ѡ��item�ĵ�4�У�������·��
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
	//ƴ�ӳ�nt·��
	strPathNT = "\\??\\" + drvPath;
	
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
}
