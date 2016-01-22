//////////////////////////////////////////////////////////////////////////
// Net.h

#ifndef  __NET_H__
#define	 __NET_H__


#define IOCTL_TCP_QUERY_INFORMATION_EX 0x00120003
#define	HTONS(a)	(((0xff&a)<<8)+((0xff00&a)>>8))

#define TCPPORT	0
#define UDPPORT	1

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

NTSTATUS	
GetObjectByName(
				OUT HANDLE *FileHandle,
				OUT PFILE_OBJECT	*FileObject,
				IN WCHAR	*DeviceName
				);

PVOID		
EnumPortInformation(
					OUT PULONG	OutLength,
					IN	USHORT	PortType
					);

///////////////////////////////////////
#endif
