/*===================================================================
 * Filename ioctl.h
 * 
 * Author: hoiker
 * Email:  hoiker@vip.qq.com
 *
 * Description:
 *
 * Date:
 *	build:
 *		Aug 11，2012
 *	revision:
 *		Jan 28，2013
 *
 * Version: 1.0
 ==================================================================*/

#ifndef _IOCTL_H_
#define _IOCTL_H_


/* 头文件 ================================================= */

//#include "global.h"


//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-0x7FFF(32767), and 0x8000(32768)-0xFFFF(65535)
// are reserved for use by customers.
//

#define FILE_DEVICE_IXER	0x8000

//
// Macro definition for defining IOCTL and FSCTL function control codes. Note
// that function codes 0-0x7FF(2047) are reserved for Microsoft Corporation,
// and 0x800(2048)-0xFFF(4095) are reserved for customers.
//

#define IXER_IOCTL_BASE	0x800

//
// The device driver IOCTLs
//

#define CTL_CODE_IXER(i)	\
	CTL_CODE(FILE_DEVICE_IXER, IXER_IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_IX_INIT				CTL_CODE_IXER(0)			//初始化
#define IOCTL_IX_FREE				CTL_CODE_IXER(1)			//退出

#define IOCTL_ENUM_PROCESS			CTL_CODE_IXER(2)			//枚举、检测进程
#define IOCTL_GET_PROCESS_LIST		CTL_CODE_IXER(3)			//获取进程数据列表
#define IOCTL_PROCESS_KILLER		CTL_CODE_IXER(4)			//结束进程
#define IOCTL_PROCESS_TERMINATOR	CTL_CODE_IXER(5)			//进程终结者(强制结束进程)

#define IOCTL_GET_Dll_LIST			CTL_CODE_IXER(6)			//枚举、检测进程Dll
#define IOCTL_DLL_TERMINATOR		CTL_CODE_IXER(7)			//卸载Dll

#define IOCTL_GET_THREAD_LIST		CTL_CODE_IXER(8)			//枚举进程线程
#define IOCTL_THREAD_TERMINATOR		CTL_CODE_IXER(9)			//线程终结者

#define IOCTL_ENUM_DRIVER			CTL_CODE_IXER(10)			//获取驱动数据列表
#define IOCTL_DRIVER_TERMINATOR		CTL_CODE_IXER(11)			//卸载驱动

#define IOCTL_GET_SSDT_INFO			CTL_CODE_IXER(12)			//获取SSDT数据
#define IOCTL_RESTORE_SSDT			CTL_CODE_IXER(13)			//恢复SSDT地址

#define IOCTL_GET_SHADOWSSDT_INFO	CTL_CODE_IXER(14)			//获取ShadowSSDT数据
#define IOCTL_RESTORE_SHADOW		CTL_CODE_IXER(15)			//恢复ShadowSSDT

#define IOCTL_GET_MSGHOOK_INFO		CTL_CODE_IXER(16)			//获取MSGHOOK数据

#define IOCTL_GET_DIR_NUM			CTL_CODE_IXER(17)			//获取文件夹个数
#define IOCTL_GET_DIR_INFO			CTL_CODE_IXER(18)			//获取文件夹下文件夹数据

#define IOCTL_DELETE_FILE			CTL_CODE_IXER(19)			//删除

#define IOCTL_GET_REGKEY_SIZE		CTL_CODE_IXER(20)			//获取键尺寸
#define IOCTL_GET_REGKEY			CTL_CODE_IXER(21)			//读取regKey
#define IOCTL_GET_REGVALUE			CTL_CODE_IXER(22)			//读取键值
#define IOCTL_DELETE_REGVALUE		CTL_CODE_IXER(23)			//删除键值

#define IOCTL_ENUM_TCP				CTL_CODE_IXER(24)			//枚举TCP
#define IOCTL_ENUM_UDP				CTL_CODE_IXER(25)			//枚举UDP


/* 宏定义 ================================================= */

#define IOCTL_TRANSFER_TYPE( _iocontrol)   (_iocontrol & 0x3)


//
// Name that Win32 front end will use to open the IXER device
//

#define IXER_WIN32_DEVICE_NAME_A	"\\\\.\\IXER"
#define IXER_WIN32_DEVICE_NAME_W	L"\\\\.\\IXER"
#define IXER_DEVICE_NAME_A			"\\Device\\IXER"
#define IXER_DEVICE_NAME_W			L"\\Device\\IXER"
#define IXER_DOS_DEVICE_NAME_A		"\\DosDevices\\IXER"
#define IXER_DOS_DEVICE_NAME_W		L"\\DosDevices\\IXER"

#ifdef _UNICODE
#define IXER_WIN32_DEVICE_NAME	IXER_WIN32_DEVICE_NAME_W
#define IXER_DEVICE_NAME		IXER_DEVICE_NAME_W
#define IXER_DOS_DEVICE_NAME	IXER_DOS_DEVICE_NAME_W
#else
#define IXER_WIN32_DEVICE_NAME	IXER_WIN32_DEVICE_NAME_A
#define IXER_DEVICE_NAME		IXER_DEVICE_NAME_A
#define IXER_DOS_DEVICE_NAME	IXER_DOS_DEVICE_NAME_A
#endif

#define MAXFUNLEN	64

#define MAXSSDTNUM  284
//////////////////////////////////////////////////////////////////////////
#endif