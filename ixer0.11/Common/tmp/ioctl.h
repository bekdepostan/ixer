/*===================================================================
 * FileName:
 *		ioctl.h
 * 
 * Author: hoiker		Email:  hoiker@vip.qq.com
 *
 * Description:
 *
 * Date:
 *		build:
 *			Aug 11，2012
 *		revision:
 *			Jan 28，2013
 *
 * Version: 1.0
 ==================================================================*/

#ifndef _IOCTL_H_
#define _IOCTL_H_


/* 头文件 ================================================= */


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

/* 宏定义 ================================================= */

#define IOCTL_TRANSFER_TYPE( _iocontrol)   (_iocontrol & 0x3)

#define IOCTL_X_TRAIL_INITIALIZE		CTL_CODE_IXER(0)	//初始化
#define IOCTL_X_TRAIL_EXIT				CTL_CODE_IXER(1)	//释放资源



//
// Name that Win32 front end will use to open the IXER device
//

#define IXER_WIN32_DEVICE_NAME_A	"\\\\.\\XTR"
#define IXER_WIN32_DEVICE_NAME_W	L"\\\\.\\XTR"
#define IXER_DEVICE_NAME_A			"\\Device\\XTR"
#define IXER_DEVICE_NAME_W			L"\\Device\\XTR"
#define IXER_DOS_DEVICE_NAME_A		"\\DosDevices\\XTR"
#define IXER_DOS_DEVICE_NAME_W		L"\\DosDevices\\XTR"

#ifdef _UNICODE
#define IXER_WIN32_DEVICE_NAME	IXER_WIN32_DEVICE_NAME_W
#define IXER_DEVICE_NAME		IXER_DEVICE_NAME_W
#define IXER_DOS_DEVICE_NAME	IXER_DOS_DEVICE_NAME_W
#else
#define IXER_WIN32_DEVICE_NAME	IXER_WIN32_DEVICE_NAME_A
#define IXER_DEVICE_NAME		IXER_DEVICE_NAME_A
#define IXER_DOS_DEVICE_NAME	IXER_DOS_DEVICE_NAME_A
#endif


//////////////////////////////////////////////////////////////////////////
#endif