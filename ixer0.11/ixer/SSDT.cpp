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
//	功能实现：查找KiServiceTable
//	输入参数：hModule为模块加载地址，
//			  dwKSDT为KeServiceDescriptorTable RVA
//	输出参数：返回KiServiceTable的偏移
//
///////////////////////////////////////////////////////////////////////////////////
DWORD
FindKiServiceTable(HMODULE hModule,DWORD dwKSDT)
{
	TRACE0("查找KiServiceTable");
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
//	功能实现：解析内核文件，获取SSDT表中的服务函数原始地址
//	输入参数：*NumOfAdd返回原始SSDT表中的服务函数个数
//	输出参数：返回DWORD类型的地址数组指针
//	注释：返回的缓冲区最终要用FreeHeapMem释放
//
///////////////////////////////////////////////////////////////////////////////////
PDWORD
EnumSSDTNativeFunAddr(PDWORD	NumOfAdd)
{
	TRACE0("枚举SSDT 原始地址");
	KERNELINFO	ntos;

	if(NULL==NumOfAdd)
	{
		TRACE0("[EnumSSDTNativeFunAdd] 传入指针无效!\n");
		return NULL;
	}

	if(!GetKernelFileName(&ntos))
	{
		TRACE0("[EnumSSDTNativeFunAdd] 获取内核文件名失败!\n");
		return NULL;
	}

	HMODULE  hKernel;
	DWORD    dwKernelBase;
	dwKernelBase=ntos.KernelBase;
	hKernel=LoadLibraryEx(ntos.KernelName,0,DONT_RESOLVE_DLL_REFERENCES);  
	if (!hKernel)
	{
        TRACE0("[EnumSSDTNativeFunAdd] 加载内核文件失败!\n");
        return NULL;        
    }

	DWORD    dwKSDT; //KeServiceDescriptorTable	RVA
	if (!(dwKSDT=(DWORD)GetProcAddress(hKernel,"KeServiceDescriptorTable"))) //在内核文件中查找KeServiceDescriptorTable地址
	{
        TRACE0("[EnumSSDTNativeFunAdd] 内核文件中查找KeServiceDescriptorTable地址失败!\n");
		FreeLibrary(hKernel);
        return NULL;
    }

	dwKSDT-=(DWORD)hKernel;       // 获取 KeServiceDescriptorTable RVA
	DWORD    dwKiServiceTable;    //获取KiServiceTable RVA
    if (!(dwKiServiceTable=FindKiServiceTable(hKernel,dwKSDT)))   // 获取KiServiceTable地址
	{
        TRACE0("[EnumSSDTNativeFunAdd] 获取KiServiceTable地址失败!\n");
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
		TRACE0("[EnumSSDTNativeFunAdd] 获取KiServiceTable地址失败!\n");
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
//	功能实现：解析内核文件，获取SSDT表中的服务函数原始地址
//	输入参数：*NumOfAdd返回原始SSDT表中的服务函数个数
//	输出参数：返回DWORD类型的地址数组指针
//	注释：返回的缓冲区最终要用FreeHeapMem释放
//
///////////////////////////////////////////////////////////////////////////////////
PSSDT_NAME
EnumServiceFunName(PDWORD	NumOfFunName)
{
	TRACE0("枚举SSDT 服务函数名");
	CHAR	Path[MAX_PATH]={0};
	HANDLE	h_file=NULL;
	DWORD	FileSize=0;
	HANDLE	h_mapfile=NULL;//库映像句柄
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
  

	//获取系统目录
 	GetSystemDirectory(Path,MAX_PATH);
 	strcat(Path,"\\ntdll.dll");
	
	__try
	{
		//打开ntdll.dll文件
		h_file=CreateFile(Path,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if(h_file==INVALID_HANDLE_VALUE)
		{
			TRACE0("[EnumServiceFunName] 打开内核文件失败!\n");
			__leave;
		}

		//获取ntdll.dll文件长度
		FileSize=GetFileSize(h_file,0);

		h_mapfile=CreateFileMapping(h_file,NULL,PAGE_READONLY,0,0,NULL);
		if(!h_mapfile)
		{
			TRACE0("[EnumServiceFunName] 映射ntdll.dll文件失败\n");
			__leave;
		}

		pImageBase=(PCHAR)MapViewOfFile(h_mapfile,FILE_MAP_READ,0,0,0);
		if(!pImageBase)
		{
			TRACE0("[EnumServiceFunName] 打开ntdll.dll映射视图失败\n");
			__leave;
		}

		if(!GetHeaders(pImageBase,&pfh,&poh,&psh))
		{
			TRACE0("[EnumServiceFunName] 获取PE头信息失败\n");
			__leave;
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
//	功能实现：获取Shadow SSDT服务函数原始地址
//	输入参数：NumOfFun为shadow ssdt的原始地址个数;
//	输出参数：返回存放原始地址数据的缓冲区指针，需要自己调用FreeHeapMem释放释放
//
///////////////////////////////////////////////////////////////////////////////////
PDWORD	GetShadowSSDTNativeAddr(PDWORD	NumOfFun)
{
	TRACE0("获取Shadow SSDT 服务函数原始地址");

	PDWORD	pData=NULL;
	DWORD	ImageBase=0x00000000;
	PCHAR	pBuf=0x00000000;
	CHAR	Path[MAX_PATH]={0};
	
	//获取当前系统目录
	GetSystemDirectory(Path,MAX_PATH);
	strcat(Path,"\\");
	strcat(Path,"win32k.sys");

	//获取win32k.sys的.data节数据
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