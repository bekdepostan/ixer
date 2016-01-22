/*===================================================================
 * Filename file.h
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: ��Ϣ����
 *
 * Date:   2013-5-21 Original from Chinghoi
 *         
 * Version: 1.0
 ==================================================================*/

#ifndef _FILE_H_
#define _FILE_H_


/* ����ȫ�ֱ��� ============================================ */
KSPIN_LOCK				spinLock;
/* ��������������Ϣ��ȥ�� */
#define _CHECKED

/* �������� ================================================ */
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