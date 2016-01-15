; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CIxAboutDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "ixer.h"

ClassCount=22
Class1=CixerApp
Class2=CixerDlg
Class3=CAboutDlg
Class4=CProcessMgr
Class5=CModuleMgr
Class6=CThreadMgr
Class7=CLRing0hooks
Class8=CSSDTMgr

ResourceCount=34
Resource1=IDD_LSSDT_MGR
Resource2=IDR_MAINFRAME
Resource3=IDD_LNETWORK_MGR
Resource4=IDD_IXER_DIALOG
Resource5=IDD_EDIT_DWVALUE
Resource6=IDD_LREG_MGR
Resource7=IDD_LKRNLMOD_MGR
Resource8=IDD_LFILE_MGR
Resource11=IDD_NEW_THREAD_MGR_DLG
Resource9=IDD_LPROC_MGR
Resource10=IDR_NET_RPOP
Resource12=IDR_SSDT_RPOP
Class9=CShadowSSDTMgr
Resource13=IDR_FILE_RPOP
Resource14=IDD_LMODULE_MGR
Resource15=IDD_ixer_DIALOG
Class10=CLRing3Hooks
Class11=CMsgHookMgr
Resource16=IDD_NEW_DLL_MGR_DLG
Class12=CFileMgr
Resource17=IDD_LMSGHOOK_MGR
Resource18=IDD_LABOUT_DLG
Resource19=IDR_MODULE_RPOP
Class13=CServiceMgr
Resource20=IDR_SRV_RPOP
Resource21=IDD_ABOUTBOX
Class14=CNetWorkMgr
Resource22=IDD_LSRV_MGR
Resource23=IDR_PROC_RPOP
Class15=CRegMgr
Resource24=IDD_EDIT_SZVALUE
Resource25=IDD_LPROCESS_MGR
Class16=CEditDwValue
Resource26=IDR_DLL_RPOP
Class17=CEditSzValue
Resource27=IDD_LDRV_MGR
Class18=CAddNewKeyItem
Resource28=IDR_SHADOW_RPOP
Resource29=IDR_THREAD_RPOP
Class19=CRegSearch
Resource30=IDR_MSGHOOK_RPOP
Class20=CDrvModMgr
Resource31=IDR_REG_RPOP
Resource32=IDD_LSERVICE_MGR
Class21=CLNtosKrnl
Resource33=IDD_LSHADOW_MGR
Class22=CIxAboutDlg
Resource34=IDR_DRV_RPOP

[CLS:CixerApp]
Type=0
HeaderFile=ixer.h
ImplementationFile=ixer.cpp
Filter=N

[CLS:CixerDlg]
Type=0
HeaderFile=ixerDlg.h
ImplementationFile=ixerDlg.cpp
Filter=D
LastObject=IDM_CONNECT_PRO_KILLER
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=ixerDlg.h
ImplementationFile=ixerDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_X_TRAIL_DIALOG]
Type=1
Class=CixerDlg
ControlCount=1
Control1=IDC_TAB_MAIN_VIEW,SysTabControl32,1342177280

[CLS:CModuleMgr]
Type=0
HeaderFile=ModuleMgr.h
ImplementationFile=ModuleMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=CModuleMgr
VirtualFilter=dWC

[CLS:CProcessMgr]
Type=0
HeaderFile=processmgr.h
ImplementationFile=processmgr.cpp
BaseClass=CDialog
LastObject=IDM_PRO_TERMINATOR
Filter=D
VirtualFilter=dWC

[MNU:IDR_MODULE_RPOP]
Type=1
Class=CModuleMgr
Command1=IDM_REFRESH_MOD_LIST
Command2=IDM_MOD_TERMINATOR
CommandCount=2

[CLS:CThreadMgr]
Type=0
HeaderFile=ThreadMgr.h
ImplementationFile=ThreadMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=CThreadMgr
VirtualFilter=dWC

[MNU:IDR_THREAD_RPOP]
Type=1
Class=CThreadMgr
Command1=IDM_REFRESH_TRD_LIST
Command2=IDM_TRD_TERMINATOR
CommandCount=2

[CLS:CLRing0hooks]
Type=0
HeaderFile=LRing0hooks.h
ImplementationFile=LRing0hooks.cpp
BaseClass=CDialog
LastObject=CLRing0hooks
Filter=D
VirtualFilter=dWC

[DLG:IDD_LPROCESS_MGR]
Type=1
Class=CProcessMgr
ControlCount=2
Control1=IDC_STATIC_PROCESS,static,1342308352
Control2=IDC_PROCESS_LISTS,SysListView32,1342242817

[DLG:IDD_LSSDT_MGR]
Type=1
Class=CSSDTMgr
ControlCount=2
Control1=IDC_SSDT_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_SSDT,static,1342308352

[CLS:CSSDTMgr]
Type=0
HeaderFile=SSDTMgr.h
ImplementationFile=SSDTMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=CSSDTMgr
VirtualFilter=dWC

[MNU:IDR_SSDT_RPOP]
Type=1
Class=CSSDTMgr
Command1=IDM_REFRESH_SSDT_LIST
Command2=IDM_SSDT_RESTORE
CommandCount=2

[CLS:CShadowSSDTMgr]
Type=0
HeaderFile=ShadowSSDTMgr.h
ImplementationFile=ShadowSSDTMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=IDM_REFRESH_SHADOW_LIST
VirtualFilter=dWC

[MNU:IDR_SHADOW_RPOP]
Type=1
Class=CShadowSSDTMgr
Command1=IDM_REFRESH_SHADOW_LIST
Command2=IDM_SHADOW_RESTORE
CommandCount=2

[DLG:IDD_LSHADOW_MGR]
Type=1
Class=CShadowSSDTMgr
ControlCount=2
Control1=IDC_SHADOWSSDT_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_SHADOWSSDT,static,1342312448

[DLG:IDD_LMSGHOOK_MGR]
Type=1
Class=CMsgHookMgr
ControlCount=2
Control1=IDC_MSG_HOOK_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_MSGHOOK,static,1342312448

[CLS:CLRing3Hooks]
Type=0
HeaderFile=LRing3Hooks.h
ImplementationFile=LRing3Hooks.cpp
BaseClass=CDialog
Filter=D
LastObject=CLRing3Hooks
VirtualFilter=dWC

[CLS:CMsgHookMgr]
Type=0
HeaderFile=MsgHookMgr.h
ImplementationFile=MsgHookMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=IDM_REFRESH_MSGHOOK_LIST
VirtualFilter=dWC

[DLG:IDD_LFILE_MGR]
Type=1
Class=CFileMgr
ControlCount=3
Control1=IDC_FILE_TREE,SysTreeView32,1342244391
Control2=IDC_FILE_LISTS,SysListView32,1342242817
Control3=IDC_STATIC_FILE,static,1342308352

[CLS:CFileMgr]
Type=0
HeaderFile=FileMgr.h
ImplementationFile=FileMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=IDM_REFRESH_FLE_LIST
VirtualFilter=dWC

[MNU:IDR_FILE_RPOP]
Type=1
Class=CFileMgr
Command1=IDM_REFRESH_FLE_LIST
Command2=IDM_DEL_FILE
CommandCount=2

[MNU:IDR_MSGHOOK_RPOP]
Type=1
Class=CMsgHookMgr
Command1=IDM_REFRESH_MSGHOOK_LIST
Command2=IDM_MSG_UNHOOK
CommandCount=2

[DLG:IDD_LSERVICE_MGR]
Type=1
Class=CServiceMgr
ControlCount=2
Control1=IDC_SERVICE_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_SERVICE,static,1342312448

[CLS:CServiceMgr]
Type=0
HeaderFile=ServiceMgr.h
ImplementationFile=ServiceMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=CServiceMgr
VirtualFilter=dWC

[MNU:IDR_SRV_RPOP]
Type=1
Class=CServiceMgr
Command1=IDM_REFRESH_SRV_LIST
Command2=IDM_SRV_START
Command3=IDM_SRV_STOP
Command4=IDM_SRV_PAUSE
Command5=IDM_SRV_RESTORE
Command6=IDM_SRV_DELETE
Command7=IDM_SRV_AUTO
Command8=IDM_SRV_MANUAL
Command9=IDM_SRV_DISABLE
CommandCount=9

[DLG:IDD_LNETWORK_MGR]
Type=1
Class=CNetWorkMgr
ControlCount=2
Control1=IDC_NETWORK_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_NETWORK,static,1342312448

[CLS:CNetWorkMgr]
Type=0
HeaderFile=NetWorkMgr.h
ImplementationFile=NetWorkMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=CNetWorkMgr
VirtualFilter=dWC

[MNU:IDR_NET_RPOP]
Type=1
Class=CNetWorkMgr
Command1=IDM_REFRESH_NET_LIST
Command2=IDM_CONNECT_PRO_KILLER
CommandCount=2

[DLG:IDD_LREG_MGR]
Type=1
Class=CRegMgr
ControlCount=3
Control1=IDC_REG_LISTS,SysListView32,1342242817
Control2=IDC_REG_TREE,SysTreeView32,1342247975
Control3=IDC_STATIC,static,1342308352

[CLS:CRegMgr]
Type=0
HeaderFile=RegMgr.h
ImplementationFile=RegMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=CRegMgr
VirtualFilter=dWC

[MNU:IDR_REG_RPOP]
Type=1
Class=CRegMgr
Command1=IDM_REFRESH_REG_LIST
Command2=IDM_REG_DELETE
CommandCount=2

[DLG:IDD_EDIT_DWVALUE]
Type=1
Class=CEditDwValue
ControlCount=9
Control1=IDOK,button,1342242816
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_STATIC,static,1342308352
Control5=IDC_EDIT_DWVALUE,edit,1350631552
Control6=IDC_STATIC,button,1342177287
Control7=IDC_RADIO_HEX,button,1342242825
Control8=IDC_RADIO_DEC,button,1342242825
Control9=IDC_EDIT_DWNAME,edit,1350633600

[CLS:CEditDwValue]
Type=0
HeaderFile=EditDwValue.h
ImplementationFile=EditDwValue.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_EDIT_DWNAME
VirtualFilter=dWC

[DLG:IDD_EDIT_SZVALUE]
Type=1
Class=CEditSzValue
ControlCount=6
Control1=IDOK,button,1342242816
Control2=IDCANCEL,button,1342242816
Control3=IDC_STATIC,static,1342308352
Control4=IDC_EDIT_SZNAME,edit,1350633600
Control5=IDC_STATIC,static,1342308352
Control6=IDC_EDIT_SZVALUE,edit,1350631552

[CLS:CEditSzValue]
Type=0
HeaderFile=EditSzValue.h
ImplementationFile=EditSzValue.cpp
BaseClass=CDialog
Filter=D
LastObject=CEditSzValue
VirtualFilter=dWC

[CLS:CAddNewKeyItem]
Type=0
HeaderFile=AddNewKeyItem.h
ImplementationFile=AddNewKeyItem.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_EDIT_ITEMNAME
VirtualFilter=dWC

[CLS:CRegSearch]
Type=0
HeaderFile=RegSearch.h
ImplementationFile=RegSearch.cpp
BaseClass=CDialog
Filter=D
LastObject=CRegSearch
VirtualFilter=dWC

[CLS:CDrvModMgr]
Type=0
HeaderFile=DrvModMgr.h
ImplementationFile=DrvModMgr.cpp
BaseClass=CDialog
Filter=D
LastObject=CDrvModMgr
VirtualFilter=dWC

[DLG:IDD_LMODULE_MGR]
Type=1
Class=CDrvModMgr
ControlCount=2
Control1=IDC_KERNAL_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_KERNAL,static,1342312448

[DLG:IDD_LKRNLMOD_MGR]
Type=1
Class=?
ControlCount=2
Control1=IDC_KRNLMOD_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_KRNLMOD,static,1342312448

[MNU:IDR_DRV_RPOP]
Type=1
Class=CDrvModMgr
Command1=IDM_REFRESH_DRV_LIST
Command2=IDM_DELETE_DRVFILE
Command3=IDM_COPY_DRVNAME
Command4=IDM_COPY_DRVPATH
Command5=IDM_GPS_DRVFILE
CommandCount=5

[MNU:IDR_PROC_RPOP]
Type=1
Class=CProcessMgr
Command1=IDM_REFRESH_PRO_LIST
Command2=IDM_PRO_THREAD_LIST
Command3=IDM_PRO_MODULE_LIST
Command4=IDM_PROC_KILLER
Command5=IDM_COPY_PROCNAME
Command6=IDM_COPY_PROCPATH
Command7=IDM_GPS_PROCFILE
CommandCount=7

[MNU:IDR_DLL_RPOP]
Type=1
Class=?
Command1=IDM_REFRESH_MOD_LIST
Command2=IDM_MOD_TERMINATOR
CommandCount=2

[DLG:IDD_NEW_THREAD_MGR_DLG]
Type=1
Class=CThreadMgr
ControlCount=1
Control1=IDC_THREAD_LISTS,SysListView32,1342242817

[DLG:IDD_NEW_DLL_MGR_DLG]
Type=1
Class=CModuleMgr
ControlCount=1
Control1=IDC_MOUDLE_LISTS,SysListView32,1342242817

[DLG:IDD_LDRV_MGR]
Type=1
Class=CDrvModMgr
ControlCount=2
Control1=IDC_DRVMOD_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_DRVMOD,static,1342312448

[DLG:IDD_LPROC_MGR]
Type=1
Class=CProcessMgr
ControlCount=2
Control1=IDC_STATIC_PROCESS,static,1342308352
Control2=IDC_PROCESS_LISTS,SysListView32,1342242817

[DLG:IDD_LSRV_MGR]
Type=1
Class=?
ControlCount=2
Control1=IDC_SERVICE_LISTS,SysListView32,1342242817
Control2=IDC_STATIC_SERVICE,static,1342312448

[CLS:CLNtosKrnl]
Type=0
HeaderFile=LNtosKrnl.h
ImplementationFile=LNtosKrnl.cpp
BaseClass=CDialog
Filter=D
LastObject=CLNtosKrnl
VirtualFilter=dWC

[DLG:IDD_IXER_DIALOG]
Type=1
Class=?
ControlCount=1
Control1=IDC_TAB_MAIN_VIEW,SysTabControl32,1342177280

[DLG:IDD_LABOUT_DLG]
Type=1
Class=CIxAboutDlg
ControlCount=8
Control1=IDC_STATIC,button,1342177287
Control2=IDC_STATIC,button,1342177287
Control3=IDC_STATIC,static,1342308352
Control4=IDC_BLOG,static,1342308352
Control5=IDC_MICRO_BLOG,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352
Control8=IDC_EMIAL,edit,1342244992

[CLS:CIxAboutDlg]
Type=0
HeaderFile=IxAboutDlg.h
ImplementationFile=IxAboutDlg.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_MICRO_BLOG
VirtualFilter=dWC

