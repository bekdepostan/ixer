//////////////////////////////////////////////////////////////////////////
// KrnlMod.h

#ifndef _KRNLMOD_H_
#define _KRNLMOD_H_

#include "..\Common\Surface.h"

extern POBJECT_TYPE IoDriverObjectType;

NTKERNELAPI
NTSTATUS
ObReferenceObjectByName(
						IN PUNICODE_STRING ObjectName,
						IN ULONG Attributes,
						IN PACCESS_STATE PassedAccessState OPTIONAL,
						IN ACCESS_MASK DesiredAccess OPTIONAL,
						IN POBJECT_TYPE ObjectType,
						IN KPROCESSOR_MODE AccessMode,
						IN OUT PVOID ParseContext OPTIONAL,
						OUT PVOID *Object);

BOOLEAN StoreOrCheckKrnlMod(BOOLEAN storeOrCheck, PWCHAR pszDrvName, ULONG ulImageBase, ULONG ulImageSize, ULONG pDrvObject, PWCHAR pwDosPath, ULONG LoadCount, PULONG numOfDrv, PDRVMOD_ITEM pDrvItem);
NTSTATUS GetDrvObjectByName(char *pszDrvName, PULONG pDrvObject);
NTSTATUS CheckHiddenDrvByPsLoadedModuleList(PULONG numOfDrv, PDRVMOD_ITEM pDrvItem);
NTSTATUS EnumKrnlModByZwQuerySysInfo(PULONG numOfDrv, PDRVMOD_ITEM pDrvItem);
PDRVMOD_ITEM GetKrnlModuleList(PULONG numOfDrv);
//////////////////////////////////////////////////////////////////////////
#endif