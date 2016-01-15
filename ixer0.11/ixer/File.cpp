//////////////////////////////////////////////////////////////////////////
// File.cpp

#include "StdAfx.h"



extern HANDLE g_hDriver;
//////////////////////////////////////////////////////////////////////////
DWORD	
SetHandleAccess(
				HANDLE Handle,  
				ACCESS_MASK GrantedAccess
				)
{
    DWORD   Bytes;
    DWORD   Buff[2];
    DWORD	Result = FALSE;
	
    Buff[0] = (DWORD)Handle;
    Buff[1] = GrantedAccess;
	
    Result = DeviceIoControl(g_hDriver,
							IOCTL_X_TRAIL_CHANGE_HANDLE,
							Buff,
							2*sizeof(DWORD),
							NULL,
							0,
							&Bytes,
							NULL);
	return Result;
}