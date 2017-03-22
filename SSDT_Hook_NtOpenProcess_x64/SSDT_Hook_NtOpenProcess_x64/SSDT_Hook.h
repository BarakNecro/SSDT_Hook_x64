#pragma once
#pragma once
#pragma once
#include <ntddk.h>

typedef struct _SYSTEM_SERVICE_TABLE {
	PVOID  		ServiceTableBase;
	PVOID  		ServiceCounterTableBase;
	ULONGLONG  	NumberOfServices;
	PVOID  		ParamTableBase;
} SYSTEM_SERVICE_TABLE, *PSYSTEM_SERVICE_TABLE;

PSYSTEM_SERVICE_TABLE KeServiceDescriptorTable;
//����ҪѰ�ҽ������õĺ���
EXTERN_C NTKERNELAPI UCHAR * PsGetProcessImageFileName(PEPROCESS Process);

typedef struct _SERVICE_DESCRIPTOR_TABLE {
	SYSTEM_SERVICE_TABLE ntoskrnl;  // ntoskrnl.exe (native api)
	SYSTEM_SERVICE_TABLE win32k;    // win32k.sys   (gdi/user)
	SYSTEM_SERVICE_TABLE Table3;    // not used
	SYSTEM_SERVICE_TABLE Table4;    // not used
}SERVICE_DESCRIPTOR_TABLE, *PSERVICE_DESCRIPTOR_TABLE;

typedef NTSTATUS(__fastcall *NTCREATEFILE)(
	__out     PHANDLE FileHandle,
	__in      ACCESS_MASK DesiredAccess,
	__in      POBJECT_ATTRIBUTES ObjectAttributes,
	__out     PIO_STATUS_BLOCK IoStatusBlock,
	__in_opt  PLARGE_INTEGER AllocationSize,
	__in      ULONG FileAttributes,
	__in      ULONG ShareAccess,
	__in      ULONG CreateDisposition,
	__in      ULONG CreateOptions,
	__in      PVOID EaBuffer,
	__in      ULONG EaLength
	);
NTCREATEFILE NtCreateFile = NULL;


//����NTOPENPROCESS  
typedef  NTSTATUS(__stdcall *NTOPENPROCESS)(OUT PHANDLE  ProcessHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes,
	IN OPTIONAL PCLIENT_ID  ClientId);

NTOPENPROCESS OldOpenProcess = NULL;


//�������  
EXTERN_C __int64 __readmsr(int);
EXTERN_C unsigned __int64 __readcr0(void);
EXTERN_C void __writecr0(
	unsigned __int64 Data
);
EXTERN_C void _disable(void);
EXTERN_C void _enable(void);

typedef NTSTATUS(__fastcall *NTCREATEFILE)(
	__out     PHANDLE FileHandle,
	__in      ACCESS_MASK DesiredAccess,
	__in      POBJECT_ATTRIBUTES ObjectAttributes,
	__out     PIO_STATUS_BLOCK IoStatusBlock,
	__in_opt  PLARGE_INTEGER AllocationSize,
	__in      ULONG FileAttributes,
	__in      ULONG ShareAccess,
	__in      ULONG CreateDisposition,
	__in      ULONG CreateOptions,
	__in      PVOID EaBuffer,
	__in      ULONG EaLength
	);


ULONG OldTpVal;
ULONG index = 35;	//��HOOK������INDEX 


					//implementation
					//�ر�ҳ�汣��
KIRQL WPOFFx64()
{
	KIRQL irql = KeRaiseIrqlToDpcLevel();
	UINT64 cr0 = __readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);
	_disable();
	return irql;
}
//����ҳ�汣��  
void WPONx64(KIRQL irql)
{
	UINT64 cr0 = __readcr0();
	cr0 |= 0x10000;
	_enable();
	__writecr0(cr0);
	KeLowerIrql(irql);
}