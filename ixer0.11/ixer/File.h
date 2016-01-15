//////////////////////////////////////////////////////////////////////////
// File.h

#ifndef  ___FILE___
#define  ___FILE___  

#define AC_GENERIC_READ        0x120089
#define AC_GENERIC_WRITE       0x120196
#define AC_DELETE              0x110080
#define AC_READ_CONTROL        0x120080
#define AC_WRITE_DAC           0x140080
#define AC_WRITE_OWNER         0x180080
#define AC_GENERIC_ALL         0x1f01ff
#define AC_STANDARD_RIGHTS_ALL 0x1f0080


DWORD	
SetHandleAccess(
				HANDLE Handle,
				ACCESS_MASK GrantedAccess);



//////////////////////////////////////////////////////////////////////////
#endif