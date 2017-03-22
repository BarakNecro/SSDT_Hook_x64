#include"SSDT_Hook.h"

#define SETBIT(x,y) x|=(1<<y) //��X�ĵ�Yλ��1
#define CLRBIT(x,y) x&=~(1<<y) //��X�ĵ�Yλ��0
#define GETBIT(x,y) (x & (1 << y)) //ȡX�ĵ�Yλ������0���0

void WPOFF()
{
	_disable(); //disable interrupts
	__writecr0(__readcr0() & (~(0x10000)));
}
void WPON()
{
	__writecr0(__readcr0() ^ 0x10000);
	_enable();
}


ULONGLONG GetKeServiceDescriptorTableShadowAddrX64()
{
	PUCHAR StartSearchAddress = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR EndSearchAddress = StartSearchAddress + 0x500;
	PUCHAR i = NULL;
	UCHAR b1 = 0, b2 = 0, b3 = 0;
	ULONGLONG templong = 0;
	ULONGLONG addr = 0;
	for (i = StartSearchAddress; i<EndSearchAddress; i++)
	{
		if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
		{
			b1 = *i;
			b2 = *(i + 1);
			b3 = *(i + 2);
			if (b1 == 0x4c && b2 == 0x8d && b3 == 0x1d) //4c8d1d
			{
				memcpy(&templong, i + 3, 4);
				addr = (ULONGLONG)templong + (ULONGLONG)i + 7;
				/*DbgPrint("b1 = %0x\n", b1);
				DbgPrint("b2 = %0x\n", b2);
				DbgPrint("b3 = %0x", b3);
				DbgPrint("templong = %lx\n", templong);
				DbgPrint("SSDT Shadow addr = %x\n", addr);
				//	DbgPrint("SSDT Shadow addr = %x\n", addr+8);*/

				return addr;
			}
		}
	}
	return 0;
}
//��ȡSSDT ��ĵ�ַ����ʵ�� SYSTEM_SERVICE_TABLE�ṹ�ĵ�ַ���ýṹ�е�ServiceTableBase��Ա��¼��SSDT�Ļ�ַ
ULONGLONG  GetKeServiceDescriptorTableAddrX64()
{
	PUCHAR StartSearchAddress = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR EndSearchAddress = StartSearchAddress + 0x500;
	PUCHAR i = NULL;
	UCHAR b1 = 0, b2 = 0, b3 = 0;
	ULONGLONG templong = 0;
	ULONGLONG addr = 0;
	for (i = StartSearchAddress; i<EndSearchAddress; i++)
	{
		if (MmIsAddressValid(i) && MmIsAddressValid(i + 1) && MmIsAddressValid(i + 2))
		{
			b1 = *(i);
			b2 = *(i + 1);
			b3 = *(i + 2);
			if (b1 == 0x4c && b2 == 0x8d && b3 == 0x15)
			{
				memcpy(&templong, i + 3, 4);
				//���Ĳ���  
				//kd> db fffff800`03e8b772  
				//fffff800`03e8b772  4c 8d 15 c7 20 23 00 4c-8d 1d 00 21 23 00 f7 83  L... #.L...!#...  
				//templong = 002320c7 ,i = 03e8b772, 7Ϊָ���  
				addr = (ULONGLONG)templong + (ULONGLONG)i + 7;
				break;
			}
		}
	}
	DbgPrint("b1 = %0x\n", b1);
	DbgPrint("b2 = %0x\n", b2);
	DbgPrint("b3 = %0x", b3);
	DbgPrint("templong = %x\n", templong);
	DbgPrint("SSDT addr = %x\n", addr);
	//DbgPrint("SSDT addr = %x\n", addr+8);
	return addr;
}
//ͨ������ֵ���±꣩��ȡSSDT���ж�Ӧ�ĺ�����ַ
ULONGLONG GetSSDTFuncCurAddrByIndex(ULONG index)
{
	LONG dwtmp = 0;
	ULONGLONG addr = 0;
	PULONG ServiceTableBase = NULL;
	ServiceTableBase = (PULONG)KeServiceDescriptorTable->ServiceTableBase;
	dwtmp = ServiceTableBase[index];
	dwtmp = dwtmp >> 4;
	addr = ((LONGLONG)dwtmp + (ULONGLONG)ServiceTableBase);//&0xFFFFFFF0;
	return addr;
}

NTSTATUS __fastcall Fake_NtCreateFile(
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
)
{
	NTSTATUS st;
	st = NtCreateFile(FileHandle, DesiredAccess, ObjectAttributes, IoStatusBlock, AllocationSize, FileAttributes, ShareAccess, CreateDisposition, CreateOptions, EaBuffer, EaLength);
	DbgPrint("Fake_NtCreateFile called: %x", st);
	DbgPrint("%s", *ObjectAttributes->ObjectName);
	DbgPrint("%s", ObjectAttributes->RootDirectory);
	//ObjectAttributes->

	return st;
}

//�����Լ���NtOpenProcess  
NTSTATUS __stdcall Fake_NtOpenProcess(OUT PHANDLE  ProcessHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes,
	IN OPTIONAL PCLIENT_ID  ClientId)
{
	PEPROCESS process = NULL;
	//NTSTATUS rtStatus = ObReferenceObjectByHandle(ProcessHandle,FILE_READ_DATA, NULL, KernelMode, &process, NULL);
	NTSTATUS st = ObReferenceObjectByHandle(ProcessHandle, 0, *PsProcessType, KernelMode, (PVOID*)&process, NULL);
	DbgPrint("����HOOK����.\n");
	if (NT_SUCCESS(st))
	{
		if (!_stricmp((char*)PsGetProcessImageFileName(process), "notepad.exe"))
		{

			return STATUS_ACCESS_DENIED;
		}
		else
		{
			return OldOpenProcess(ProcessHandle, DesiredAccess, ObjectAttributes, ClientId);
		}
	}
	else
	{
		return STATUS_ACCESS_DENIED;
	}
}



ULONG GetOffsetAddress(ULONGLONG FuncAddr, CHAR ParamCount)
{
	LONG dwtmp = 0, i;
	CHAR b = 0, bits[4] = { 0 };
	PULONG ServiceTableBase = NULL;
	ServiceTableBase = (PULONG)KeServiceDescriptorTable->ServiceTableBase;
	dwtmp = (LONG)(FuncAddr - (ULONGLONG)ServiceTableBase);
	dwtmp = dwtmp << 4;
	//�������
	if (ParamCount>4)
		ParamCount = ParamCount - 4;
	else
		ParamCount = 0;
	//���dwtmp�ĵ�һ���ֽ�
	memcpy(&b, &dwtmp, 1);
	//�������λ����д��������
	for (i = 0; i<4; i++)
	{
		bits[i] = GETBIT(ParamCount, i);
		if (bits[i])
			SETBIT(b, i);
		else
			CLRBIT(b, i);
	}
	//�����ݸ��ƻ�ȥ
	memcpy(&dwtmp, &b, 1);
	return dwtmp;
}

//�ں����ò����ķ�����������ת��(�Լ���NtOpenProcess����KeBugCheckEx������Ȼ����KeBugCheckEx��������ҪHook��NtOpenProcess)  
VOID FuckKeBugCheckEx()
{
	KIRQL irql;
	ULONGLONG myfun;
	UCHAR jmp_code[] = "\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";
	myfun = (ULONGLONG)Fake_NtOpenProcess;
	memcpy(jmp_code + 6, &myfun, 8);
	irql = WPOFFx64();
	memset(KeBugCheckEx, 0x90, 15);
	memcpy(KeBugCheckEx, jmp_code, 14);
	WPONx64(irql);
}

VOID HookSSDT()
{
	KIRQL irql;
	LONG dwtmp = 0;
	PULONG ServiceTableBase = NULL;
	//get old address
	DbgPrint("NtOpenProcess index: %ld", index);
	OldOpenProcess = (NTOPENPROCESS)GetSSDTFuncCurAddrByIndex(index);
	DbgPrint("Old_OpenProcess: %llx", (ULONGLONG)OldOpenProcess);
	//show new address
	ServiceTableBase = (PULONG)KeServiceDescriptorTable->ServiceTableBase;
	//get offset value
	//dwtmp = GetOffsetAddress((ULONGLONG)KeBugCheckEx, 11);//ȥ������ҲûӰ��
	//set kebugcheckex
	FuckKeBugCheckEx();
	//record old offset value
	OldTpVal = ServiceTableBase[index];
	irql = WPOFFx64();
	ServiceTableBase[index] = GetOffsetAddress((ULONGLONG)KeBugCheckEx, 4);
	WPONx64(irql);
	DbgPrint("KeBugCheckEx: %llx", (ULONGLONG)KeBugCheckEx);
	DbgPrint("New_OpenProcess: %llx", GetSSDTFuncCurAddrByIndex(index));
}

VOID UnhookSSDT()
{
	KIRQL irql;
	PULONG ServiceTableBase = NULL;
	ServiceTableBase = (PULONG)KeServiceDescriptorTable->ServiceTableBase;
	//set value
	irql = WPOFFx64();
	ServiceTableBase[index] = OldTpVal;	//GetOffsetAddress((ULONGLONG)NtCreateFile);
	WPONx64(irql);
}


#pragma INITCODE
extern "C" NTSTATUS DriverEntry(
	IN PDRIVER_OBJECT pDriverObject,
	IN PUNICODE_STRING pRegistryPath)
{
	//DbgBreakPoint();
	NTSTATUS status = STATUS_SUCCESS;
	KdPrint(("Enter DriverEntry\n"));
	KeServiceDescriptorTable = (PSYSTEM_SERVICE_TABLE)GetKeServiceDescriptorTableAddrX64();
	GetKeServiceDescriptorTableShadowAddrX64();
	ULONG NtTerminateAddr = GetSSDTFuncCurAddrByIndex(41);
	DbgPrint("NtTerminateAddr %x", NtTerminateAddr);

	HookSSDT();


	KdPrint(("DriverEntry end\n"));
	return status;
}