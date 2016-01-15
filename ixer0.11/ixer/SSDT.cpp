//////////////////////////////////////////////////////////////////////////
// SSDT.cpp

#include "StdAfx.h"
#include "Module.h"
#include "Memory.h"
#include "SSDT.h"
#include "PE.h"
#include "OS.h"

//////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣�����KiServiceTable
//	���������hModuleΪģ����ص�ַ��
//			  dwKSDTΪKeServiceDescriptorTable RVA
//	�������������KiServiceTable��ƫ��
//
///////////////////////////////////////////////////////////////////////////////////
DWORD
FindKiServiceTable(HMODULE hModule,DWORD dwKSDT)
{
	TRACE0("����KiServiceTable");
    PIMAGE_FILE_HEADER    pfh;
    PIMAGE_OPTIONAL_HEADER    poh;
    PIMAGE_SECTION_HEADER    psh;
    PIMAGE_BASE_RELOCATION    pbr;
    PIMAGE_FIXUP_ENTRY    pfe;    
    
    DWORD    dwFixups=0,i,dwPointerRva,dwPointsToRva,dwKiServiceTable;
    BOOL    bFirstChunk;
	
    GetHeaders((char *)hModule,&pfh,&poh,&psh);
	
    if ((poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) &&
        (!((pfh->Characteristics)&IMAGE_FILE_RELOCS_STRIPPED))) {
        
        pbr=(PIMAGE_BASE_RELOCATION)RVATOVA(poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress,hModule);
		
        bFirstChunk=TRUE;
        while (bFirstChunk || pbr->VirtualAddress) {
            bFirstChunk=FALSE;
			
            pfe=(PIMAGE_FIXUP_ENTRY)((DWORD)pbr+sizeof(IMAGE_BASE_RELOCATION));
			
            for (i=0;i<(pbr->SizeOfBlock-sizeof(IMAGE_BASE_RELOCATION))>>1;i++,pfe++) {
                if (pfe->type==IMAGE_REL_BASED_HIGHLOW) {
                    dwFixups++;
                    dwPointerRva=pbr->VirtualAddress+pfe->offset;
                    dwPointsToRva=*(PDWORD)((DWORD)hModule+dwPointerRva)-(DWORD)poh->ImageBase;
                    if (dwPointsToRva==dwKSDT) {
						
                        if (*(PWORD)((DWORD)hModule+dwPointerRva-2)==0x05c7) {
                            dwKiServiceTable=*(PDWORD)((DWORD)hModule+dwPointerRva+4)-poh->ImageBase;
                            return dwKiServiceTable;
                        }
                    }
                    
                } 
            }
            *(PDWORD)&pbr+=pbr->SizeOfBlock;
        }
    }    
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣������ں��ļ�����ȡSSDT���еķ�����ԭʼ��ַ
//	���������*NumOfAdd����ԭʼSSDT���еķ���������
//	�������������DWORD���͵ĵ�ַ����ָ��
//	ע�ͣ����صĻ���������Ҫ��FreeHeapMem�ͷ�
//
///////////////////////////////////////////////////////////////////////////////////
PDWORD
EnumSSDTNativeFunAddr(PDWORD	NumOfAdd)
{
	TRACE0("ö��SSDT ԭʼ��ַ");
	KERNELINFO	ntos;

	if(NULL==NumOfAdd)
	{
		TRACE0("[EnumSSDTNativeFunAdd] ����ָ����Ч!\n");
		return NULL;
	}

	if(!GetKernelFileName(&ntos))
	{
		TRACE0("[EnumSSDTNativeFunAdd] ��ȡ�ں��ļ���ʧ��!\n");
		return NULL;
	}

	HMODULE  hKernel;
	DWORD    dwKernelBase;
	dwKernelBase=ntos.KernelBase;
	hKernel=LoadLibraryEx(ntos.KernelName,0,DONT_RESOLVE_DLL_REFERENCES);  
	if (!hKernel)
	{
        TRACE0("[EnumSSDTNativeFunAdd] �����ں��ļ�ʧ��!\n");
        return NULL;        
    }

	DWORD    dwKSDT; //KeServiceDescriptorTable	RVA
	if (!(dwKSDT=(DWORD)GetProcAddress(hKernel,"KeServiceDescriptorTable"))) //���ں��ļ��в���KeServiceDescriptorTable��ַ
	{
        TRACE0("[EnumSSDTNativeFunAdd] �ں��ļ��в���KeServiceDescriptorTable��ַʧ��!\n");
		FreeLibrary(hKernel);
        return NULL;
    }

	dwKSDT-=(DWORD)hKernel;       // ��ȡ KeServiceDescriptorTable RVA
	DWORD    dwKiServiceTable;    //��ȡKiServiceTable RVA
    if (!(dwKiServiceTable=FindKiServiceTable(hKernel,dwKSDT)))   // ��ȡKiServiceTable��ַ
	{
        TRACE0("[EnumSSDTNativeFunAdd] ��ȡKiServiceTable��ַʧ��!\n");
		FreeLibrary(hKernel);
        return NULL;
    }

	PIMAGE_FILE_HEADER    pfh;
	PIMAGE_OPTIONAL_HEADER    poh;
    PIMAGE_SECTION_HEADER    psh;
	GetHeaders((char *)hKernel,&pfh,&poh,&psh);

	PDWORD    pService;
	int dwIndex=0;
	for (pService=(PDWORD)((DWORD)hKernel+dwKiServiceTable);
	*pService-poh->ImageBase<poh->SizeOfImage;
         pService++,dwIndex++);
	
	PDWORD	pData=NULL;
	pData=(PDWORD)AllocHeapMem(dwIndex*4+4);
	if(NULL==pData)
	{
		TRACE0("[EnumSSDTNativeFunAdd] ��ȡKiServiceTable��ַʧ��!\n");
		FreeLibrary(hKernel);
        return NULL;
	}

	*NumOfAdd=dwIndex;

	dwIndex=0;
	for (pService=(PDWORD)((DWORD)hKernel+dwKiServiceTable);
	*pService-poh->ImageBase<poh->SizeOfImage;
    pService++,dwIndex++)
	{
		pData[dwIndex]=*pService-poh->ImageBase+dwKernelBase;
	}

	FreeLibrary(hKernel);
	return pData;
}

//////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣������ں��ļ�����ȡSSDT���еķ�����ԭʼ��ַ
//	���������*NumOfAdd����ԭʼSSDT���еķ���������
//	�������������DWORD���͵ĵ�ַ����ָ��
//	ע�ͣ����صĻ���������Ҫ��FreeHeapMem�ͷ�
//
///////////////////////////////////////////////////////////////////////////////////
PSSDT_NAME
EnumServiceFunName(PDWORD	NumOfFunName)
{
	TRACE0("ö��SSDT ��������");
	CHAR	Path[MAX_PATH]={0};
	HANDLE	h_file=NULL;
	DWORD	FileSize=0;
	HANDLE	h_mapfile=NULL;//��ӳ����
	PCHAR	pImageBase=NULL;
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
	PSSDT_NAME	pSSDTName=NULL;
  

	//��ȡϵͳĿ¼
 	GetSystemDirectory(Path,MAX_PATH);
 	strcat(Path,"\\ntdll.dll");
	
	__try
	{
		//��ntdll.dll�ļ�
		h_file=CreateFile(Path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(h_file==INVALID_HANDLE_VALUE)
		{
			TRACE0("[EnumServiceFunName] ���ں��ļ�ʧ��!\n");
			__leave;
		}

		//��ȡntdll.dll�ļ�����
		FileSize=GetFileSize(h_file,0);

		h_mapfile=CreateFileMapping(h_file,NULL,PAGE_READONLY,0,0,NULL);
		if(!h_mapfile)
		{
			TRACE0("[EnumServiceFunName] ӳ��ntdll.dll�ļ�ʧ��\n");
			__leave;
		}

		pImageBase=(PCHAR)MapViewOfFile(h_mapfile,FILE_MAP_READ,0,0,0);
		if(!pImageBase)
		{
			TRACE0("[EnumServiceFunName] ��ntdll.dllӳ����ͼʧ��\n");
			__leave;
		}

		if(!GetHeaders(pImageBase,&pfh,&poh,&psh))
		{
			TRACE0("[EnumServiceFunName] ��ȡPEͷ��Ϣʧ��\n");
			__leave;
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
			if ('N' == pName[0] && 't' == pName[1])
			{
				LPCVOID pFunc=(LPCVOID)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pFuncArray[pOrdNameArray[iCount]]));
				SSDTENTRY entry;
				CopyMemory( &entry, pFunc, sizeof(SSDTENTRY) );
				if ( MOV_OPCODE== entry.byMov )
				{
					NumOfNativeFun++;
				}
			}
		}

		if(NumOfFunName)
		{
			*NumOfFunName=NumOfNativeFun;
		}

		pSSDTName=(PSSDT_NAME)AllocHeapMem(NumOfNativeFun*sizeof(SSDT_NAME));
		if(!pSSDTName)
		{
			__leave;
		}

		for(iCount=0;iCount<NumOfName;iCount++)
		{
			PCSTR	pName=(PCSTR)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pNameArray[iCount]));
			if ('N'== pName[0]&&'t'==pName[1])
			{
				LPCVOID pFunc=(LPCVOID)((DWORD)pImageBase+RVAToFileOffset(pImageBase,pFuncArray[pOrdNameArray[iCount]]));
				SSDTENTRY entry;
				CopyMemory( &entry, pFunc, sizeof( SSDTENTRY ) );
				if ( MOV_OPCODE== entry.byMov )
				{
					strncpy(pSSDTName[entry.dwIndex].funName,pName,strlen(pName));
					pSSDTName[entry.dwIndex].index=entry.dwIndex;
				}
			}
		}
	}
	__finally
	{
		if(pImageBase)
		{
			UnmapViewOfFile(pImageBase);
			pImageBase=NULL;
		}
		if(h_mapfile)
		{
			CloseHandle(h_mapfile);
			h_mapfile=NULL;
		}
		if(h_file)
		{
			CloseHandle(h_file);
			h_file=NULL;
		}
	}

	return pSSDTName;
}

///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣���ȡShadow SSDT������ԭʼ��ַ
//	���������NumOfFunΪshadow ssdt��ԭʼ��ַ����;
//	������������ش��ԭʼ��ַ���ݵĻ�����ָ�룬��Ҫ�Լ�����FreeHeapMem�ͷ��ͷ�
//
///////////////////////////////////////////////////////////////////////////////////
PDWORD	GetShadowSSDTNativeAddr(PDWORD	NumOfFun)
{
	TRACE0("��ȡShadow SSDT ������ԭʼ��ַ");

	PDWORD	pData=NULL;
	DWORD	ImageBase=0x00000000;
	PCHAR	pBuf=0x00000000;
	CHAR	Path[MAX_PATH]={0};
	
	//��ȡ��ǰϵͳĿ¼
	GetSystemDirectory(Path,MAX_PATH);
	strcat(Path,"\\");
	strcat(Path,"win32k.sys");

	//��ȡwin32k.sys��.data������
	pData=(PDWORD)GetSettionPoint(Path,".data",&ImageBase,&pBuf);
	if(!pData)
	{
		return NULL;
	}

	PDWORD	pNativeAdd=NULL;

	switch (GetCurrentOSVersion())
	{
	case VER_WXPSP2:
	case VER_WXPSP3:
	case VER_W2K3SP2:
		{
			DWORD	NumOfAdd=0x00000000;
			DWORD	iCount=0;

			for(iCount=0;pData[iCount]!=0x00000101;iCount++);
			if(NumOfFun)
			{
				*NumOfFun=iCount;
			}

			NumOfAdd=iCount;

			pNativeAdd=(PDWORD)AllocHeapMem(NumOfAdd*sizeof(DWORD));
			if(pNativeAdd)
			{
				for(iCount=0;iCount<NumOfAdd;iCount++)
				{
					pNativeAdd[iCount]=pData[iCount];
				}
			}
			break;
		}
	case VER_WXPSP1:
		{
			DWORD	NumOfAdd=0x00000000;
			DWORD	iCount=0;

			pData=(PDWORD)((DWORD)pData+0x6540);
			
			for(iCount=0;pData[iCount]!=0x00000101;iCount++);
			if(NumOfFun)
			{
				*NumOfFun=iCount;
			}
			
			NumOfAdd=iCount;
			
			pNativeAdd=(PDWORD)AllocHeapMem(NumOfAdd*sizeof(DWORD));
			if(pNativeAdd)
			{
				for(iCount=0;iCount<NumOfAdd;iCount++)
				{
					pNativeAdd[iCount]=pData[iCount];
				}
			}
			break;
		}
	case VER_W2K3:
		{
			DWORD	NumOfAdd=0x00000000;
			DWORD	iCount=0;
			
			pData=(PDWORD)((DWORD)pData+0x9524);
			
			for(iCount=0;pData[iCount]!=0x00000101;iCount++);
			if(NumOfFun)
			{
				*NumOfFun=iCount;
			}

			NumOfAdd=iCount;
			
			pNativeAdd=(PDWORD)AllocHeapMem(NumOfAdd*sizeof(DWORD));
			if(pNativeAdd)
			{
				for(iCount=0;iCount<NumOfAdd;iCount++)
				{
					pNativeAdd[iCount]=pData[iCount];
				}
			}
			break;
		}
	case VER_VISTA11:
	case VER_VISTASP1:
	case VER_VISTAULT:
		{
			DWORD	W32kBase=FindModuleLoadBase("win32k.sys",10);
			if(W32kBase)
			{
				DWORD	NumOfAdd=0x00000000;
				DWORD	iCount=0;
				
				for(iCount=0;pData[iCount]!=0x00000101;iCount++);
				if(NumOfFun)
				{
					*NumOfFun=iCount;
				}
				
				NumOfAdd=iCount;
				
				pNativeAdd=(PDWORD)AllocHeapMem(NumOfAdd*sizeof(DWORD));
				if(pNativeAdd)
				{
					for(iCount=0;iCount<NumOfAdd;iCount++)
					{
						pNativeAdd[iCount]=pData[iCount]-ImageBase+W32kBase;
					}
				}
			}

			break;
		}
	default:
		{
			FreeHeapMem(pBuf);
		}	
	}
	
	FreeHeapMem(pBuf);
	return pNativeAdd;
}