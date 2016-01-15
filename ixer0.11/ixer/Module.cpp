//////////////////////////////////////////////////////////////////////////
// Module.cpp

#include "stdafx.h"
#include "rootkit.h"
#include "API.h"
#include "processMgr.h"
#include "Module.h"


extern CProcessMgr	m_proMgrDlg;
//////////////////////////////////////////////////////////////////////////
void TranslateFilePathName(LPCTSTR szFilename, LPTSTR szWin32Name)
{
	LPCTSTR pszInfo;

	// sanity checks
	if (szFilename[0] == '\0')
	{
		szWin32Name[0]='\0';
		return;
	}
	if (szFilename == NULL)
	{
		szWin32Name[0]='\0';
		return;
	}

	//假设:"C:\WINDOWS"
	UINT Len = ::GetWindowsDirectoryA(szWin32Name, MAX_PATH);
	if ( !Len )
		return;

	// check for "strange" filenames
	//1."%SystemRoot%\xxx.xxx" --> "C:\WINDOWS" + "\xxx.xxx"
	if(strnicmp(szFilename, "%SystemRoot%", strlen("%SystemRoot%")) == 0)
	{
		//* "C:\WINDOWS" + "%SystemRoot%\处后字符串内容"
		strcat(szWin32Name, &szFilename[strlen("%SystemRoot%")]);
		return;
	}

	//2."%WinDir%\xxx.xxx" --> "C:\WINDOWS" + "\xxx.xxx"
	if(strnicmp(szFilename, "%WinDir%", strlen("%WinDir%")) == 0)
	{
		//* "C:\WINDOWS" + "%WinDir%\处后字符串内容"
		strcat(szWin32Name, &szFilename[strlen("%WinDir%")]);
		return;
	}

	//3."\windows\xxx.xxx" --> "C:\WINDOWS" + '\' + "xxx.xxx"
	if(strnicmp(szFilename, "\\windows\\", strlen("\\windows\\")) == 0)
	{
		//假设 "C:\WINDOWS" + '\'
		strcat(szWin32Name, "\\");
		//* "C:\WINDOWS" + "\windows\处后字符串内容"
		strcat(szWin32Name, &szFilename[strlen("\\windows\\")]);
		return;
	} 

	//4."\Program Files\xxx.xxx" --> "C:" + "\Program Files\xxx.xxx"
	if(strnicmp(szFilename, "\\Program Files\\", strlen("\\Program Files\\")) == 0) 
	{
		//pszInfo指向szWin32Name中第一个'\'的位置
		pszInfo=strstr(szWin32Name, "\\");				// top：pszInfo指针 -> szWin32Name指针
		if(pszInfo != NULL)
		{
			//"C:\WINDOWS" + "\Program Files\处及后字符串内容"
			strcpy( (char *)pszInfo, szFilename);
			return;	
		}
	} 

	//5."xxx\SystemRoot\xxx.xxx" --> "C:\WINDOWS\system32\xxx.xxx"
	pszInfo=strstr(szFilename, "\\SystemRoot\\");
	if (pszInfo == szFilename)
	{
		//		"\SystemRoot\system32\smss.exe"
		// ---> "C:\WINDOWS\system32\smss.exe"  using GetWindowsDirectory().
		//"C:\WINDOWS" + '\'
		strcat(szWin32Name, "\\");
		//"C:\WINDOWS\" + "\SystemRoot\后字符串内容"
		strcat(szWin32Name, &szFilename[strlen("\\SystemRoot\\")]); 
		return;
	}

	//6."\??\xxx\xxx.xxx"
	// "\??\C:\WINNT\system32\winlogon.exe" --> "C:\WINNT\system32\winlogon.exe"
	pszInfo=strstr(szFilename, "\\??\\");
	// "\??\"后字符串内容 == "\xxx\xxx.xxx"？
	if (pszInfo == szFilename)
	{
		//等则复制"\??\"后字符串内容
		strcpy(szWin32Name, &szFilename[strlen("\\??\\")]);
		return;
	}

	//7."system32\dirvers\xxx.sys" --> "C:\WINDOWS\system32\dirvers\xxx.sys"
	//
	// *All of the following added for compatible driver path by Chinghoi，2013/03/24
	if (strnicmp(szFilename, "system32\\", strlen("system32\\")) == 0)
	{
		//假设 "C:\WINDOWS" + '\'
		strcat(szWin32Name, "\\");
		//"C:\WINDOWS" + szFilename
		strcat(szWin32Name, szFilename);
		return;
	}

	//8."xxx.sys" --> "C:\WINDOWS\system32\dirvers\xxx.sys"
	if (strnicmp(&szFilename[strlen(szFilename)-4], ".sys", strlen(".sys")) == 0)
	{
		//"C:\WINDOWS" + "\system32\drivers\"
		strcat(szWin32Name, "\\system32\\drivers\\");
		//"C:\WINDOWS\system32\driver\" + "xxx.sys"
		strcat(szWin32Name, szFilename);
		return;
	}

	//9."xxx.exe" --> "C:\WINDOWS\system32\xxx.exe"
	if (strnicmp(&szFilename[strlen(szFilename)-4], ".sys", strlen(".exe")) == 0)
	{
		//"C:\WINDOWS" + "\system32\"
		strcat(szWin32Name, "\\system32\\");
		//"C:\WINDOWS\system32\" + "xxx.exe"
		strcat(szWin32Name, szFilename);
		return;
	}

	// standard name
	strcpy(szWin32Name, szFilename);
}
 
void GetHookModuleName(DWORD dwFunAddress, char *lpModuleName,ULONG pid)
{
	MODULEENTRY32 _stModule;
	HANDLE _hSnapshot; 
	unsigned int distant=-1;
	char path[MAX_PATH]={0};
	
	lpModuleName[0]=0;
	
	RtlZeroMemory(&_stModule,sizeof(_stModule));
	_stModule.dwSize=sizeof(_stModule);
	_hSnapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,pid);
	
	if (Module32First(_hSnapshot,&_stModule))
	{
		do {
			if ((ULONG)_stModule.modBaseAddr<=(ULONG)dwFunAddress) 
			{
				if(distant == -1)
				{
					distant=(ULONG)dwFunAddress - (ULONG)_stModule.modBaseAddr;
					lstrcpyn(path,_stModule.szExePath,MAX_PATH);  
				} 
				else if( (ULONG)dwFunAddress - (ULONG)_stModule.modBaseAddr < distant )
				{
					distant=(ULONG)dwFunAddress-(ULONG)_stModule.modBaseAddr;
					lstrcpyn(path,_stModule.szExePath,MAX_PATH);  
				} 
			}
		}while (Module32Next(_hSnapshot,&_stModule));
	}
	else {
		return ;
	} 
	CloseHandle(_hSnapshot);  

	lstrcpyn(lpModuleName,path,MAX_PATH);  
}
