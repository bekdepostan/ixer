//////////////////////////////////////////////////////////////////////////
// OS.cpp

#include "StdAfx.h"
#include "rootkit.h"
#include "OS.h"

///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣��жϵ�ǰ����ϵͳ�汾
//	�����������
//	�������������������غ�������������ĺ�
//
///////////////////////////////////////////////////////////////////////////////////
WORD	GetCurrentOSVersion(void)
{
	OSVERSIONINFOEX	osVersion;
	BOOL	bStatus=FALSE;

	//��ȡ����ϵͳ�汾
	osVersion.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	bStatus=GetVersionEx((OSVERSIONINFO*)&osVersion);
	if(!bStatus)
	{
		TRACE0("[GetCurrentOSVersion] ��ȡ����ϵͳ�汾ʧ��!");
		return VER_GET_ERROR;
	}

	//ȷ����NTϵͳ
	if(VER_PLATFORM_WIN32_NT!=osVersion.dwPlatformId)
	{
		TRACE0("[GetCurrentOSVersion] ��֧�ַ�NT��Windows�汾");
		return VER_UNSUPPORT;
	}

	//�ж����ĸ�����ϵͳ����
	switch (osVersion.dwMajorVersion)
	{
	case 5:
		{
			switch(osVersion.dwMinorVersion)
			{
			case 1:
				{
					switch (osVersion.wServicePackMajor)
					{
					case 0:
						{
							TRACE0("[GetCurrentOSVersion] Windows XP��ͨ�汾");
							return VER_WINXP;
						}
					case 1:
						{
							//XP sp1
							TRACE0("[GetCurrentOSVersion] Windows XP SP1");
							return VER_WXPSP1;
						}
					case 2:
						{
							//XP sp2
							TRACE0("[GetCurrentOSVersion] Windows XP SP2");
							return VER_WXPSP2;
						}
					case 3:
						{
							//XP sp3
							TRACE0("[GetCurrentOSVersion] Windows XP SP3");
							return VER_WXPSP3;
						}
					default:
						{
							TRACE0("[GetCurrentOSVersion]  Windows 5.1 SPXXXX ��֧��");
							return VER_UNSUPPORT;
						}
					}
				}
			case 2:
				{
					switch (osVersion.wServicePackMajor)
					{
					case 0:
						{
							//2003��ͨ�汾
							TRACE0("[GetCurrentOSVersion]  Windows 2003 ��ͨ�汾");
							return VER_W2K3;
						}
					case 1:
						{
							//2003sp1
							TRACE0("[GetCurrentOSVersion]  Windows 2003 SP1");
							return VER_W2K3SP1;
						}
					case 2:
						{
							//2003sp2
							TRACE0("[GetCurrentOSVersion]  Windows 2003 SP2");
							return VER_W2K3SP2;
						}
					default:
						{
							TRACE0("[GetCurrentOSVersion]  Windows 2003 5.2 SPXXXX ��֧��");
							break;
						}
					}
				}
			}
		
			break;
		}
	case 6:
		{
			switch (osVersion.dwMinorVersion)
			{
			case 0:
				{
					switch (osVersion.wServicePackMajor)
					{
					case 0:
						{
							//VISTA 1.1
							TRACE0("[GetCurrentOSVersion]  Windows Vista 1.1");
							return VER_VISTA11;
						}
					case 1:
						{
							//VISTA SP1
							TRACE0("[GetCurrentOSVersion]  Windows Vista SP1");
							return VER_VISTASP1;
						}
					default:
						{
							TRACE0("[GetCurrentOSVersion]  Windows Vista 6.0 SPXXXX ��֧��");
							return VER_UNSUPPORT;
						}
					}
					break;
				}
			default:
				{
					TRACE0("[GetCurrentOSVersion]  Windows Vista 6.XXXX ��֧��");
					return VER_UNSUPPORT;
				}
			}
			break;
		}
	default:
		{
			TRACE0("[GetCurrentOSVersion]  Windows Vista 6.XXXX ��֧��");
			return VER_UNSUPPORT;
		}
	}

	TRACE0("[GetCurrentOSVersion]  ��ȡϵͳ�汾��Ϣʧ��!");
	return VER_GET_ERROR;
}


///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣���ȡ�ں��ļ���Ϣ
//	���������KERNELINFO���͵Ļ�����ָ��
//	�����������ȡ�ɹ�����TRUE�����򷵻�FALSE
//
///////////////////////////////////////////////////////////////////////////////////
bool	
GetKernelFileName(PKERNELINFO	pKernelInfo)
{
	TRACE0("��ȡ�ں��ļ���");
	PMODULES    pModules=(PMODULES)&pModules;
	DWORD    dwNeededSize,rc;
	
	//��ȡϵͳģ����Ϣ
	rc=NtQuerySystemInformation(SystemModuleInformation,pModules,sizeof(PMODULES),&dwNeededSize);
	if (rc==STATUS_INFO_LENGTH_MISMATCH) //����ڴ治��
	{
        pModules=(PMODULES)GlobalAlloc(GPTR,dwNeededSize) ; //���·����ڴ�
        rc=NtQuerySystemInformation(SystemModuleInformation,pModules,dwNeededSize,NULL); //ϵͳ�ں��ļ��������ڵ�һ����ö��1��
	} 
	
	if (!NT_SUCCESS(rc))
	{
		TRACE0("[GetKernelFileName] ��ȡ�ں��ļ���ʧ��!\n");
		return FALSE;
	}
	
	//���ں��ļ���Ϣ�ŵ���������
	pKernelInfo->KernelBase=(DWORD)pModules->smi.ImageBase;
	pKernelInfo->Size=(DWORD)pModules->smi.ImageSize;
	strcpy(pKernelInfo->KernelName,pModules->smi.imageNameOffset+pModules->smi.imagePath);
	GlobalFree(pModules);
	return TRUE;
}