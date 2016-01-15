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
//	功能实现：将数字表示的IP地址转换成用字符串表示的IP
//	输入参数：IP的数字姓氏
//	输出参数：指向IP字符串的缓冲区
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
//	功能实现：获取进程PID和IMAGE全路径信息
//	输入参数：Num为获取的进程个数的指针
//	输出参数：指向进程全路径信息的缓冲区指针
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
		TRACE0("[GetProcessPidNameList] 获取进程列表失败\n");
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
	
	Module32First(hModuleSnap, &me32); // 获得进程全路径.
	lstrcpy(pinfo[iCount].Name, me32.szExePath);
	while(Process32Next(hProcessSnap, &processEntry)) 
	{
		iCount++;
		MODULEENTRY32 me32 = {0}; 
		me32.dwSize = sizeof(MODULEENTRY32); 
		HANDLE hModuleSnap = CreateToolhelp32Snapshot
			(TH32CS_SNAPMODULE, processEntry.th32ProcessID); 

		pinfo[iCount].Pid=processEntry.th32ProcessID;			
        Module32First(hModuleSnap, &me32); // 获得进程全路径.
		lstrcpy(pinfo[iCount].Name, me32.szExePath);
		CloseHandle(hModuleSnap);
	} 
	CloseHandle(hProcessSnap);
	*Num=NumOfProcess;
	return pinfo;
}

///////////////////////////////////////////////////////////////////////////////////
//
//	功能实现：通过驱动中向TCPIP.SYS发送IRP来获取端口信息
//	输入参数：获取的端口信息个数
//	输出参数：返回PNETLINKINFO类型的存放端口信息的缓冲区指针,
//  注释：用完后，自己需要调用FreeHeapMem释放申请的缓冲区内存
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
	//AfxMessageBox("获取进程信息成功！");

	__try
	{
		TcpInfo=(PCONNINFO102)AllocHeapMem(MAX_NETLINK*sizeof(CONNINFO102));
		if(!TcpInfo)
		{
			TRACE0("[KernelEnumPortInfo] TcpInfo申请内存失败\n");
			*numOfTcp=0;
			*numOfUdp=0;
			pNetlink=NULL;
			__leave;
		}

		BOOL	bRc=FALSE;
		unsigned long int bytesReturned;

		//传送控制I/O指令
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

		//AfxMessageBox("获取tcp网络连接信息成功！");

		TcpNum=bytesReturned/sizeof(CONNINFO102);
		*numOfTcp=TcpNum;

		UdpInfo=(PUDPCONNINFO)AllocHeapMem(MAX_NETLINK*sizeof(UDPCONNINFO));
		if(!UdpInfo)
		{
			TRACE0("[KernelEnumPortInfo] UdpInfo申请内存失败\n");
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

		//AfxMessageBox("获取udp网络连接信息成功！");

		UdpNum=bytesReturned/sizeof(UDPCONNINFO);
		*numOfUdp=UdpNum;

		pNetlink=(PNETLINKINFO)AllocHeapMem((TcpNum+UdpNum)*sizeof(NETLINKINFO));
		if(!pNetlink)
		{
			TRACE0("[KernelEnumPortInfo] UdpInfo申请内存失败\n");
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


		//AfxMessageBox("加入tcp网络连接信息成功！");

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

		//AfxMessageBox("加入udp网络连接信息成功！");
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