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

ULONG	g_baseAddr=0;				//ѡ��itemģ��Ļ�ַ
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
	//������ͷ��Ϣ
	m_moduleList.SetHeadings( _T("ģ��·��,  220;��ַ,  80;��С,  80;�ļ�����,  150"));		//��ڵ�ַ,  100;
	//��������Ϣ
	m_moduleList.LoadColumnInfo();

	//CListCtrl��չ��ʽ�ı�
	DWORD dwStyle = m_moduleList.GetStyle();	//ȡ����ʽ
	dwStyle |=  LVS_EX_GRIDLINES				//������ʽ
			| LVS_EX_FULLROWSELECT				//����ѡ��
			| LVS_EX_HEADERDRAGDROP;			//�б�ͷ�����Ϸ�

	//������չ���
	m_moduleList.SetExtendedStyle(dwStyle);

	//��ʼ����ʾģ��List
	ShowModuleList();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CModuleMgr::ShowModuleList()
{
	unsigned int moduleNums=0;					//ģ������
	char		 proName_Nums[256]="";			//��ʾģ���б�Ľ�������ģ����

	//��ȡ��ǰModuleList���������ListCtrl������ģ����
	moduleNums=GetCurrentModuleList(this);

	//��ʽ��ģ�鴰�ڱ����ַ���
	sprintf(proName_Nums, "[%s]-����ģ��(%d)", g_strProcName, moduleNums);
	//����ģ�鴰�ڱ���
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

	//���ͽ���ID������
	ULONG	dwProId=g_dwPId;
	char	selItemProId[16];				//����ID(ѡ��item���ַ�����ʽ"����ID")
	ULONG	i=0;
	char	baseAddress_c[16];				//��ַ
	char	sizeOfImage_c[16];				//ӳ���С
	char	dllModPath_c[MAXPATHLEN];
	char	fixDllPath_c[MAXPATHLEN];		//·��
 	char	pszDllCorporation[256]="";
	CSortListCtrl *m_ModuleList;			//ģ��Ի�����ģ���б�ؼ�

	//��ʼ������ָ�� ����Ҫ��������
	if(pDialog)
	{
		m_ModuleList = &pDialog->m_moduleList;
		sprintf(selItemProId, "%d", g_dwPId);
	}
	else
		return 0;
	
	//���ListCtrl�б�
	m_ModuleList->DeleteAllItems();
	
	if(g_hDriver==INVALID_HANDLE_VALUE  || g_hDriver==NULL)
		return 0;

	ZeroMemory(g_ctrlBuff, MAXBUFFERSIZE);
	memcpy( g_ctrlBuff, &dwProId, sizeof(ULONG) );	//�ѡ�����ID�����������뻺����

	//������������̽ṹ�ڴ�
	pModInfo=(DLL_ITEM *)malloc(sizeof(DLL_ITEM)*ifDllNum*2);
	if (!pModInfo)
	{
		free(pModInfo);
		return dllCount;
	}

	//�ڴ�����
	ZeroMemory(pModInfo, sizeof(DLL_ITEM)*ifDllNum*2);
	//��ȡ����
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
					//ת��ģ���ַ
					sprintf(baseAddress_c, "0x%08X", pModInfo[i].ulBaseAddr);
					//ת��ģ���С
					sprintf(sizeOfImage_c, "0x%08X", pModInfo[i].ulImageSize);
					//���ַ�ת�༸��
					WideCharToMultiByte(CP_ACP, 0, pModInfo[i].pwszPath, -1, dllModPath_c, sizeof(dllModPath_c), NULL, NULL);
					//����ת��·��
					TranslateFilePathName(dllModPath_c, fixDllPath_c);
					//��ȡDLL�ļ���˾
					GetFileCorporation(fixDllPath_c, pszDllCorporation);
					
					//�����б�ؼ� ("���,40;ģ����,100;��ַ,100;���,100;����pid,100;ģ��·��,250")
					m_ModuleList->AddItem(
										  NULL,					//ͼ��������
										  fixDllPath_c,			//ģ��·��
										  baseAddress_c,		//��ַ
										  sizeOfImage_c,		//��С
										  pszDllCorporation		//DLL�ļ���˾
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
		lpXBuffer=new BYTE[dwXInfoSize];				//����汾��Ϣ��С���ڴ�
		memset(lpXBuffer, 0, dwXInfoSize*sizeof(BYTE));	//���㣬���ڱ���汾��Ϣ
		//��ȡ�ļ��汾��Ϣ
		if (GetFileVersionInfo(pszFileFullPath, dwXHandle, dwXInfoSize, lpXBuffer))
		{
			//�ļ��������Ϣ
			struct LANGANDCODEPAGE 
			{
				WORD wLanguage;
				WORD wCodePage;
			} *lpTranslate;

			if( !VerQueryValue(lpXBuffer, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &uDataSize) )
				free(lpXBuffer);

			//��˾
			wsprintf(textBuffer,TEXT("\\StringFileInfo\\%04x%04x\\CompanyName"),
							lpTranslate[0].wLanguage,
							lpTranslate[0].wCodePage);

			//�ļ��Ҽ���"����"->"�汾"->"��˾"
			if( VerQueryValue(lpXBuffer, textBuffer, &lpXData, &uDataSize) )
				strcpy( (CHAR *)pszFileCorporation, LPTSTR(lpXData) );	//����File Corporation
		}
		free(lpXBuffer);
		lpXBuffer=NULL;

		return TRUE;
	}
	else	//��ȡ�ļ��汾��Ϣ��Сʧ�ܣ������ļ��Ƿ����
	{
		//"ֻ��"��ʽ���ļ�
		hFile=fopen(pszFileFullPath, "r");
		if ( !hFile )
		{
			//�ų�"Idle"��"System"2���������·��
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

			strcpy( (CHAR *)pszFileCorporation, "�ļ�������" );
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

				//�Ҽ�
void CModuleMgr::OnRclickMoudleLists(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
		NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
		if (pNMListView->iItem != -1 && pNMListView->iSubItem != -1)
		{
			CString strbaseAddr;
			int		selectedItem=0;			//ѡ��item��
			//��ȡ�����ListCtrl�ĵ��λ��
			CPoint pt;
			GetCursorPos(&pt);
			//���ؽ��̵��Ҽ��˵�
			CMenu menu;
			menu.LoadMenu(IDR_DLL_RPOP);
			//��ȡ��һ���Ӳ˵�
			CMenu *pMenu = menu.GetSubMenu(0);
			//�����˵���׷��ѡ�е�Item
			pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, this);
			selectedItem = m_moduleList.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);		//-1�ӵ�1��item��ʼ������,ƥ��ѡ��item

			// ѡ��item�ĵ�2�У���Dll��ַ
			strbaseAddr = m_moduleList.GetItemText(selectedItem, 1);
			// ת��������ѡ��itemģ��Ļ�ַ
			g_baseAddr = strtoul( (LPCTSTR)strbaseAddr, '\0', 16);					//ת���ַ���ΪLong�����浽ȫ�ֱ���g_baseAddr													
		}
		*pResult = 0;
}
				//����ģ��
void CModuleMgr::OnRefreshModList() 
{
	// TODO: Add your command handler code here
	ShowModuleList();
}

//ж��ģ��
void CModuleMgr::OnModTerminator() 
{
	// TODO: Add your command handler code here
	//����ģ���PID����ַ������
	ULONG	dwProId=g_dwPId;
	ULONG	dwbaseAddr=g_baseAddr;
	DWORD	dw;

	ZeroMemory(g_ctrlBuff, MAXBUFFERSIZE);
	
	//��ģ������������ID����ַ�����������뻺����
	*(ULONG *)g_ctrlBuff=dwProId;
	*(ULONG *)( (ULONG)g_ctrlBuff + 4 )=dwbaseAddr;

	if( !g_hDriver || g_hDriver == INVALID_HANDLE_VALUE)
		return;
	else
	{
		//�ѡ�����ģ���ID����ַ�����͵������豸
		DeviceIoControl(g_hDriver, IOCTL_DLL_TERMINATOR, g_ctrlBuff, MAXBUFFERSIZE, g_ctrlBuff, MAXBUFFERSIZE, &dw, 0);
		ShowModuleList();
	}
}



