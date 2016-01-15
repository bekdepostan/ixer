// ixer.cpp : Defines the class ixers for the application.
//

#include "stdafx.h"
#include "..\Common\Surface.h"
#include "ixer.h"
#include "ixerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CixerApp

BEGIN_MESSAGE_MAP(CixerApp, CWinApp)
	//{{AFX_MSG_MAP(CixerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CixerApp construction

CixerApp::CixerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CixerApp object

//////////////////////////////////////////////////////////////////////////
//全局变量和函数
CixerApp		theApp;								//主应用程序对象

HANDLE			g_hDriver=INVALID_HANDLE_VALUE;		//设备句柄

char			g_ctrlBuff[MAXBUFFERSIZE];			//DeviceIoControl函数的输入输出buffer

//DATA_MEM_POOL	*g_dataMemPool;						//ring0和ring3共享的数据内存池

// PROCESS_ITEM	g_allProcessItems[2*MAX_PROCESS_NUM];
// ULONG			g_allProcessNum;

/////////////////////////////////////////////////////////////////////////////
// CixerApp initialization

BOOL CixerApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CixerDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
// 接受多个参数并打印调试信息
void MyOutputDebugString(IN const char * sz, ...)
{
	char szData[4096]={0};
	//接受参数
	va_list args;
	va_start(args, sz);
	_vsnprintf(szData, sizeof(szData) - 1, sz, args);
	va_end(args);
	//本机输出调试信息
	OutputDebugString(szData);
}

VOID	showErrBox( void )
{
#ifdef _DEBUG

	ULONG	dwErrCode;
	char	lpText[512];
	dwErrCode = GetLastError( );

	LPVOID	lpMsgBuf;
	// 格式化消息
	FormatMessage ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					dwErrCode,
					MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),	// Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL );

	sprintf( lpText, "err code: %d \rLookup: %s", dwErrCode, lpMsgBuf );
	// Display the string.
	::MessageBox( NULL, lpText, "ixer err Lookup", 0x00200000L | MB_OK|MB_ICONINFORMATION );	//(LPCTSTR)lpMsgBuf
	
	// Free the buffer.
	LocalFree( lpMsgBuf );
#endif
}