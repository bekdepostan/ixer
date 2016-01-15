/*===================================================================
 * Filename: common.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 通用模块，公用的函数在这里定义
 *
 * Date:   2013-5-16 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <Dbghelp.h>



/* 头文件 =================================================*/


BOOL InitSymbolsHandler();

BOOL	EnumSyms( char		*imageName,
				  ULONG		moduleBase,
				  PSYM_ENUMERATESYMBOLS_CALLBACK EnumRoutine,
				  PVOID		Context );

BOOL LoadSymModule( char *imageName, ULONG moduleBase );
BOOLEAN CALLBACK EnumSymRoutine(
								PSYMBOL_INFO pSymInfo,
								ULONG     SymSize,
								PVOID     UserContext );

BOOL GetFunctionBelongingModuleName(DWORD ulPid, LPVOID lpvBase, LPSTR lpFilename, DWORD nSize);
BOOL AdjustPrivileges( LPCTSTR lpName );
//////////////////////////////////////////////////////////////////////////
#endif