#include "Module.h"

// 32位
ULONG_PTR GetModuleX86(PEPROCESS Process, PPEB32 peb32, PUNICODE_STRING moudleName, PULONG_PTR sizeImage)
{
	NTSTATUS stus = STATUS_SUCCESS;
	ULONG_PTR retBase = 0;
	ULONG_PTR copied = 0;
	stus = MmCopyVirtualMemory(Process, peb32, Process, peb32, 0x1, UserMode, &copied);
	if (!NT_SUCCESS(stus)) return 0;

	PPEB_LDR_DATA32 pebLdr = (PPEB_LDR_DATA32)UlongToPtr(peb32->Ldr);
	PLIST_ENTRY32 pList = (PLIST_ENTRY32)&pebLdr->InLoadOrderModuleList;
	PLDR_DATA_TABLE_ENTRY32 plistNext = (PLDR_DATA_TABLE_ENTRY32)ULongToPtr(pList->Flink);

	while ((ULONG64)pList != (ULONG64)plistNext)
	{
		PWCH baseDllName = (PWCH)plistNext->BaseDllName.Buffer;
		UNICODE_STRING uBaseName = { 0 };
		RtlInitUnicodeString(&uBaseName, baseDllName);
		if (RtlCompareUnicodeString(moudleName, &uBaseName, TRUE) == 0)
		{
			KdPrint(("[hotge]:imageBase = %llx,sizeofimage = %llx,%wZ\r\n", (ULONG64)plistNext->DllBase, plistNext->SizeOfImage, moudleName));
			retBase = (ULONG_PTR)plistNext->DllBase;
			if (sizeImage) *sizeImage = plistNext->SizeOfImage;
			break;
		}
		plistNext = (PLDR_DATA_TABLE_ENTRY32)plistNext->InLoadOrderLinks.Flink;
	}
	return retBase;
}

// 64位
ULONG_PTR GetModuleX64(PEPROCESS Process, PPEB peb64, PUNICODE_STRING moudleName, PULONG_PTR sizeImage)
{
	NTSTATUS stus = STATUS_SUCCESS;
	ULONG_PTR retBase = 0;
	ULONG_PTR copied = 0;
	stus = MmCopyVirtualMemory(Process, peb64, Process, peb64, 0x1, UserMode, &copied);
	if (!NT_SUCCESS(stus)) return 0;

	PPEB_LDR_DATA pebLdr = peb64->Ldr;
	PLIST_ENTRY pList = (PLIST_ENTRY)&pebLdr->InLoadOrderModuleList;
	PLDR_DATA_TABLE_ENTRY plistNext = (PLDR_DATA_TABLE_ENTRY)pList->Flink;
	while ((ULONG64)pList != (ULONG64)plistNext)
	{
		if (RtlCompareUnicodeString(moudleName, &(plistNext->BaseDllName), TRUE) == 0)
		{
			KdPrint(("[hotge]:imageBase = %llx,sizeofimage = %llx,%wZ\r\n", (ULONG64)plistNext->DllBase, plistNext->SizeOfImage, moudleName));
			retBase = (ULONG_PTR)plistNext->DllBase;
			if (sizeImage) *sizeImage = plistNext->SizeOfImage;
			break;
		}
		plistNext = (PLDR_DATA_TABLE_ENTRY)plistNext->InLoadOrderLinks.Flink;
	}
	return retBase;
}

/// <summary>
/// 得到模块地址
/// </summary>
ULONG_PTR GetModuleR3(HANDLE pid, char* moduleName, PULONG_PTR sizeImage)
{
	if (moduleName == 0) return 0;

	NTSTATUS stus = STATUS_SUCCESS;
	PEPROCESS eProcess = 0;
	stus = PsLookupProcessByProcessId(pid, &eProcess);
	if (!NT_SUCCESS(stus)) return 0;

	STRING str = { 0 };
	RtlInitAnsiString(&str, moduleName);

	UNICODE_STRING uniModuleame = { 0 };
	stus = RtlAnsiStringToUnicodeString(&uniModuleame, &str, TRUE);
	if(!NT_SUCCESS(stus)) return 0;
	_wcsupr(uniModuleame.Buffer);

	ULONG_PTR module = 0;
	KAPC_STATE apcStus = { 0 };
	KeStackAttachProcess(eProcess, &apcStus);
	PPEB peb64 = (PPEB)PsGetProcessPeb(eProcess);
	PPEB32 peb32 = (PPEB32)PsGetProcessWow64Process(eProcess);
	if (!peb32)
	{
		// x64
		module = GetModuleX64(eProcess, peb64, &uniModuleame, sizeImage);
	}
	else
	{
		// x32
		module = GetModuleX86(eProcess, peb32, &uniModuleame, sizeImage);
	}
	KeUnstackDetachProcess(&apcStus);
	RtlFreeUnicodeString(&uniModuleame);
	return module;
}