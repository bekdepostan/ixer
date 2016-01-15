//////////////////////////////////////////////////////////////////////////
// PE.cpp

#include "StdAfx.h"
#include "Memory.h"
#include "PE.h"

//--------------------------------------------PE文件操作-------------------------------------------------


///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：解析PE文件，获取PE结构中各个头的指针
//	输入参数：ibase为映像基地址；
//			  pfh,poh,psh分别为IMAGE_FILE_HEADER，IMAGE_OPTIONAL_HEADER，
//			  IMAGE_SECTION_HEADER的指针
//	输出参数：返回TRUE表示获取成功，否则失败
//
///////////////////////////////////////////////////////////////////////////////////
DWORD GetHeaders(PCHAR ibase,
				 PIMAGE_FILE_HEADER *pfh,
				 PIMAGE_OPTIONAL_HEADER *poh,
				 PIMAGE_SECTION_HEADER *psh)
				 
{
	TRACE0("获取PE文件头信息");
    PIMAGE_DOS_HEADER mzhead=(PIMAGE_DOS_HEADER)ibase;
    
    if    ((mzhead->e_magic!=IMAGE_DOS_SIGNATURE) ||        
        (ibaseDD[mzhead->e_lfanew]!=IMAGE_NT_SIGNATURE))
        return FALSE;
    
    *pfh=(PIMAGE_FILE_HEADER)&ibase[mzhead->e_lfanew];
    if (((PIMAGE_NT_HEADERS)*pfh)->Signature!=IMAGE_NT_SIGNATURE) 
        return FALSE;
    *pfh=(PIMAGE_FILE_HEADER)((PBYTE)*pfh+sizeof(IMAGE_NT_SIGNATURE));
    
    *poh=(PIMAGE_OPTIONAL_HEADER)((PBYTE)*pfh+sizeof(IMAGE_FILE_HEADER));
    if ((*poh)->Magic!=IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        return FALSE;
    
    *psh=(PIMAGE_SECTION_HEADER)((PBYTE)*poh+sizeof(IMAGE_OPTIONAL_HEADER));
    return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：将指定的RVA地址转换成相对文件的偏移
//	输入参数：Base是文件加载的基地址;
//			  RVA是要获取文件偏移的RVA地址
//	输出参数：返回DWORD类型的相对文件的偏移量
//
///////////////////////////////////////////////////////////////////////////////////
DWORD	RVAToFileOffset(PCHAR	Base,DWORD	RVAAddress)
{
	PIMAGE_FILE_HEADER		pfh	= NULL;
	PIMAGE_OPTIONAL_HEADER	poh	= NULL;
	PIMAGE_SECTION_HEADER  psh	=NULL;
	WORD	NumOfSection=0;		
	
	if(!Base)
	{
		TRACE0("[RVAToFileOffset] 基地址无效\n");
		return 0x00000000;
	}
	
	if(!GetHeaders(Base,&pfh,&poh,&psh))
	{
		TRACE0("[RVAToFileOffset] 获取PE头信息失败\n");
	}
	
	NumOfSection=pfh->NumberOfSections;
	if(0==NumOfSection)
	{
		TRACE0("[RVAToFileOffset] 节个数为零\n");
	}
	
	for(WORD	iCount=0;iCount<NumOfSection;iCount++)
	{
		DWORD	SecVA=psh[iCount].VirtualAddress;
		if((RVAAddress>SecVA)&&(RVAAddress<SecVA+psh[iCount].SizeOfRawData))
		{
			return RVAAddress-SecVA+psh[iCount].PointerToRawData;
		}
	}
	
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：获取指定文件中的指定节的指针
//	输入参数：ImagePath为要获取指定节指针的映像路径
//			  FindSetionName为要查找的节名
//			  ImageBase为返回的映像的默认加载基地址;
//			  pFile为存放内存映像的缓冲区地址;
//	输出参数：返回指向指定节数据的指针
//
///////////////////////////////////////////////////////////////////////////////////
PCHAR	
GetSettionPoint(PCHAR	ImagePath,PCHAR FindSetionName,PDWORD	ImageBase,PCHAR*	pFile)
{
	TRACE0("获取指定映像指定节指针");
	
	HANDLE	h_file=NULL;
	CHAR	Path[MAX_PATH]={0};
	DWORD	Filesize=0;//文件长度
	PCHAR	pFileBuf=NULL;
	PIMAGE_FILE_HEADER    pfh;
    PIMAGE_OPTIONAL_HEADER    poh;
    PIMAGE_SECTION_HEADER    psh;
	
	//打开文件
	h_file=CreateFile(ImagePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==h_file)
	{
		TRACE0("[GetSettionPoint] 打开映像文件失败\n");
		return NULL;
	}
	
	Filesize=GetFileSize(h_file,0);
	
	pFileBuf=(PCHAR)AllocHeapMem(Filesize);
	if(!pFileBuf)
	{
		TRACE0("[GetSettionPoint] 申请内存失败\n");
		CloseHandle(h_file);
		return NULL;
	}
	
	DWORD	Readlen;
	ReadFile(h_file,pFileBuf,Filesize,&Readlen,NULL);
	if(Readlen<=0)
	{
		TRACE0("[GetSettionPoint] 映像文件大小无效\n");
		CloseHandle(h_file);
		FreeHeapMem(pFileBuf);
		return NULL;
	}
	
	CloseHandle(h_file);
	
	GetHeaders(pFileBuf,&pfh,&poh,&psh);
	
	if(NULL!=ImageBase)
	{
		*ImageBase=poh->ImageBase;
	}
	
	PIMAGE_SECTION_HEADER    psec=psh;
	DWORD	NumOfSec=pfh->NumberOfSections;
	DWORD	iCount=0;
	while(iCount<NumOfSec)
	{
		if(!stricmp((char*)psec->Name,FindSetionName))
		{
			if(psec->PointerToRawData==0)
			{
				FreeHeapMem(pFileBuf);
				return NULL;
			}
			break;
		}
		iCount++;
		psec++;
	}
	
	PCHAR	pData=(PCHAR)(pFileBuf+psec->PointerToRawData);
	
	//保存存放win32k.sys文件数据的缓冲区指针，用于以后释放
	*pFile=pFileBuf;
	
	return pData;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：获取指定PE文件中指定函数的导出地址
//	输入参数：Base为PE文件加载的基地址
//			  FindFunName为要查找的函数名
//			  NameLength为函数名长度
//	输出参数：返回指定函数名的函数首地址
//
///////////////////////////////////////////////////////////////////////////////////
DWORD	
FindFunAddress(DWORD	Base,PCHAR	FindFunName,DWORD	NameLength)
{
	TRACE0("查找指定函数的地址");
	PCHAR	pImageBase=(PCHAR)Base;
	DWORD	ImageBase=0x0000000;
	PIMAGE_FILE_HEADER		pfh		= NULL;
	PIMAGE_OPTIONAL_HEADER	poh		= NULL;
	PIMAGE_SECTION_HEADER  psh		=NULL;
    PIMAGE_EXPORT_DIRECTORY pExpDir = NULL;
	DWORD	NumOfName=0x00000000;
	PDWORD	pNameArray=NULL;
	PWORD	pOrdNameArray=NULL;
	PDWORD	pFuncArray=NULL;
	DWORD	NumOfNativeFun=0x00000000;
	DWORD	FunAddress=0x00000000;  
	
	if(!GetHeaders(pImageBase,&pfh,&poh,&psh))
	{
		TRACE0("[FindFunAddress] 获取PE头信息失败\n");
		return 0x00000000;
	}
	
	ImageBase=poh->ImageBase;
	//获取导出表
	pExpDir=(PIMAGE_EXPORT_DIRECTORY)((DWORD)pImageBase+RVAToFileOffset(pImageBase,poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress));
	//获取导出函数名个数
	NumOfName=pExpDir->NumberOfNames;
	//获取名字地址数组
	pNameArray=(PDWORD)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pExpDir->AddressOfNames));
	//获取名字序数表
	pOrdNameArray=(PWORD)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pExpDir->AddressOfNameOrdinals));
	//函数地址表
	pFuncArray=(PDWORD)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pExpDir->AddressOfFunctions));
	
	for(DWORD	iCount=0;iCount<NumOfName;iCount++)
	{
		PCSTR	pName=(PCSTR)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pNameArray[iCount]));
		if (!stricmp(pName,FindFunName))
		{
			TRACE0("[FindFunAddress] 找到指定函数\n");
			LPCVOID pFunc=(LPCVOID)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pFuncArray[pOrdNameArray[iCount]]));
			FunAddress=(DWORD)pFunc;
			return FunAddress;
		}
	}
	
	TRACE0("[FindFunAddress] 没有找到指定函数\n");
	return 0x00000000;
}