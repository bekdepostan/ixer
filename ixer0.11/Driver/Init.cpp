//////////////////////////////////////////////////////////////////////////
// Initialize.c

#include "precomp.h"
#include "func.h"
#include "WinOSVersion.h"
#include "Init.h"



/*========================================================================
*
* ������:	InitMySystem
*
* ����:		��
*
* ��������:	��ʼ���������ж�windows OS�汾����ʼ���������ں˽ṹƫ����
* �� DriverEntry �����е��ã�����������:system
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS InitMySystem()
{
	NTSTATUS status;

	//////////////////////////////////////////////////////////////////////////
	// WindowsVersionģ���ʼ��
	// ���ݲ���ϵͳ�Ĳ�ͬ����ʼ���ṹ��ƫ��.

	// ��ȡ����ϵͳ�汾
	status = GetWindowsVersion();
	if ( !NT_SUCCESS(status) )
		return status;

	//��ʼ���ں˱�����ƫ����
	status = InitOffsetbyWindowsVersion();
	if ( !NT_SUCCESS(status) )
		return status;

	GetPspTerminateThreadByPointer();
	GetPspExitThread();

	return STATUS_SUCCESS;
} 
