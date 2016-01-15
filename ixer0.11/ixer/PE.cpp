//////////////////////////////////////////////////////////////////////////
// PE.cpp

#include "StdAfx.h"
#include "Memory.h"
#include "PE.h"

//--------------------------------------------PE�ļ�����-------------------------------------------------


///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣�����PE�ļ�����ȡPE�ṹ�и���ͷ��ָ��
//	���������ibaseΪӳ�����ַ��
//			  pfh,poh,psh�ֱ�ΪIMAGE_FILE_HEADER��IMAGE_OPTIONAL_HEADER��
//			  IMAGE_SECTION_HEADER��ָ��
//	�������������TRUE��ʾ��ȡ�ɹ�������ʧ��
//
///////////////////////////////////////////////////////////////////////////////////
DWORD GetHeaders(PCHAR ibase,
				 PIMAGE_FILE_HEADER *pfh,
				 PIMAGE_OPTIONAL_HEADER *poh,
				 PIMAGE_SECTION_HEADER *psh)
				 
{
	TRACE0("��ȡPE�ļ�ͷ��Ϣ");
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
//	����ʵ�֣���ָ����RVA��ַת��������ļ���ƫ��
//	���������Base���ļ����صĻ���ַ;
//			  RVA��Ҫ��ȡ�ļ�ƫ�Ƶ�RVA��ַ
//	�������������DWORD���͵�����ļ���ƫ����
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
		TRACE0("[RVAToFileOffset] ����ַ��Ч\n");
		return 0x00000000;
	}
	
	if(!GetHeaders(Base,&pfh,&poh,&psh))
	{
		TRACE0("[RVAToFileOffset] ��ȡPEͷ��Ϣʧ��\n");
	}
	
	NumOfSection=pfh->NumberOfSections;
	if(0==NumOfSection)
	{
		TRACE0("[RVAToFileOffset] �ڸ���Ϊ��\n");
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
//	����ʵ�֣���ȡָ���ļ��е�ָ���ڵ�ָ��
//	���������ImagePathΪҪ��ȡָ����ָ���ӳ��·��
//			  FindSetionNameΪҪ���ҵĽ���
//			  ImageBaseΪ���ص�ӳ���Ĭ�ϼ��ػ���ַ;
//			  pFileΪ����ڴ�ӳ��Ļ�������ַ;
//	�������������ָ��ָ�������ݵ�ָ��
//
///////////////////////////////////////////////////////////////////////////////////
PCHAR	
GetSettionPoint(PCHAR	ImagePath,PCHAR FindSetionName,PDWORD	ImageBase,PCHAR*	pFile)
{
	TRACE0("��ȡָ��ӳ��ָ����ָ��");
	
	HANDLE	h_file=NULL;
	CHAR	Path[MAX_PATH]={0};
	DWORD	Filesize=0;//�ļ�����
	PCHAR	pFileBuf=NULL;
	PIMAGE_FILE_HEADER    pfh;
    PIMAGE_OPTIONAL_HEADER    poh;
    PIMAGE_SECTION_HEADER    psh;
	
	//���ļ�
	h_file=CreateFile(ImagePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if(INVALID_HANDLE_VALUE==h_file)
	{
		TRACE0("[GetSettionPoint] ��ӳ���ļ�ʧ��\n");
		return NULL;
	}
	
	Filesize=GetFileSize(h_file,0);
	
	pFileBuf=(PCHAR)AllocHeapMem(Filesize);
	if(!pFileBuf)
	{
		TRACE0("[GetSettionPoint] �����ڴ�ʧ��\n");
		CloseHandle(h_file);
		return NULL;
	}
	
	DWORD	Readlen;
	ReadFile(h_file,pFileBuf,Filesize,&Readlen,NULL);
	if(Readlen<=0)
	{
		TRACE0("[GetSettionPoint] ӳ���ļ���С��Ч\n");
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
	
	//������win32k.sys�ļ����ݵĻ�����ָ�룬�����Ժ��ͷ�
	*pFile=pFileBuf;
	
	return pData;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣���ȡָ��PE�ļ���ָ�������ĵ�����ַ
//	���������BaseΪPE�ļ����صĻ���ַ
//			  FindFunNameΪҪ���ҵĺ�����
//			  NameLengthΪ����������
//	�������������ָ���������ĺ����׵�ַ
//
///////////////////////////////////////////////////////////////////////////////////
DWORD	
FindFunAddress(DWORD	Base,PCHAR	FindFunName,DWORD	NameLength)
{
	TRACE0("����ָ�������ĵ�ַ");
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
		TRACE0("[FindFunAddress] ��ȡPEͷ��Ϣʧ��\n");
		return 0x00000000;
	}
	
	ImageBase=poh->ImageBase;
	//��ȡ������
	pExpDir=(PIMAGE_EXPORT_DIRECTORY)((DWORD)pImageBase+RVAToFileOffset(pImageBase,poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress));
	//��ȡ��������������
	NumOfName=pExpDir->NumberOfNames;
	//��ȡ���ֵ�ַ����
	pNameArray=(PDWORD)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pExpDir->AddressOfNames));
	//��ȡ����������
	pOrdNameArray=(PWORD)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pExpDir->AddressOfNameOrdinals));
	//������ַ��
	pFuncArray=(PDWORD)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pExpDir->AddressOfFunctions));
	
	for(DWORD	iCount=0;iCount<NumOfName;iCount++)
	{
		PCSTR	pName=(PCSTR)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pNameArray[iCount]));
		if (!stricmp(pName,FindFunName))
		{
			TRACE0("[FindFunAddress] �ҵ�ָ������\n");
			LPCVOID pFunc=(LPCVOID)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pFuncArray[pOrdNameArray[iCount]]));
			FunAddress=(DWORD)pFunc;
			return FunAddress;
		}
	}
	
	TRACE0("[FindFunAddress] û���ҵ�ָ������\n");
	return 0x00000000;
}