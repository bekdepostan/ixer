//////////////////////////////////////////////////////////////////////////
// Module.h

#ifndef ___MODULE___
#define ___MODULE___

#include "StdAfx.h"
#include "rootkit.h"



void TranslateFilePathName(LPCTSTR szFilename, LPTSTR szWin32Name);

BOOL UnmapViewOfModule ( DWORD dwProcessId, LPVOID lpBaseAddr );

int GetKernelModule( CSortListCtrl * m_KModuleList );
//////////////////////////////////////////////////////////////////////////


PMODULES
EnumModuleInfo(
			   void
			   );

void
FreeModuleInfo(
			   PMODULES	pModuleInfo
			   );

PCHAR
FindModuleName(
			   DWORD	FunAddress,
			   PDWORD	NameLen,
			   bool	Flag
			   );

DWORD
FindModuleLoadBase(
				   PCHAR	ModuleName,
				   DWORD	NameLen
				   );


void GetHookModuleName(DWORD dwBaseAddress, char *lpModuleName,ULONG pid);



//////////////////////////////////////////////////////////////////////////
#endif	___MODULE___