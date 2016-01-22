/*===================================================================
 * Filename reg.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: ע���ģ��
 *
 * Date:   2013-5-19 Original from Chinghoi
 *
 * Version: 1.0
 ==================================================================*/

#ifndef _REGISTRY_H_
#define _REGISTRY_H_


/* ͷ�ļ� ================================================= */

#include "ntdefine.h"


/* �ṹ���� ================================================ */


/* �������� ================================================ */

NTSTATUS
QueryKeyItemInfo( LPCWSTR KeyFullPath, PULONG  pNumber, ULONG Size );

NTSTATUS
EnumerateKey( LPCWSTR KeyFullPath, PVOID pBuffer, ULONG SubKey );

NTSTATUS
EnumerateValueKey( LPCWSTR KeyFullPath, PVOID pBuffer, ULONG Value );

NTSTATUS
DeleteValueKey(  LPCWSTR KeyFullPath, LPCWSTR Value );


#endif