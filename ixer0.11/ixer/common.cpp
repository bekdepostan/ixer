/*===================================================================
 * Filename: common.cpp
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: ͨ��ģ�飬���õĺ��������ﶨ��
 *
 * Date:   2013-5-16 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#include "StdAfx.h"
#include <windows.h>
#include <Dbghelp.h>
#include <Psapi.h>
#include "ShadowSSDTMgr.h"
#include "common.h"

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment (lib, "Psapi.lib")


HANDLE	g_ixerhProc;

extern	DWORD		g_W32pServiceTable;
extern	SHADOWFUNC	g_ShadowFunc[667];
extern	char	symFilePath[MAX_PATH];


typedef PLOADED_IMAGE(__stdcall *IMAGELOAD)(
											 __in  PSTR DllName,
											 __in  PSTR DllPath
											 );
IMAGELOAD pImageLoad;

typedef BOOL(__stdcall *IMAGEUNLOAD)(
										__in  PLOADED_IMAGE LoadedImage
									);
IMAGEUNLOAD pImageUnload;


typedef BOOL(__stdcall *SYMGETSYMBOLFILE)(
										  __in_opt HANDLE hProcess,
										  __in_opt PCSTR SymPath,
										  __in PCSTR ImageFile,
										  __in DWORD Type,
										  __out_ecount(cSymbolFile) PSTR SymbolFile,
										  __in size_t cSymbolFile,
										  __out_ecount(cDbgFile) PSTR DbgFile,
										  __in size_t cDbgFile
										  );
SYMGETSYMBOLFILE symGetSymbolFile;



BOOLEAN CALLBACK EnumSymRoutine(
								PSYMBOL_INFO pSymInfo,
								ULONG     SymSize,
								PVOID     UserContext )
{
	PDWORD	pW32pServiceTable = NULL;
	INT		i = 0;
	
	if ( !UserContext )
	{
		if ( strstr( pSymInfo->Name, "W32pServiceTable" ) )
		{
			char symbolInfo[128];
			
			sprintf ( symbolInfo, "%s : 0x%X \n", pSymInfo->Name, pSymInfo->Address );	
			OutputDebugString( symbolInfo );
			g_W32pServiceTable = (DWORD)pSymInfo->Address;
		}
	}
	else
	{
		pW32pServiceTable = (PDWORD)UserContext;
		for (i = 0; i < 667; i++)
		{
			if ( *(pW32pServiceTable + i) == (DWORD)pSymInfo->Address )
			{
				g_ShadowFunc[i].nativeAddr = (DWORD)pSymInfo->Address;
				lstrcpyn( g_ShadowFunc[i].funcName, pSymInfo->Name, 100 );		
			}
		}
	}
	
	return TRUE;
}

BOOL InitSymbolsHandler()
{
	HANDLE	hFile;
	char	path[MAX_PATH]={0};
	char	fileName[MAX_PATH]={0};
	char	symPath[MAX_PATH*2]={0};
	BOOL	boRet;
	
	// ��ȡϵͳĿ¼
	if ( !GetWindowsDirectory( path, MAX_PATH ) )
	{
		OutputDebugString ("cannot get system directory. \n");
		return FALSE;
	}
	// "C:\WINDOWS\symbols"
	strcat( path, "\\symbols" );

	// ��"C:\WINDOWS"�´���symbolsĿ¼
	CreateDirectoryA( path, NULL );

	strcpy( fileName, path );
	// "C:\WINDOWS\symbols\symsrv.yes"
	strcat( fileName, "\\symsrv.yes");

	// ��"C:\WINDOWS\symbols"Ŀ¼�´���/��symsrv.yes�ļ�
	hFile = CreateFileA( fileName,
						FILE_ALL_ACCESS,
						FILE_SHARE_READ,
						NULL,
						OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL );

	if (hFile == INVALID_HANDLE_VALUE)
	{
		char err[128];
		
		sprintf( err, "Create or open \"symsrv.yes\" file error: 0x%X \n", GetLastError() );
		OutputDebugString( err );
		return FALSE;
	}
	CloseHandle(hFile);

	// 
//	Sleep(3000);
	// ��ȡ��ǰ���̾��
	g_ixerhProc = GetCurrentProcess();
	// symbol����ѡ��
	SymSetOptions ( SYMOPT_CASE_INSENSITIVE|SYMOPT_DEFERRED_LOADS|SYMOPT_UNDNAME );

	// 
	char* symUrl = "http://msdl.microsoft.com/download/symbols";
	sprintf( symPath, "SRV*%s*%s", path, symUrl );

	// "SRV*C:\WINDOWS\symbols*http://msdl.microsoft.com/download/symbols"
	OutputDebugString (symPath);
	// ��ʼ�����Ŵ�����
	boRet = SymInitialize( g_ixerhProc, symPath, TRUE );	//����ؽ�������ģ��ĵ��Է���

	return boRet;
}

//��ȡָ����ַ�Ĺ���ģ����
BOOL GetFunctionBelongingModuleName(DWORD ulPid, LPVOID lpvBase, LPSTR lpFilename, DWORD nSize)
{
	//�̺߳���ģ��
	char	tmpFltStrMod[MAX_PATH]={0};
	
	//��������Ȩ��
	AdjustPrivileges( SE_DEBUG_NAME );
	//�򿪽���
	HANDLE	hPro=OpenProcess(PROCESS_ALL_ACCESS, FALSE, ulPid);
	//��ȡ�̺߳���ģ��
	DWORD rtnSize=GetMappedFileName(hPro, lpvBase, lpFilename, nSize);
	if (!rtnSize)
	{
		ZeroMemory(lpFilename, nSize);
		return FALSE;
	}
	
	strcpy(tmpFltStrMod, lpFilename);
	for (int i=strlen(tmpFltStrMod)-1; i > -1; i--)
	{
		if (tmpFltStrMod[i] == '\\')
		{
			strcpy(lpFilename, &tmpFltStrMod[i+1]);	//������һ��'\'Ȼ��+1ȡ��ַ
			break;
		}
	}
	
	return TRUE;
}

//��������Ȩ��
BOOL AdjustPrivileges( LPCTSTR lpName )
{
	HANDLE		hToken;
	LUID		luid;
	TOKEN_PRIVILEGES tkPriv;

	/* �򿪽������ƻ� */
	if ( !OpenProcessToken(	GetCurrentProcess(),
							TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
							&hToken ) )
	{
		return FALSE;
	}

	/* �鿴��Ȩֵ��ʶ */
	if ( !LookupPrivilegeValue( NULL, lpName, &luid) )
	{
		CloseHandle(hToken);
		return FALSE;
	}
	
	tkPriv.PrivilegeCount = 1;
	tkPriv.Privileges[0].Luid = luid;
	tkPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	/* ����Ȩ�� */
	if ( !AdjustTokenPrivileges( hToken, FALSE, &tkPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL ) )
	{
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}