//////////////////////////////////////////////////////////////////////////
// Net.h

#ifndef	___NET___
#define ___NET___


typedef	struct InterPidInfo
{
	DWORD	Pid;
	CHAR	Name[MAX_PATH];
}	PIDINFO,*PPIDINFO;

//TCP相关结构体
typedef struct _MIB_TCPEXROW
{
	DWORD dwState;          //连接状态
	DWORD dwLocalAddr;      //当前计算机IP地址
	DWORD dwLocalPort;      //当前计算机端口
	DWORD dwRemoteAddr;     //远程计算机IP地址
	DWORD dwRemotePort;     //远程计算机端口
	DWORD dwProcessPID;     //当前进程PID
}MIB_TCPEXROW,*PMIB_TCPEXROW;

typedef struct _MIB_TCPEXPTABLE
{
	DWORD dwNumEntries;
	MIB_TCPEXROW table[100]; 
}MIB_TCPEXTABLE,*PMIB_TCPEXTABLE;


//TCP相关结构体
typedef struct _VISTAMIB_TCPEXROW
{
	DWORD dwState;          //连接状态
	DWORD dwLocalAddr;      //当前计算机IP地址
	DWORD dwLocalPort;      //当前计算机端口
	DWORD dwRemoteAddr;     //远程计算机IP地址
	DWORD dwRemotePort;     //远程计算机端口
	DWORD dwProcessPID;     //当前进程PID
	DWORD Un;
}VISTAMIB_TCPEXROW,*PVISTAMIB_TCPEXROW;

typedef struct _VISTAMIB_TCPEXPTABLE
{
	DWORD dwNumEntries;
	VISTAMIB_TCPEXROW table[100]; 
}VISTAMIB_TCPEXTABLE,*PVISTAMIB_TCPEXTABLE;

//UDP相关结构体
typedef struct _MIB_UDPEXROW
{
	DWORD dwLocalAddr;     //本地计算机IP地址
    DWORD dwLocalPort;     //本地计算机端口
    DWORD dwProcessPId;    //当前进程PID
}MIB_UDPEXROW,*PMIB_UDPEXROW;

typedef struct _MIB_UDPEXTABLE
{
	DWORD dwNumEntries;
	MIB_UDPEXROW table[100]; 
}MIB_UDPEXTABLE,*PMIB_UDPEXTABLE;

//所用的AllocateAndGetTcpExTableFromStack 原形
typedef DWORD(WINAPI *_AllocateAndGetTcpExtableFromStack)
(
 PMIB_TCPEXTABLE *pTcpTable,
 BOOL bOrder,                
 HANDLE heap,
 DWORD zero,
 DWORD flags
 );

typedef DWORD (WINAPI *_AllocateAndGetUdpExtableFromStack)
(
 PMIB_UDPEXTABLE *pUdpTable, // 连接表缓冲区.
 BOOL bOrder,                
 HANDLE heap,
 DWORD zero,
 DWORD flags
);

typedef	DWORD	(WINAPI *_InternalGetTcpTable2)
(
	PVISTAMIB_TCPEXTABLE *pTcpTable,
	HANDLE heap,
	DWORD flags
);

typedef	DWORD	(WINAPI *_InternalGetUdpTableWithOwnerPid)
(
	PMIB_UDPEXTABLE *pUdpTable,
	HANDLE heap,
	DWORD flags
);

//TCP 端口状态.
static char TcpState[][32] = 
{
		TEXT("???"),
		TEXT("CLOSED"),
		TEXT("LISTENING"),
		TEXT("SYN_SENT"),
		TEXT("SYN_RCVD"),
		TEXT("ESTABLISHED"),
		TEXT("FIN_WAIT1"),
		TEXT("FIN_WAIT2"),
		TEXT("CLOSE_WAIT"),
		TEXT("CLOSING"),
		TEXT("LAST_ACK"),
		TEXT("TIME_WAIT"),
		TEXT("DELETE_TCB")
};

PCHAR 
GetIP(
	UINT ipaddr
	 );

DWORD	
GetPidIndex(
	PPIDINFO	pinfo,
	DWORD Num,
	DWORD	FindPid
	);

PPIDINFO 
GetProcessPidNameList(
	PDWORD Num
	);

PNETLINKINFO
EnumPortInfo( PDWORD numOfTcp, PDWORD numOfUdp );

PNETLINKINFO
VistaEnumPortInfo(
	PDWORD	NumOfLink
	);

//以下为通过驱动枚举的端口需要的数据结构
#define MAX_NETLINK	512
#define	HTONS(a)	(((0xff&a)<<8)+((0xff00&a)>>8))
typedef struct _CONNINFO102
{
	ULONG	status;
	ULONG	src_addr;
	USHORT	src_port;
	USHORT	unk1;
	ULONG	dst_addr;
	USHORT	dst_port;
	USHORT	unk2;
	ULONG	pid;
} CONNINFO102,*PCONNINFO102;

typedef	struct _UDPCONNINFO
{
	ULONG	src_addr;
	USHORT	src_port;
	ULONG	pid;
}	UDPCONNINFO,*PUDPCONNINFO;

PNETLINKINFO
KernelEnumPortInfo( PDWORD numOfTcp, PDWORD numOfUdp );



//////////////////////////////////////////////////////////////////////////
#endif