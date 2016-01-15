//////////////////////////////////////////////////////////////////////////
// IO_CTL.h

#ifndef  ___IO_CTL___
#define	 ___IO_CTL___


//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-0x7FFF(32767), and 0x8000(32768)-0xFFFF(65535)
// are reserved for use by customers.
//

#define FILE_DEVICE_X_DEVICE	0x8000

//
// Macro definition for defining IOCTL and FSCTL function control codes. Note
// that function codes 0-0x7FF(2047) are reserved for Microsoft Corporation,
// and 0x800(2048)-0xFFF(4095) are reserved for customers.
//

#define X_DEVICE_IOCTL_BASE	0x800

//
// The device driver IOCTLs
//

#define CTL_CODE_X_DEVICE(i)	\
CTL_CODE(FILE_DEVICE_X_DEVICE, X_DEVICE_IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_X_TRAIL_DEVICE_INITIALIZE		CTL_CODE_X_DEVICE(0)	//初始化
#define IOCTL_X_TRAIL_DEVICE_EXIT			CTL_CODE_X_DEVICE(1)	//退出并释放资源

#define IOCTL_X_TRAIL_GUARD_ENABLE			CTL_CODE_X_DEVICE(2)	//开启防御系统
#define IOCTL_X_TRAIL_GUARD_DISENABLE		CTL_CODE_X_DEVICE(3)	//关闭防御系统

#define IOCTL_X_TRAIL_MAP_PROCESS_LIST		CTL_CODE_X_DEVICE(4)	//映射ProcessList到Ring3
#define IOCTL_X_TRAIL_MAP_THREAD_LIST		CTL_CODE_X_DEVICE(5)	//映射进程ThreadList到Ring3
#define IOCTL_X_TRAIL_MAP_MODULE_LIST		CTL_CODE_X_DEVICE(6)	//映射进程ModuleList到Ring3

#define IOCTL_X_TRAIL_PROCESS_KILLER		CTL_CODE_X_DEVICE(7)	//结束进程
#define IOCTL_X_TRAIL_PROCESS_TERMINATOR	CTL_CODE_X_DEVICE(8)	//强制结束进程

#define IOCTL_X_TRAIL_THREAD_TERMINATOR		CTL_CODE_X_DEVICE(9)	//强制结束线程
#define IOCTL_X_TRAIL_MODULE_TERMINATOR		CTL_CODE_X_DEVICE(10)	//卸载模块

#define IOCTL_X_TRAIL_GET_SSDT_INFO			CTL_CODE_X_DEVICE(11)	//获取SSDT数据
#define IOCTL_X_TRAIL_SSDT_RESTORE			CTL_CODE_X_DEVICE(12)	//恢复SSDT

#define IOCTL_X_TRAIL_GET_SHADOWSSDT_INFO	CTL_CODE_X_DEVICE(13)	//获取ShadowSSDT数据
#define IOCTL_X_TRAIL_SHADOW_RESTORE		CTL_CODE_X_DEVICE(14)	//恢复ShadowSSDT

#define IOCTL_X_TRAIL_GET_DIR_NUM_INFO		CTL_CODE_X_DEVICE(15)	//获取文件夹个数
#define IOCTL_X_TRAIL_GET_DIR_INFO			CTL_CODE_X_DEVICE(16)	//获取文件夹下文件夹数据

#define IOCTL_X_TRAIL_CHANGE_HANDLE			CTL_CODE_X_DEVICE(17)	//修改句柄
#define IOCTL_X_TRAIL_FILE_GRINDER			CTL_CODE_X_DEVICE(18)	//文件粉碎机

#define IOCTL_X_TRAIL_ENUMTCP				CTL_CODE_X_DEVICE(19)	//枚举TCP
#define IOCTL_X_TRAIL_ENUMUDP				CTL_CODE_X_DEVICE(20)	//枚举UDP

#define IOCTL_X_TRAIL_FLUSHHASH				CTL_CODE_X_DEVICE(21)	//刷新哈希表数据
#define IOCTL_X_TRAIL_ENUMKRNLMOD			CTL_CODE_X_DEVICE(22)	//枚举内核模块

//
// Name that Win32 front end will use to open the X_Device device
//

#define X_DEVICE_WIN32_DEVICE_NAME_A	"\\\\.\\X_Device"
#define X_DEVICE_WIN32_DEVICE_NAME_W	L"\\\\.\\X_Device"
#define X_DEVICE_DEVICE_NAME_A			"\\Device\\X_Device"
#define X_DEVICE_DEVICE_NAME_W			L"\\Device\\X_Device"		//设备名(Unicode)
#define X_DEVICE_DOS_DEVICE_NAME_A		"\\DosDevices\\X_Device"
#define X_DEVICE_DOS_DEVICE_NAME_W		L"\\DosDevices\\X_Device"	//Dos设备名(Unicode)

#ifdef _UNICODE
#define X_DEVICE_WIN32_DEVICE_NAME	X_DEVICE_WIN32_DEVICE_NAME_W
#define X_DEVICE_DEVICE_NAME		X_DEVICE_DEVICE_NAME_W
#define X_DEVICE_DOS_DEVICE_NAME	X_DEVICE_DOS_DEVICE_NAME_W
#else
#define X_DEVICE_WIN32_DEVICE_NAME	X_DEVICE_WIN32_DEVICE_NAME_A
#define X_DEVICE_DEVICE_NAME		X_DEVICE_DEVICE_NAME_A
#define X_DEVICE_DOS_DEVICE_NAME	X_DEVICE_DOS_DEVICE_NAME_A
#endif


//
// A structure representing the instance information associated with
// a particular device
//

typedef struct _DEVICE_EXTENSION			//设备扩展区内存结构
{
	ULONG  StateVariable;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

// The maximum registry path length that will be copied
//
#define MAXPATHLEN     1024
#define MAXNAMELEN     64

//
// Maximum amount of memory that will be grabbed
//
#define MAXMEM          1000000

#define MAXERRSTRINGLEN 256

#define MAXFUNLEN	64

#define MAXSSDTNUM  284


#define END_FLAG 0xFFFFFFFF



//////////////////////////////////////////////////////////////////////////
#endif