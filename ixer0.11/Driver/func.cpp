//////////////////////////////////////////////////////////////////////////
// func.c

#include "precomp.h"
#include "WinOSVersion.h"
#include "proc.h"
#include "..\Common\Surface.h"
#include "func.h"
#include "ix.h"



ULONG GetFunctionAddr( IN PCWSTR FunctionName)
{
    UNICODE_STRING UniCodeFunctionName;
    RtlInitUnicodeString( &UniCodeFunctionName, FunctionName );
    return (ULONG)MmGetSystemRoutineAddress( &UniCodeFunctionName );   
}


/*========================================================================
*
* 函数名:	GetPspTerminateThreadByPointer
*
* 参数:		无
*
* 功能描述:	搜索获取PspTerminateThreadByPointer函数地址
*
* 返回值:	BOOLEAN
*
=========================================================================*/
//////////////////////////////////////////////////////////////////////////
// XP: nt!PsTerminateSystemThread -> GetPspTerminateThreadByPointer
// 2k: NtTerminateThread -> GetPspTerminateThreadByPointer
//////////////////////////////////////////////////////////////////////////
BOOLEAN GetPspTerminateThreadByPointer()
{
    char	*PsTerminateSystemThreadAddr;
    int		iLen;
    DWORD	dwAddr;
	
	__try
	{
		if (g_WindowsVersion==WINDOWS_VERSION_2K)
		{
			return FALSE;
		}
		else
		{
			//获取内核导出函数PsTerminateSystemThread地址
			PsTerminateSystemThreadAddr = (PUCHAR)PsTerminateSystemThread;
			if( !PsTerminateSystemThreadAddr )
				return FALSE;
			
			//开始搜索PspTerminateThreadByPointer函数地址
			/*  xp sp2
			lkd> u PsTerminateSystemThread l 20
			nt!PsTerminateSystemThread:
			805d2670 8bff            mov     edi,edi
			805d2672 55              push    ebp
			805d2673 8bec            mov     ebp,esp
			805d2675 64a124010000    mov     eax,dword ptr fs:[00000124h]
			805d267b f6804802000010  test    byte ptr [eax+248h],10h
			805d2682 7507            jne     nt!PsTerminateSystemThread+0x1b (805d268b)
			805d2684 b80d0000c0      mov     eax,0C000000Dh
			805d2689 eb09            jmp     nt!PsTerminateSystemThread+0x24 (805d2694)
			805d268b ff7508          push    dword ptr [ebp+8]
			805d268e 50              push    eax
			805d268f e828fcffff      call    nt!PspTerminateThreadByPointer (805d22bc)
			*/

			/* 2k3
			kd> u PsTerminateSystemThread l 30
			nt!PsTerminateSystemThread:
			805f3c50 64a124010000    mov     eax,dword ptr fs:[00000124h]
			805f3c56 f6805002000010  test    byte ptr [eax+250h],10h
			805f3c5d 0f848f310200    je      nt!PsTerminateSystemThread+0xf (80616df2)
			805f3c63 6a01            push    1
			805f3c65 ff742408        push    dword ptr [esp+8]
			805f3c69 50              push    eax
			805f3c6a e821b4f8ff      call    nt!PspTerminateThreadByPointer (8057f090)
			*/
//			DbgPrint("PsTerminateSystemThread= 0x%x\n",PsTerminateSystemThreadAddr);
			for (iLen=0;iLen<0x100;iLen++)
			{
				//xp sp2
				if ( *PsTerminateSystemThreadAddr == (char)0xff
					 && *(PsTerminateSystemThreadAddr+1) == (char)0x75
					 && *(PsTerminateSystemThreadAddr+2) == (char)0x08 )
				{ 
					PsTerminateSystemThreadAddr += 5;
					dwAddr = *(DWORD *)PsTerminateSystemThreadAddr + (DWORD)PsTerminateSystemThreadAddr + 4;

					MyPspTerminateThreadByPointer = (PVOID)dwAddr;
					DbgPrint("$XTR：g_PspTerminateThreadByPointer= 0x%x\n", dwAddr);
					g_PspTerminateThreadByPointer = dwAddr;
					return TRUE; 
				}
				//2k3
				if (*PsTerminateSystemThreadAddr == (char)0xff
					&& *(PsTerminateSystemThreadAddr+1) == (char)0x74
					&& *(PsTerminateSystemThreadAddr+2) == (char)0x24
					&& *(PsTerminateSystemThreadAddr+3) == (char)0x08
					)
				{ 
					PsTerminateSystemThreadAddr += 6;
					dwAddr = *(DWORD *)PsTerminateSystemThreadAddr + (DWORD)PsTerminateSystemThreadAddr +4;
					
					MyPspTerminateThreadByPointer = (PVOID)dwAddr;
					DbgPrint("$XTR：g_PspTerminateThreadByPointer= 0x%x\n", dwAddr);
					g_PspTerminateThreadByPointer = dwAddr;
					return TRUE; 
				}
				PsTerminateSystemThreadAddr++;
			}
		}
	}
	__finally
	{ 
	}
    return FALSE;
}


//////////////////////////////////////////////////////////////////////////
// GetPspExitThread函数
// XP: g_PspTerminateThreadByPointer -> GetPspExitThread 
//////////////////////////////////////////////////////////////////////////
/*
lkd> u g_PspTerminateThreadByPointer l 20
nt!g_PspTerminateThreadByPointer:
805d22bc 8bff            mov     edi,edi
805d22be 55              push    ebp
805d22bf 8bec            mov     ebp,esp
805d22c1 83ec0c          sub     esp,0Ch
805d22c4 834df8ff        or      dword ptr [ebp-8],0FFFFFFFFh
805d22c8 56              push    esi
805d22c9 57              push    edi
805d22ca 8b7d08          mov     edi,dword ptr [ebp+8]
805d22cd 8db748020000    lea     esi,[edi+248h]
805d22d3 f60640          test    byte ptr [esi],40h
805d22d6 c745f4c0bdf0ff  mov     dword ptr [ebp-0Ch],0FFF0BDC0h
805d22dd 7417            je      nt!g_PspTerminateThreadByPointer+0x3a (805d22f6)
805d22df 8b8720020000    mov     eax,dword ptr [edi+220h]
805d22e5 0574010000      add     eax,174h
805d22ea 50              push    eax
805d22eb 57              push    edi
805d22ec 688c225d80      push    offset nt!PspExitNormalApc+0x54 (805d228c)
805d22f1 e8f2f1ffff      call    nt!PspCatchCriticalBreak (805d14e8)
805d22f6 64a124010000    mov     eax,dword ptr fs:[00000124h]
805d22fc 3bf8            cmp     edi,eax
805d22fe 750e            jne     nt!g_PspTerminateThreadByPointer+0x52 (805d230e)
805d2300 33c0            xor     eax,eax
805d2302 40              inc     eax
805d2303 f00906          lock or dword ptr [esi],eax
805d2306 ff750c          push    dword ptr [ebp+0Ch]
805d2309 e81cf8ffff      call    nt!PspExitThread (805d1b2a) <--------
*/
/*
nt!PspTerminateThreadByPointer+0x41:
8057f113 ff750c          push    dword ptr [ebp+0Ch]
8057f116 e87cfbffff      call    nt!PspExitThread (8057ec97)
*/
BOOL GetPspExitThread()
{ 
	int iLen;
	DWORD dwAddr;  
	char *PspTerminateThreadByPointerAddr;

	//首先搜索g_PspTerminateThreadByPointer
	if(!GetPspTerminateThreadByPointer())
		return FALSE;

	//如果g_PspTerminateThreadByPointer地址为空 退出
	if (g_PspTerminateThreadByPointer==0 )
		return FALSE;

	__try{
		//区分操作系统
		if (g_WindowsVersion==WINDOWS_VERSION_2K )
		{ 
			return FALSE;
		}
		else
		{ 
			//初始化
			PspTerminateThreadByPointerAddr= (char *)g_PspTerminateThreadByPointer;
			__asm
			{
				__emit 0x90;
				__emit 0x90;
			}
			for (iLen=0;iLen<0x100;iLen++)
			{
				if (*PspTerminateThreadByPointerAddr == (char)0xff
					&& *(PspTerminateThreadByPointerAddr+1) == (char)0x75
					&& *(PspTerminateThreadByPointerAddr+2) == (char)0x0C
					)
				{
					__asm
					{
						__emit 0x90;
						__emit 0x90;
					}
					PspTerminateThreadByPointerAddr += 4;
					dwAddr = *(DWORD *)PspTerminateThreadByPointerAddr + (DWORD)PspTerminateThreadByPointerAddr +4;

					DbgPrint("PspExitThread= 0x%x\n",dwAddr);
					g_PspExitThread = dwAddr;
					return TRUE; 
				}
				PspTerminateThreadByPointerAddr++;
			}
		}
	}
	__finally
	{
	}
	return FALSE;
}

/*
kd> u KeInsertQueueApc l 20
nt!KeInsertQueueApc:
804fb704 8bff            mov     edi,edi
804fb706 55              push    ebp
804fb707 8bec            mov     ebp,esp
804fb709 83ec0c          sub     esp,0Ch
804fb70c 53              push    ebx
804fb70d 56              push    esi
804fb70e 57              push    edi
804fb70f 8b7d08          mov     edi,dword ptr [ebp+8]
804fb712 8b7708          mov     esi,dword ptr [edi+8]
804fb715 8d8ee8000000    lea     ecx,[esi+0E8h]
804fb71b 8d55f4          lea     edx,[ebp-0Ch]
804fb71e ff1590864d80    call    dword ptr [nt!_imp_KeAcquireInStackQueuedSpinLockRaiseToSynch (804d8690)]
804fb724 32db            xor     bl,bl
804fb726 389e66010000    cmp     byte ptr [esi+166h],bl
804fb72c 7418            je      nt!KeInsertQueueApc+0x42 (804fb746)
804fb72e 8b450c          mov     eax,dword ptr [ebp+0Ch]
804fb731 8b5514          mov     edx,dword ptr [ebp+14h]
804fb734 894724          mov     dword ptr [edi+24h],eax
804fb737 8b4510          mov     eax,dword ptr [ebp+10h]
804fb73a 8bcf            mov     ecx,edi
804fb73c 894728          mov     dword ptr [edi+28h],eax
804fb73f e8e0290000      call    nt!KiInsertQueueApc (804fe124)	***********************
804fb744 8ad8            mov     bl,al
  */ 
//根据特征值，从KeInsertQueueApc搜索中搜索KiInsertQueueApc
ULONG GetKiInsertQueueApc()
{
	ULONG sp_code1=0x28,sp_code2=0xe8,sp_code3=0xd88a;  //特征码,sp_code3 windbg显示错误，应该为d88a
	ULONG address=0;
	PUCHAR addr;
	PUCHAR p;
	
	//首先得到 KeInsertQueueApc 地址
	addr=(PUCHAR)GetFunctionAddr(L"KeInsertQueueApc");
	if(addr==NULL)
	{
		DbgPrint("[GetKiInsertQueueApc] GetFunctionAddr error!\n");
		return 0;
	}
	//开始搜索 	KiInsertQueueApc
	for(p=addr;p<addr+PAGE_SIZE;p++)
	{
		if((*(p-1)==sp_code1)
			&&(*p==sp_code2)
			&&(*(PUSHORT)(p+5)==sp_code3))	
		{
			address=*(PULONG)(p+1)+(ULONG)(p+5);
			break;			
		}		
	} 

	DbgPrint("[KeInsertQueueApc] addr %x\n",(ULONG)addr);
    DbgPrint("[KiInsertQueueApc] address %x\n",address);

    return address;
}




/*
kd> u KeAttachProcess l 20
nt!KeAttachProcess:
804f853c 8bff            mov     edi,edi
804f853e 55              push    ebp
804f853f 8bec            mov     ebp,esp
804f8541 56              push    esi
804f8542 57              push    edi
804f8543 64a124010000    mov     eax,dword ptr fs:[00000124h]
804f8549 8b7d08          mov     edi,dword ptr [ebp+8]
804f854c 8bf0            mov     esi,eax
804f854e 397e44          cmp     dword ptr [esi+44h],edi
804f8551 742d            je      nt!KeAttachProcess+0x44 (804f8580)
804f8553 80be6501000000  cmp     byte ptr [esi+165h],0
804f855a 752a            jne     nt!KeAttachProcess+0x4a (804f8586)
804f855c 64a194090000    mov     eax,dword ptr fs:[00000994h]
804f8562 85c0            test    eax,eax
804f8564 7520            jne     nt!KeAttachProcess+0x4a (804f8586)
804f8566 ff1514874d80    call    dword ptr [nt!_imp__KeRaiseIrqlToDpcLevel (804d8714)]
804f856c 884508          mov     byte ptr [ebp+8],al
804f856f 8d864c010000    lea     eax,[esi+14Ch]
804f8575 50              push    eax
804f8576 ff7508          push    dword ptr [ebp+8]
804f8579 57              push    edi
804f857a 56              push    esi
804f857b e8b8feffff      call    nt!KiAttachProcess (804f8438)*************************************
804f8580 5f              pop     edi
804f8581 5e              pop     esi
804f8582 5d              pop     ebp
804f8583 c20400          ret     4
*/
//根据特征值，从KeAttachProcess搜索中搜索KiAttachProcess
ULONG GetKiAttachProcess()
{
	ULONG sp_code1=0x56,sp_code2=0xe8,sp_code3=0x5e5f;  //特征码,sp_code3 windbg显示错误，应该为d88a
	ULONG address=0;
	PUCHAR addr;
	PUCHAR p;
	
	//首先得到 KeInsertQueueApc 地址
	addr=(PUCHAR)GetFunctionAddr(L"KeAttachProcess");
	if(addr==NULL)
	{
		DbgPrint("[GetKiAttachProcess] GetFunctionAddr error!\n");
		return 0;
	}
	//开始搜索 	KiAttachProcess
	for(p=addr;p<addr+PAGE_SIZE;p++)
	{
		if((*(p-1)==sp_code1)
			&&(*p==sp_code2)
			&&(*(PUSHORT)(p+5)==sp_code3))	
		{
			address=*(PULONG)(p+1)+(ULONG)(p+5);
			break;			
		}		
	} 
	
	DbgPrint("[GetKiAttachProcess] KeAttachProcess = %x\n",(ULONG)addr);	
    DbgPrint("[GetKiAttachProcess] KiAttachProcess = %x\n",address);

    return address;	
}

NTSTATUS CharStringToWideChar(BOOLEAN boAnsiOrWide, char *srcAnsiStr, PWCHAR desWideChar)
{
	NTSTATUS status=STATUS_SUCCESS;
	ANSI_STRING ansi;
	UNICODE_STRING unico = {0};

	unico.Buffer=NULL;
	unico.Length=0;
	unico.MaximumLength=0;

	if (boAnsiOrWide)
	{
		//不为空则
		if (srcAnsiStr != '\0')
		{
			RtlInitAnsiString(&ansi, srcAnsiStr);
			//转char
			status=RtlAnsiStringToUnicodeString(&unico, &ansi, TRUE);
			wcscpy(desWideChar, unico.Buffer);
		}
		else
		{
			status=STATUS_UNSUCCESSFUL;
		}
	}
	else
	{
		//不为空
		if (desWideChar != '\0')
		{
			RtlInitUnicodeString(&unico, desWideChar);
			//转wchar
			status=RtlUnicodeStringToAnsiString(&ansi, &unico, TRUE);
			strcpy(srcAnsiStr, ansi.Buffer);
		}
		else
		{
			status=STATUS_UNSUCCESSFUL;
		}
	}

	return status;
}