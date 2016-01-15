//////////////////////////////////////////////////////////////////////////
// Net.cpp

#include "StdAfx.h"
#include <ws2spi.h>
#include "Memory.h"
#include "Net.h"


#pragma comment(lib, "ws2_32.lib")

extern HANDLE g_hDriver;
//////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣������ֱ�ʾ��IP��ַת�������ַ�����ʾ��IP
//	���������IP����������
//	���������ָ��IP�ַ����Ļ�����
//
///////////////////////////////////////////////////////////////////////////////////
PCHAR GetIP(UINT ipaddr)
{
	static CHAR pIP[20];
	ZeroMemory(pIP,20);
	UINT nipaddr = htonl(ipaddr);
	sprintf(pIP, "%d.%d.%d.%d",
		(nipaddr >>24) &0xFF,
		(nipaddr>>16) &0xFF,
		(nipaddr>>8) &0xFF,
		(nipaddr)&0xFF);
	return pIP;
}

DWORD	GetPidIndex(PPIDINFO	pinfo,DWORD Num,DWORD	FindPid)
{
	for(DWORD	iCount=0;iCount<Num;iCount++)
	{
		if(pinfo[iCount].Pid==FindPid)
		{
			return iCount;
		}
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣���ȡ����PID��IMAGEȫ·����Ϣ
//	���������NumΪ��ȡ�Ľ��̸�����ָ��
//	���������ָ�����ȫ·����Ϣ�Ļ�����ָ��
//
///////////////////////////////////////////////////////////////////////////////////
PPIDINFO GetProcessPidNameList(PDWORD Num)
{
	DWORD		NumOfProcess=0;
	PPIDINFO	pinfo=NULL;
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	PROCESSENTRY32 processEntry = { 0 };
	processEntry.dwSize = sizeof(PROCESSENTRY32); 

	if (hProcessSnap == INVALID_HANDLE_VALUE) 
	{
		TRACE0("[GetProcessPidNameList] ��ȡ�����б�ʧ��\n");
		return NULL;
	}
	
	BOOL bRet=Process32First(hProcessSnap, &processEntry);
	NumOfProcess++;
	while(Process32Next(hProcessSnap, &processEntry)) 
	{
		NumOfProcess++;
	}
	CloseHandle(hProcessSnap);

	pinfo=(PPIDINFO)AllocHeapMem(sizeof(PIDINFO)*NumOfProcess);

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) 
	{
		FreeHeapMem(pinfo);
		return NULL;
	}

	bRet=Process32First(hProcessSnap, &processEntry);
	DWORD	iCount=0;
	pinfo[iCount].Pid=processEntry.th32ProcessID;
	MODULEENTRY32 me32 = {0}; 
	HANDLE hModuleSnap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE,
													pinfo[iCount].Pid ); 
	
	Module32First(hModuleSnap, &me32); // ��ý���ȫ·��.
	lstrcpy(pinfo[iCount].Name, me32.szExePath);
	while(Process32Next(hProcessSnap, &processEntry)) 
	{
		iCount++;
		MODULEENTRY32 me32 = {0}; 
		me32.dwSize = sizeof(MODULEENTRY32); 
		HANDLE hModuleSnap = CreateToolhelp32Snapshot
			(TH32CS_SNAPMODULE, processEntry.th32ProcessID); 

		pinfo[iCount].Pid=processEntry.th32ProcessID;			
        Module32First(hModuleSnap, &me32); // ��ý���ȫ·��.
		lstrcpy(pinfo[iCount].Name, me32.szExePath);
		CloseHandle(hModuleSnap);
	} 
	CloseHandle(hProcessSnap);
	*Num=NumOfProcess;
	return pinfo;
}

///////////////////////////////////////////////////////////////////////////////////
//
//	����ʵ�֣�ͨ����������TCPIP.SYS����IRP����ȡ�˿���Ϣ
//	�����������ȡ�Ķ˿���Ϣ����
//	�������������PNETLINKINFO���͵Ĵ�Ŷ˿���Ϣ�Ļ�����ָ��,
//  ע�ͣ�������Լ���Ҫ����FreeHeapMem�ͷ�����Ļ������ڴ�
//
///////////////////////////////////////////////////////////////////////////////////
PNETLINKINFO
KernelEnumPortInfo( PDWORD numOfTcp, PDWORD numOfUdp )
{
	DWORD			TcpNum=0;
	DWORD			UdpNum=0;
	PNETLINKINFO	pNetlink=NULL;
	PCONNINFO102	TcpInfo=NULL;
	PUDPCONNINFO	UdpInfo=NULL;
	PPIDINFO		pinfo=NULL;
	DWORD			NumOfProcess=0;

	pinfo = GetProcessPidNameList(&NumOfProcess);
	if(!pinfo)
	{
		*numOfTcp=0;
		*numOfUdp=0;
		return NULL;
	}
	//AfxMessageBox("��ȡ������Ϣ�ɹ���");

	__try
	{
		TcpInfo=(PCONNINFO102)AllocHeapMem(MAX_NETLINK*sizeof(CONNINFO102));
		if(!TcpInfo)
		{
			TRACE0("[KernelEnumPortInfo] TcpInfo�����ڴ�ʧ��\n");
			*numOfTcp=0;
			*numOfUdp=0;
			pNetlink=NULL;
			__leave;
		}

		BOOL	bRc=FALSE;
		unsigned long int bytesReturned;

		//���Ϳ���I/Oָ��
		bRc = DeviceIoControl ( g_hDriver, 
								(DWORD)IOCTL_ENUM_TCP, 
								NULL, 
								0, 
								TcpInfo,
								MAX_NETLINK*sizeof(CONNINFO102),
								&bytesReturned,
								NULL);
		if(!bRc)
		{
			*numOfTcp=0;
			*numOfUdp=0;
			pNetlink=NULL;
			__leave;
		}

		//AfxMessageBox("��ȡtcp����������Ϣ�ɹ���");

		TcpNum=bytesReturned/sizeof(CONNINFO102);
		*numOfTcp=TcpNum;

		UdpInfo=(PUDPCONNINFO)AllocHeapMem(MAX_NETLINK*sizeof(UDPCONNINFO));
		if(!UdpInfo)
		{
			TRACE0("[KernelEnumPortInfo] UdpInfo�����ڴ�ʧ��\n");
			*numOfUdp=0;
			pNetlink=NULL;
			__leave;
		}

		bRc = DeviceIoControl ( g_hDriver, 
								(DWORD)IOCTL_ENUM_UDP, 
								NULL,
								0,
								UdpInfo,
								MAX_NETLINK*sizeof(UDPCONNINFO),
								&bytesReturned,
								NULL );
		if(!bRc)
		{
			*numOfUdp=0;
			pNetlink=NULL;
			__leave;
		}

		//AfxMessageBox("��ȡudp����������Ϣ�ɹ���");

		UdpNum=bytesReturned/sizeof(UDPCONNINFO);
		*numOfUdp=UdpNum;

		pNetlink=(PNETLINKINFO)AllocHeapMem((TcpNum+UdpNum)*sizeof(NETLINKINFO));
		if(!pNetlink)
		{
			TRACE0("[KernelEnumPortInfo] UdpInfo�����ڴ�ʧ��\n");
			*numOfTcp=0;
			*numOfUdp=0;
			pNetlink=NULL;
			__leave;
		}

		DWORD	iCount=0;
		for(iCount=0; iCount<TcpNum; iCount++)
		{
			DWORD	Index = GetPidIndex(pinfo,NumOfProcess,TcpInfo[iCount].pid);

			pNetlink[iCount].Pid=TcpInfo[iCount].pid;
			pNetlink[iCount].SrcPort=HTONS(TcpInfo[iCount].src_port);

			if(TcpInfo[iCount].dst_addr==0)
			{
				pNetlink[iCount].DesPort=0;
			}
			else
			{
				pNetlink[iCount].DesPort=HTONS(TcpInfo[iCount].dst_port);
			}

			strcpy(pNetlink[iCount].Type,"TCP");
			strcpy(pNetlink[iCount].SrcIP,GetIP(TcpInfo[iCount].src_addr));
			strcpy(pNetlink[iCount].DesIP,GetIP(TcpInfo[iCount].dst_addr));
			strcpy(pNetlink[iCount].ImagePath,pinfo[Index].Name);
			strcpy(pNetlink[iCount].State,TcpState[TcpInfo[iCount].status]);
		}


		//AfxMessageBox("����tcp����������Ϣ�ɹ���");

		for( DWORD uCount=0; uCount<UdpNum; uCount++,iCount++ )
		{
			DWORD	Index = GetPidIndex(pinfo, NumOfProcess, UdpInfo[uCount].pid);

			pNetlink[iCount].Pid=UdpInfo[uCount].pid;
			pNetlink[iCount].SrcPort=HTONS(UdpInfo[uCount].src_port);
			pNetlink[iCount].DesPort=0;

			strcpy(pNetlink[iCount].Type,"UDP");
			strcpy(pNetlink[iCount].SrcIP, GetIP(UdpInfo[uCount].src_addr));
			strcpy(pNetlink[iCount].DesIP,"*");
			strcpy(pNetlink[iCount].ImagePath, pinfo[Index].Name);
			strcpy(pNetlink[iCount].State,"");
		}

		//AfxMessageBox("����udp����������Ϣ�ɹ���");
	}
	__finally
	{
		if(TcpInfo)
		{
			FreeHeapMem(TcpInfo);
		}

		if(UdpInfo)
		{
			FreeHeapMem(UdpInfo);
		}

		if(pinfo)
		{
			FreeHeapMem(pinfo);
		}
	}

	return pNetlink;
}