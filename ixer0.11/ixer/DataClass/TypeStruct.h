//////////////////////////////////////////////////////////////////////////
//Í·ÎÄ¼þ
//////////////////////////////////////////////////////////////////////////
#ifndef  ___TYPE_STRUCT___
#define	 ___TYPE_STRUCT___

typedef  PVOID  PEPROCESS;

typedef struct TIME_FIELDS {
    short  Year;        // range [1601...]
    short  Month;       // range [1..12]
    short  Day;         // range [1..31]
    short  Hour;        // range [0..23]
    short  Minute;      // range [0..59]
    short  Second;      // range [0..59]
    short  Milliseconds;// range [0..999]
    short  Weekday;     // range [0..6] == [Sunday..Saturday]
} TIME_FIELDS;


typedef enum _GUI_LIST_TYPE{
	GUILT_NONE=0,
		GUILT_BehaviorList,
		GUILT_MBehaviorList,
		GUILT_RamiList,
		GUILT_VirusList,
		GUILT_ProcessList,
		GUILT_FileList,
		GUILT_FileDeleteList,
		GUILT_ExploitList,
		GUILT_ThreadList,
		GUILT_ModuleList,
		GUILT_SSDTList,
		GUILT_FSDList,
		GUILT_NETList,
		GUILT_SoftwareList,
		GUILT_SystemServiceList,
		GUILT_MessageHookList		
}GUI_LIST_TYPE;

typedef	struct KernelStruct
{
	char	KernelName[24];
	DWORD	KernelBase;
	DWORD	Size;
}KERNELINFO,*PKERNELINFO;

typedef	struct InterIDTInfo
{
	BOOL	HideFlag;
	DWORD	IntNum;
	DWORD	Selector;
	DWORD	FunAddress;
	CHAR	ImagePath[MAX_PATH];
	WORD	Ring;
	WORD	SegmentPresent;
	WORD	Bits;
	WORD	Type;
}IDTINFO,*PIDTINFO; 

typedef	struct InterGDTInfo
{
	USHORT selector;
    unsigned long base;
	unsigned long limit : 20;
    unsigned char g : 1;
	unsigned char d : 1;
	unsigned char avl : 1;     	
    unsigned char p : 1;
    unsigned char dpl : 2;
    unsigned char dt : 1;
    unsigned char type : 4; 
} GDTINFO,*PGDTINFO;
 
//////////////////////////////////////////////////////////////////////////
#endif