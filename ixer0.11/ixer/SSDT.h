//////////////////////////////////////////////////////////////////////////
// SSDT.h

#ifndef	___SSDT___
#define ___SSDT___


typedef struct {
    WORD    offset:12;
    WORD    type:4;
} IMAGE_FIXUP_ENTRY, *PIMAGE_FIXUP_ENTRY;

#pragma pack( push, 1 )
typedef struct _tagSSDTEntry {
    BYTE  byMov;	//0xb8
    DWORD dwIndex;
} SSDTENTRY;
#pragma pack( pop )

#define RVATOVA(base,offset) ((PVOID)((DWORD)(base)+(DWORD)(offset)))
#define	MOV_OPCODE	0xB8


DWORD
FindKiServiceTable(
				   HMODULE hModule,
				   DWORD dwKSDT
				   );

PDWORD
EnumSSDTNativeFunAddr(
					 PDWORD	NumOfAdd
					 );

PSSDT_NAME
EnumServiceFunName(
				   PDWORD	NumOfFunName
				   );

PDWORD
GetShadowSSDTNativeAddr(
					   PDWORD	NumOfFun
					   );


//////////////////////////////////////////////////////////////////////////
#endif