/*===================================================================
 * Filename file.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 消息钩子
 *
 * Date:   2013-5-21 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#ifndef _FILE_H_
#define _FILE_H_


/* 定义全局变量 ============================================ */
KSPIN_LOCK				spinLock;
/* 如果不输出调试信息则去掉 */
#define _CHECKED

/* 函数声明 ================================================ */
PVOID	QueryDirectoryFileInfo( PCHAR pDirName, PULONG dwRetSize );

NTSTATUS 
EventCompletion( IN PDEVICE_OBJECT DeviceObject, 
				IN PIRP Irp, 
				IN PVOID Context );

PDEVICE_OBJECT
IxIoGetRelatedDeviceObject( IN PFILE_OBJECT FileObject );

PDEVICE_OBJECT
__stdcall
IxIoGetAttachedDevice( PDEVICE_OBJECT DeviceObject );

NTSTATUS
IxDeleteFile( LPCWSTR FileFullPath );

NTSTATUS
StripFileAttributes( HANDLE FileHandle );

NTSTATUS
DeleteFileByHandle( HANDLE FileHandle );

NTSTATUS
SetFileCompletion( IN PDEVICE_OBJECT DeviceObject,
					IN PIRP Irp,
				  IN PVOID Context );
//////////////////////////////////////////////////////////////////////////
#endif