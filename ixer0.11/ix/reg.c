/*===================================================================
 * Filename reg.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: ע���ģ��
 *
 * Date:   2013-5-19 Original from Chinghoi
 *
 * Version: 1.0
 ==================================================================*/

#include "precomp.h"
#include "..\Common\Surface.h"
#include "reg.h"


/*========================================================================
*
* ������:	QueryKeyItemInfo
*
* ����:		LPCWSTR		[IN]	- KeyFullPath
*			PULONG		[OUT]	- pNumber		ʵ����һ������
*								  pNumber[0]	subKey(����) �ĸ���
*								  pNumber[1]	keyValue �ĸ���
*								  pNumber[2]	Value Name + Data���ܻ�����
*			ULONG		[IN]	- Size		- �������Ĵ�С
*
* ��������:	��ȡע������Ϣ
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS
QueryKeyItemInfo( LPCWSTR KeyFullPath, PULONG  pNumber, ULONG Size )
{
	NTSTATUS					Status=STATUS_SUCCESS;
	HANDLE						KeyHandle;
	UNICODE_STRING				uniKey;
	OBJECT_ATTRIBUTES			ObjectAttributes;
	ULONG						cb;
	ULONG						index;
	PKEY_FULL_INFORMATION		pfi;
	PKEY_VALUE_FULL_INFORMATION	pvi;
 

	#ifdef _CHECKED
		DbgPrint( "$ixer:\tQueryKeyNumber( %ws, 0x%08x, %d )", KeyFullPath, pNumber, Size );
	#endif

	if ( Size < 3 * sizeof(ULONG) )
	{
		Status = STATUS_INVALID_PARAMETER;
		return Status;
	}

	RtlInitUnicodeString( &uniKey, KeyFullPath );

	InitializeObjectAttributes(	&ObjectAttributes,
								&uniKey,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL );

	/* �򿪼���ö��Ȩ�� */
	Status = ZwOpenKey( &KeyHandle, KEY_ENUMERATE_SUB_KEYS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* ����2�Σ���һ�λ�ȡ��������С */
	ZwQueryKey( KeyHandle, KeyFullInformation, NULL, 0, &cb );
	if ( cb != 0 )
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tResultLength:\t0x%08x", cb );
		#endif

		pfi = (PKEY_FULL_INFORMATION)ExAllocatePool( PagedPool, cb );
		if ( pfi == NULL )
		{
			Status = STATUS_INSUFFICIENT_RESOURCES;
			ZwClose( KeyHandle );

			#ifdef _CHECKED
				DbgPrint( "$ixer:\tExAllocatePool" );
			#endif

			return Status;
		}

		/* �ڶ��ε��ã���ȡ������Ϣ */
		Status = ZwQueryKey( KeyHandle, KeyFullInformation, pfi, cb, &cb );
		if (!NT_SUCCESS(Status))
		{
			#ifdef _CHECKED
				DbgPrint( "$ixer:\tZwQueryKey:\t0x%08x", Status );
			#endif

			ExFreePool( pfi );
			ZwClose( KeyHandle );

			return Status;
		}

		/* ��� */
		pNumber[0] = pfi->SubKeys;	//subKey��
		pNumber[1] = pfi->Values;	//keyValue��
		pNumber[2] = 0;

		for ( index = 0; index < pfi->Values; index++ )
		{
			/* ö�ټ�ֵ�ߴ� */
			ZwEnumerateValueKey( KeyHandle, index, KeyValueFullInformation, NULL, 0, &cb );
			if ( cb != 0 )
			{
				pvi = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool( PagedPool, cb );
				if ( pvi == NULL )
				{
					#ifdef _CHECKED
						DbgPrint( "$ixer:\tExAllocatePool()" );
					#endif

					Status = STATUS_INSUFFICIENT_RESOURCES;
					ZwClose( KeyHandle );

					return Status;
				}

				/* ö�ټ�ֵ��Ϣ */
				Status = ZwEnumerateValueKey( KeyHandle, index, KeyValueFullInformation, pvi, cb, &cb );
				if (!NT_SUCCESS(Status))
				{
					#ifdef _CHECKED
						DbgPrint( "$ixer:\tZwEnumerateKey:\t0x%08x", Status );
					#endif

					ExFreePool( pvi );
					ZwClose( KeyHandle );

					return Status;
				}
				if ( pvi->DataLength > 0 )
				{
					pNumber[2] = pNumber[2] + pvi->DataLength;
					pNumber[2] = pNumber[2] + pvi->NameLength - 1;
				}

				ExFreePool( pvi );
			}
		}

		#ifdef _CHECKED
			DbgPrint( "$ixer:\tSubKeys:\t%d", pfi->SubKeys );
		#endif

		ExFreePool( pfi );
	} // end if

	ZwClose( KeyHandle );

	#ifdef _CHECKED
		DbgPrint( "$ixer:\tQueryKeyNumber() end:\t0x%08x", Status );
	#endif

	return Status;
}

/*========================================================================
*
* ������:	EnumerateKey
*
* ����:		LPCWSTR	[IN] KeyFullPath	- ע���ȫ·��
*											 \Registry\Machine\XXXXX
*			PVOID	[IN] pBuffer		- ���������
*			ULONG	[IN] SubKey			- �Ӽ��ĸ���
*
* ��������:	ö��ע����
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS
EnumerateKey( LPCWSTR KeyFullPath, PVOID pBuffer, ULONG SubKey )
{
	NTSTATUS				Status=STATUS_SUCCESS;
	HANDLE					KeyHandle;
	UNICODE_STRING			uniKey;
	OBJECT_ATTRIBUTES		ObjectAttributes;
	ULONG					index;
	ULONG					ResultLength;
	PKEY_BASIC_INFORMATION	pbi;
	PKEY_FULL_INFORMATION	pfi;
	PREGISTRY_KEY			pRegKey;


	#ifdef _CHECKED
		DbgPrint( "$ixer:\tEnumerateKey( %ws, 0x%08x, %d )", KeyFullPath, pBuffer, SubKey );
	#endif

	//���������ָ��PREGISTRY_KEY����ָ��
	pRegKey = pBuffer;

	RtlInitUnicodeString( &uniKey, KeyFullPath );

	InitializeObjectAttributes(	&ObjectAttributes,
								&uniKey,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL );

	/* �򿪼� */
	Status = ZwOpenKey( &KeyHandle, KEY_ENUMERATE_SUB_KEYS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* ö���Ӽ� */
	for ( index = 0; index < SubKey; index++ )
	{
		/* ����2�Σ���һ�λ�ȡ��������С */
		ZwEnumerateKey( KeyHandle, index, KeyBasicInformation, NULL, 0, &ResultLength );
		if ( ResultLength != 0 )
		{
			pbi = (PKEY_BASIC_INFORMATION)ExAllocatePool( PagedPool, ResultLength );
			if ( pbi == NULL )
			{
				#ifdef _CHECKED
					DbgPrint( "$ixer:\tExAllocatePool()" );
				#endif

				Status = STATUS_INSUFFICIENT_RESOURCES;
				ZwClose( KeyHandle );

				return Status;
			}

			/* ��2�λ�ȡ��Ϣ */
			Status = ZwEnumerateKey( KeyHandle, index, KeyBasicInformation, pbi, ResultLength, &ResultLength );
			if (!NT_SUCCESS(Status))
			{
				#ifdef _CHECKED
					DbgPrint( "$ixer:\tZwEnumerateKey:\t0x%08x", Status );
				#endif

				ExFreePool( pbi );
				ZwClose( KeyHandle );

				return Status;
			}

			if ( pbi->NameLength > MAX_FILE_NAME )
			{
				#ifdef _CHECKED
					DbgPrint( "$ixer:\tNameLength:\t0x%x", pbi->NameLength );
				#endif

				Status = STATUS_INSUFFICIENT_RESOURCES;
				ExFreePool( pbi );
				ZwClose( KeyHandle );

				return Status;
			}

			/* ��������������� */
			memset( pRegKey[index].Name, 0, MAX_FILE_NAME );
			//����
			RtlCopyMemory( pRegKey[index].Name, pbi->Name, pbi->NameLength );

			ExFreePool( pbi );
		} // if
	} // for

	ZwClose( KeyHandle );

	#ifdef _CHECKED
		DbgPrint( "$ixer:\tEnumerateKey() end:\t0x%08x", Status );
	#endif

	return Status;
}


/*========================================================================
*
* ������:	EnumerateValueKey
*
* ����:		LPCWSTR	[IN]	KeyFullPath	- ע���ȫ·��
*										  \Registry\Machine\XXXXX
*			PVOID	[IN]	pBuffer		- ���������
*			ULONG	[IN]	Value		- ֵ�ĸ���
*
* ��������:	ö��ע���ֵ
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS
EnumerateValueKey( LPCWSTR KeyFullPath, PVOID pBuffer, ULONG Value )
{
	NTSTATUS					Status=STATUS_SUCCESS;
	HANDLE						KeyHandle;
	UNICODE_STRING				uniKey;
	OBJECT_ATTRIBUTES			ObjectAttributes;
	ULONG						index;
	ULONG						ResultLength;
	PKEY_VALUE_FULL_INFORMATION	pvi;
	PREGISTRY_VALUE				pRegValue;


	pRegValue = pBuffer;

	#ifdef _CHECKED
		DbgPrint( "$ixer:\tEnumerateValueKey( %ws, 0x%08x, %d )", KeyFullPath, pBuffer, Value );
	#endif

	RtlInitUnicodeString( &uniKey, KeyFullPath );

	InitializeObjectAttributes(	&ObjectAttributes,
								&uniKey,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL );
	/* �򿪼� */
	Status = ZwOpenKey( &KeyHandle, KEY_ENUMERATE_SUB_KEYS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* ö��ֵ */
	for ( index = 0; index < Value; index++ )
	{
		/* ����2�Σ���һ�λ�ȡ��������С */
		ZwEnumerateValueKey( KeyHandle, index, KeyValueFullInformation, NULL, 0, &ResultLength );
		if ( ResultLength != 0 )
		{
			pvi = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool( PagedPool, ResultLength );
			if ( pvi == NULL )
			{
				#ifdef _CHECKED
					DbgPrint( "$ixer:\tExAllocatePool()" );
				#endif

				Status = STATUS_INSUFFICIENT_RESOURCES;
				ZwClose( KeyHandle );

				return Status;
			}

			/* ��2�λ�ȡ��Ϣ */
			Status = ZwEnumerateValueKey( KeyHandle, index, KeyValueFullInformation, pvi, ResultLength, &ResultLength );
			if (!NT_SUCCESS(Status))
			{
				#ifdef _CHECKED
					DbgPrint( "$ixer:\tZwEnumerateKey:\t0x%08x", Status );
				#endif

				ExFreePool( pvi );
				ZwClose( KeyHandle );

				return Status;
			}

			/* ������Ϣ */
			pRegValue->Type = pvi->Type;
			pRegValue->DataOffset = sizeof(REGISTRY_VALUE) + pvi->NameLength - 1;
			pRegValue->DataLength = pvi->DataLength;
			pRegValue->NameLength = pvi->NameLength;

			RtlCopyMemory( pRegValue->Name, pvi->Name, pvi->NameLength );
			RtlCopyMemory( (PVOID)((ULONG)pRegValue + pRegValue->DataOffset), (PVOID)(pvi->DataOffset + (ULONG)pvi), pvi->DataLength );

			pRegValue = (PREGISTRY_VALUE)((ULONG)pRegValue + sizeof(REGISTRY_VALUE) + pvi->NameLength + pvi->DataLength - 1);
			
			ExFreePool( pvi );
		} // end if
	} // end for

	ZwClose( KeyHandle );

	#ifdef _CHECKED
		DbgPrint( "$ixer:\tEnumerateValueKey() end:\t0x%08x", Status );
	#endif

	return Status;
}

NTSTATUS DeleteRegKey( LPCWSTR KeyFullPath )
{
	NTSTATUS				status=STATUS_SUCCESS;
	HANDLE					KeyHandle, subKeyHandle;
	UNICODE_STRING			uniKeyPath, uniKeyName;
	OBJECT_ATTRIBUTES		ObjectAttr, subKeyObjAttr;
	ULONG					ulKeyInfoLen;
	PKEY_FULL_INFORMATION	pKeyFullInfo;
	PKEY_BASIC_INFORMATION	pKeyBasicInfo;
	ULONG					i;


	KdPrint(( "[DeleteKey]ZwDeleteKey( %ws )\n", KeyFullPath ));

	RtlInitUnicodeString( &uniKeyPath, KeyFullPath );

	InitializeObjectAttributes(	&ObjectAttr,
								&uniKeyPath,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL );
	/* ��key */
	status = ZwOpenKey( &KeyHandle, KEY_ALL_ACCESS, &ObjectAttr );
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(( "[DeleteKey]ZwOpenKey( %wZ ) failed! status: 0x%08x\n", &uniKeyPath, status ));
		return status;
	}

	/* ɾ��key */
	status = ZwDeleteKey( KeyHandle );
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(( "[DeleteKey]ZwDeleteKey( %wZ ) failed! status: 0x%08x\n", &uniKeyPath, status ));

		ZwClose( KeyHandle);
		return status;
	}

	KdPrint(( "[DeleteKey]ZwDeleteKey( %wZ ) status: 0x%08x\n", &uniKeyPath, status ));
	ZwClose( KeyHandle );
	return status;
}
/*========================================================================
*
* ������:	DeleteValueKey
*
* ����:		LPCWSTR	[IN]	KeyFullPath	- ע���ȫ·��
*										  \Registry\Machine\XXXXX
*			LPCWSTR	[IN]	Value		- ֵ����
*
* ��������:	ɾ��ע���ֵ
*
* ����ֵ:	NTSTATUS
*
=========================================================================*/
NTSTATUS
DeleteValueKey(  LPCWSTR KeyFullPath, LPCWSTR Value )
{
	NTSTATUS					Status=STATUS_SUCCESS;
	HANDLE						KeyHandle;
	UNICODE_STRING				uniKey;
	UNICODE_STRING				uniValue;
	OBJECT_ATTRIBUTES			ObjectAttributes;


	#ifdef _CHECKED
		DbgPrint( "$ixer:\tDeleteValueKey( %ws, %ws )", KeyFullPath, Value );
	#endif

	RtlInitUnicodeString( &uniKey, KeyFullPath );
	RtlInitUnicodeString( &uniValue, Value );

	InitializeObjectAttributes(	&ObjectAttributes,
								&uniKey,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL );
	/* �򿪼� */
	Status = ZwOpenKey( &KeyHandle, KEY_ALL_ACCESS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* ɾ����ֵ */
	Status = ZwDeleteValueKey( KeyHandle, &uniValue );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwDeleteValueKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	ZwClose( KeyHandle );

	#ifdef _CHECKED
		DbgPrint( "$ixer:\tDeleteValueKey() end:\t0x%08x", Status );
	#endif

	return Status;
}