/*===================================================================
 * Filename reg.c
 * 
 * Author: Chinghoi
 * Email:  chinghoi@vip.qq.com
 *
 * Description: 注册表模块
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
* 函数名:	QueryKeyItemInfo
*
* 参数:		LPCWSTR		[IN]	- KeyFullPath
*			PULONG		[OUT]	- pNumber		实际是一个数组
*								  pNumber[0]	subKey(子项) 的个数
*								  pNumber[1]	keyValue 的个数
*								  pNumber[2]	Value Name + Data的总缓冲区
*			ULONG		[IN]	- Size		- 缓冲区的大小
*
* 功能描述:	获取注册表键信息
*
* 返回值:	NTSTATUS
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

	/* 打开键，枚举权限 */
	Status = ZwOpenKey( &KeyHandle, KEY_ENUMERATE_SUB_KEYS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* 调用2次，第一次获取缓冲区大小 */
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

		/* 第二次调用，获取子项信息 */
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

		/* 输出 */
		pNumber[0] = pfi->SubKeys;	//subKey数
		pNumber[1] = pfi->Values;	//keyValue数
		pNumber[2] = 0;

		for ( index = 0; index < pfi->Values; index++ )
		{
			/* 枚举键值尺寸 */
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

				/* 枚举键值信息 */
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
* 函数名:	EnumerateKey
*
* 参数:		LPCWSTR	[IN] KeyFullPath	- 注册表全路径
*											 \Registry\Machine\XXXXX
*			PVOID	[IN] pBuffer		- 输出缓冲区
*			ULONG	[IN] SubKey			- 子键的个数
*
* 功能描述:	枚举注册表键
*
* 返回值:	NTSTATUS
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
	PREGISTRY_KEY			pRegKey;


	#ifdef _CHECKED
		DbgPrint( "$ixer:\tEnumerateKey( %ws, 0x%08x, %d )", KeyFullPath, pBuffer, SubKey );
	#endif

	//输出缓冲区指向PREGISTRY_KEY类型指针
	pRegKey = pBuffer;

	RtlInitUnicodeString( &uniKey, KeyFullPath );

	InitializeObjectAttributes(	&ObjectAttributes,
								&uniKey,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL );

	/* 打开键 */
	Status = ZwOpenKey( &KeyHandle, KEY_ENUMERATE_SUB_KEYS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* 枚举子键 */
	for ( index = 0; index < SubKey; index++ )
	{
		/* 调用2次，第一次获取缓冲区大小 */
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

			/* 第2次获取信息 */
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

			/* 拷贝到输出缓冲区 */
			memset( pRegKey[index].Name, 0, MAX_FILE_NAME );
			//键名
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
* 函数名:	EnumerateValueKey
*
* 参数:		LPCWSTR	[IN]	KeyFullPath	- 注册表全路径
*										  \Registry\Machine\XXXXX
*			PVOID	[IN]	pBuffer		- 输出缓冲区
*			ULONG	[IN]	Value		- 值的个数
*
* 功能描述:	枚举注册表值
*
* 返回值:	NTSTATUS
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
	/* 打开键 */
	Status = ZwOpenKey( &KeyHandle, KEY_ENUMERATE_SUB_KEYS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* 枚举值 */
	for ( index = 0; index < Value; index++ )
	{
		/* 调用2次，第一次获取缓冲区大小 */
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

			/* 第2次获取信息 */
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

			/* 保存信息 */
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
	HANDLE					KeyHandle;
	UNICODE_STRING			uniKeyPath;
	OBJECT_ATTRIBUTES		ObjectAttr;


	KdPrint(( "[DeleteKey]ZwDeleteKey( %ws )\n", KeyFullPath ));

	RtlInitUnicodeString( &uniKeyPath, KeyFullPath );

	InitializeObjectAttributes(	&ObjectAttr,
								&uniKeyPath,
								OBJ_CASE_INSENSITIVE,
								NULL,
								NULL );
	/* 打开key */
	status = ZwOpenKey( &KeyHandle, KEY_ALL_ACCESS, &ObjectAttr );
	if ( !NT_SUCCESS(status) )
	{
		KdPrint(( "[DeleteKey]ZwOpenKey( %wZ ) failed! status: 0x%08x\n", &uniKeyPath, status ));
		return status;
	}

	/* 删除key */
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
* 函数名:	DeleteValueKey
*
* 参数:		LPCWSTR	[IN]	KeyFullPath	- 注册表全路径
*										  \Registry\Machine\XXXXX
*			LPCWSTR	[IN]	Value		- 值名称
*
* 功能描述:	删除注册表值
*
* 返回值:	NTSTATUS
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
	/* 打开键 */
	Status = ZwOpenKey( &KeyHandle, KEY_ALL_ACCESS, &ObjectAttributes );
	if (!NT_SUCCESS(Status))
	{
		#ifdef _CHECKED
			DbgPrint( "$ixer:\tZwOpenKey:\t0x%08x", Status );
		#endif

		return Status;
	}

	/* 删除键值 */
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