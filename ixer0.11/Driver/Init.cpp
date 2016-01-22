//////////////////////////////////////////////////////////////////////////
// Initialize.c

#include "precomp.h"
#include "func.h"
#include "WinOSVersion.h"
#include "Init.h"



/*========================================================================
*
* 函数名:	InitMySystem
*
* 参数:		无
*
* 功能描述:	初始化函数，判断windows OS版本，初始化环境、内核结构偏移量
* 在 DriverEntry 函数中调用，进程上下文:system
*
* 返回值:	NTSTATUS
*
=========================================================================*/
NTSTATUS InitMySystem()
{
	NTSTATUS status;

	//////////////////////////////////////////////////////////////////////////
	// WindowsVersion模块初始化
	// 根据操作系统的不同，初始化结构体偏移.

	// 获取操作系统版本
	status = GetWindowsVersion();
	if ( !NT_SUCCESS(status) )
		return status;

	//初始化内核变量的偏移量
	status = InitOffsetbyWindowsVersion();
	if ( !NT_SUCCESS(status) )
		return status;

	GetPspTerminateThreadByPointer();
	GetPspExitThread();

	return STATUS_SUCCESS;
} 
