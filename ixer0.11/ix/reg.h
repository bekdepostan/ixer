/*===================================================================
 * Filename reg.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 注册表模块
 *
 * Date:   2013-5-19 Original from Chinghoi
 *
 * Version: 1.0
 ==================================================================*/

#ifndef _REGISTRY_H_
#define _REGISTRY_H_


/* 头文件 ================================================= */

#include "ntdefine.h"


/* 结构定义 ================================================ */


/* 函数声明 ================================================ */

NTSTATUS
QueryKeyItemInfo( LPCWSTR KeyFullPath, PULONG  pNumber, ULONG Size );

NTSTATUS
EnumerateKey( LPCWSTR KeyFullPath, PVOID pBuffer, ULONG SubKey );

NTSTATUS
EnumerateValueKey( LPCWSTR KeyFullPath, PVOID pBuffer, ULONG Value );

NTSTATUS
DeleteValueKey(  LPCWSTR KeyFullPath, LPCWSTR Value );


#endif