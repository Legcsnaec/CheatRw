#include "ReadWrite.h"
#include <intrin.h>
#define PROCESS_RUNCODE		0x103
#define EPROCESS_CR3_OFFSET 0x28

// 读方法1,切Cr3直接读
NTSTATUS ReadMemory1(HANDLE pid, PVOID tagAddress, PVOID readBuffer, SIZE_T readSize)
{
	if (tagAddress >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) < tagAddress)
	{
		return STATUS_ACCESS_VIOLATION;
	}
	if (readBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	NTSTATUS stat = STATUS_UNSUCCESSFUL;
	PEPROCESS eprocess = NULL;
	KAPC_STATE apcState = { 0 };
	stat = PsLookupProcessByProcessId(pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		return stat;
	}
	if (PsGetProcessExitStatus(eprocess) != PROCESS_RUNCODE)
	{
		// 进程退出了
		stat = STATUS_INVALID_PARAMETER_1;
		goto end;
	}

	PVOID mem = ExAllocatePool(NonPagedPool, readSize);
	if (mem == NULL)
	{
		stat = STATUS_INVALID_ADDRESS;
		goto end;
	}
	memset(mem, 0, readSize);

	// 切cr3直接读
	stat = STATUS_UNSUCCESSFUL;
	ULONG64 newCr3 = *(PULONG64)((ULONG64)eprocess + EPROCESS_CR3_OFFSET);
	ULONG64 oldCr3 = __readcr3();

	KeEnterCriticalRegion();
	_disable();
	__writecr3(newCr3);
	if (MmIsAddressValid(tagAddress) && MmIsAddressValid((PVOID)((ULONG64)tagAddress + readSize)))
	{
		memcpy(mem, tagAddress, readSize);
		stat = STATUS_SUCCESS;
	}
	_enable();
	__writecr3(oldCr3);
	KeLeaveCriticalRegion();

	if (NT_SUCCESS(stat))
	{
		memcpy(readBuffer, mem, readSize);
	}
	ExFreePool(mem);
end:
	ObDereferenceObject(eprocess);
	return stat;
}

NTSTATUS ReadMemory(HANDLE pid, PVOID tagAddress, PVOID readBuffer, SIZE_T readSize)
{
	return ReadMemory1(pid, tagAddress, readBuffer, readSize);
}