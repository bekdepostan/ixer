//////////////////////////////////////////////////////////////////////////
//头文件
//////////////////////////////////////////////////////////////////////////
#ifndef  ___WindowsVersion___
#define  ___WindowsVersion___ 

//操作系统标识 枚举类型
typedef enum WIN_VER_DETAIL {
    WINDOWS_VERSION_NONE,       // 0
	WINDOWS_VERSION_2K,
	WINDOWS_VERSION_XP,
	WINDOWS_VERSION_2K3,
	WINDOWS_VERSION_2K3_SP1_SP2,
	WINDOWS_VERSION_VISTA,
	WINDOWS_VERSION_7
} WIN_VER_DETAIL;



NTSTATUS GetWindowsVersion();
NTSTATUS InitOffsetbyWindowsVersion();

#endif