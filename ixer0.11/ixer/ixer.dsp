# Microsoft Developer Studio Project File - Name="ixer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ixer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ixer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ixer.mak" CFG="ixer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ixer - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ixer - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ixer - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"E:\雪枫\代表作\ixer0.11\Bin\ixer.exe"

!ELSEIF  "$(CFG)" == "ixer - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /out:"E:\雪枫\代表作\ixer0.11\Bin\ixer.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ixer - Win32 Release"
# Name "ixer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "UIClass"

# PROP Default_Filter ""
# Begin Group "SortListCtrl"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UIClass\SortListCtrl\SortHeaderCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\UIClass\SortListCtrl\SortHeaderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\UIClass\SortListCtrl\SortListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\UIClass\SortListCtrl\SortListCtrl.h
# End Source File
# End Group
# Begin Group "Hyperlink"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\UIClass\Hyperlink\HyperLink.cpp
# End Source File
# Begin Source File

SOURCE=.\UIClass\Hyperlink\Hyperlink.h
# End Source File
# End Group
# End Group
# Begin Group "UIModule"

# PROP Default_Filter ""
# Begin Group "ProcessMgr"

# PROP Default_Filter ""
# Begin Group "DllMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\moduleMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\moduleMgr.h
# End Source File
# End Group
# Begin Group "ThreadMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\threadMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\threadMgr.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\processMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\processMgr.h
# End Source File
# End Group
# Begin Group "FileMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\FileMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\FileMgr.h
# End Source File
# End Group
# Begin Group "NetWorkMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\NetWorkMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\NetWorkMgr.h
# End Source File
# End Group
# Begin Group "RegMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\RegMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\RegMgr.h
# End Source File
# End Group
# Begin Group "DrvMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DrvModMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\DrvModMgr.h
# End Source File
# End Group
# Begin Group "ssdtMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ShadowSSDTMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\ShadowSSDTMgr.h
# End Source File
# Begin Source File

SOURCE=.\SSDTMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\SSDTMgr.h
# End Source File
# End Group
# Begin Group "MsghookMgr"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MsgHookMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\MsgHookMgr.h
# End Source File
# End Group
# Begin Group "IxAboutDlg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IxAboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\IxAboutDlg.h
# End Source File
# End Group
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\common.cpp
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=..\Common\ioctl.h
# End Source File
# Begin Source File

SOURCE=..\Common\Surface.h
# End Source File
# End Group
# Begin Group "DataClass"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DataClass\TypeStruct.h
# End Source File
# End Group
# Begin Group "FunModule"

# PROP Default_Filter ""
# Begin Group "Dll_func"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Module.cpp
# End Source File
# Begin Source File

SOURCE=.\Module.h
# End Source File
# End Group
# Begin Group "Mem_Fun"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Memory.cpp
# End Source File
# Begin Source File

SOURCE=.\Memory.h
# End Source File
# End Group
# Begin Group "PE_Fun"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\PE.cpp
# End Source File
# Begin Source File

SOURCE=.\PE.h
# End Source File
# End Group
# Begin Group "OS_Fun"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\OS.cpp
# End Source File
# Begin Source File

SOURCE=.\OS.h
# End Source File
# End Group
# Begin Group "Pro_Fun"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\rootkit.h
# End Source File
# End Group
# Begin Group "API_Fun"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\API.cpp
# End Source File
# Begin Source File

SOURCE=.\API.h
# End Source File
# End Group
# Begin Group "MsgHook_Fun"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MsgHook.h
# End Source File
# End Group
# Begin Group "Reg_Fun"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Reg.cpp
# End Source File
# Begin Source File

SOURCE=.\Reg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Net.cpp
# End Source File
# Begin Source File

SOURCE=.\Net.h
# End Source File
# End Group
# Begin Group "FunClass"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\WindowsVersion.cpp
# End Source File
# Begin Source File

SOURCE=.\WindowsVersion.h
# End Source File
# End Group
# Begin Source File

SOURCE=".\ixer.cpp"
# End Source File
# Begin Source File

SOURCE=".\ixer.rc"
# End Source File
# Begin Source File

SOURCE=".\ixerDlg.cpp"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=".\ixer.h"
# End Source File
# Begin Source File

SOURCE=".\ixerDlg.h"
# End Source File
# Begin Source File

SOURCE=.\NtUnDoc.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\35floppy.ico
# End Source File
# Begin Source File

SOURCE=.\res\AtUAC.bin
# End Source File
# Begin Source File

SOURCE=.\res\CDdrive.ico
# End Source File
# Begin Source File

SOURCE=.\res\Clsdfold.ico
# End Source File
# Begin Source File

SOURCE=.\res\Computer.ico
# End Source File
# Begin Source File

SOURCE=.\res\default.ico
# End Source File
# Begin Source File

SOURCE=.\res\default1.bin
# End Source File
# Begin Source File

SOURCE=.\res\Drive.ico
# End Source File
# Begin Source File

SOURCE=.\res\Drv.ico
# End Source File
# Begin Source File

SOURCE=.\res\dword.ico
# End Source File
# Begin Source File

SOURCE=.\res\floppy.ico
# End Source File
# Begin Source File

SOURCE=".\res\ixer.ico"
# End Source File
# Begin Source File

SOURCE=".\res\ixer.rc2"
# End Source File
# Begin Source File

SOURCE=.\res\Mycomp.ico
# End Source File
# Begin Source File

SOURCE=.\res\Openfold.ico
# End Source File
# Begin Source File

SOURCE=.\res\regClsdfold.ico
# End Source File
# Begin Source File

SOURCE=.\res\regComputer.ico
# End Source File
# Begin Source File

SOURCE=.\res\regDWORD.ico
# End Source File
# Begin Source File

SOURCE=.\res\regOpenfold.ico
# End Source File
# Begin Source File

SOURCE=.\res\regSZ.ico
# End Source File
# Begin Source File

SOURCE=.\res\SETUPFILE.ico
# End Source File
# Begin Source File

SOURCE=.\res\sz.ico
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
