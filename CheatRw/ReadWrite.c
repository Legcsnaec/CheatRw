#include <intrin.h>
#include "ReadWrite.h"
#include "CheatTools.h"

#define PROCESS_RUNCODE			0x103
#define EPROCESS_CR3_OFFSET		0x28

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

	PEPROCESS eprocess = NULL;
	NTSTATUS stat = PsLookupProcessByProcessId(pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		return stat;
	}
	if (PsGetProcessExitStatus(eprocess) != PROCESS_RUNCODE)
	{
		stat = STATUS_INVALID_PARAMETER_1;
		goto end;
	}

	PVOID tmpMem = ExAllocatePool(NonPagedPool, readSize);
	if (tmpMem == NULL)
	{
		stat = STATUS_INVALID_ADDRESS;
		goto end;
	}
	RtlZeroMemory(tmpMem, readSize);

	stat = STATUS_UNSUCCESSFUL;
	ULONG64 newCr3 = *(PULONG64)((ULONG64)eprocess + EPROCESS_CR3_OFFSET);
	ULONG64 oldCr3 = __readcr3();

	KeEnterCriticalRegion();
	_disable();
	__writecr3(newCr3);
	if (tagAddress && MmIsAddressValid(tagAddress) && MmIsAddressValid((PVOID)((ULONG64)tagAddress + readSize)))
	{
		memcpy(tmpMem, tagAddress, readSize);
		stat = STATUS_SUCCESS;
	}
	_enable();
	__writecr3(oldCr3);
	KeLeaveCriticalRegion();

	if (NT_SUCCESS(stat))
	{
		memcpy(readBuffer, tmpMem, readSize);
	}
	ExFreePool(tmpMem);
end:
	ObDereferenceObject(eprocess);
	return stat;
}

// 读方法2,使用MmCopyVirtualMemory(最好的方式)
NTSTATUS ReadMemory2(HANDLE pid, PVOID tagAddress, PVOID readBuffer, SIZE_T readSize)
{
	if (tagAddress >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) < tagAddress)
	{
		return STATUS_ACCESS_VIOLATION;
	}
	if (readBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	PEPROCESS eprocess = NULL;
	NTSTATUS stat = PsLookupProcessByProcessId(pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		return stat;
	}
	if (PsGetProcessExitStatus(eprocess) != PROCESS_RUNCODE)
	{
		stat = STATUS_INVALID_PARAMETER_1;
		goto end;
	}

	SIZE_T retSize = 0;
	stat = CT_MmCopyVirtualMemory(eprocess, tagAddress, IoGetCurrentProcess(), readBuffer, readSize, UserMode, &retSize);

end:
	ObDereferenceObject(eprocess);
	return stat;
}

// 读方法3,附加读
NTSTATUS ReadMemory3(HANDLE pid, PVOID tagAddress, PVOID readBuffer, SIZE_T readSize)
{
	if (tagAddress >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) < tagAddress)
	{
		return STATUS_ACCESS_VIOLATION;
	}
	if (readBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	PEPROCESS eprocess = NULL;
	KAPC_STATE apcState = { 0 };
	NTSTATUS stat = PsLookupProcessByProcessId(pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		return stat;
	}
	if (PsGetProcessExitStatus(eprocess) != PROCESS_RUNCODE)
	{
		stat = STATUS_INVALID_PARAMETER_1;
		goto end;
	}

	PVOID tmpMem = ExAllocatePool(NonPagedPool, readSize);
	if (tmpMem == NULL)
	{
		stat = STATUS_INVALID_ADDRESS;
		goto end;
	}
	RtlZeroMemory(tmpMem, readSize);

	stat = STATUS_UNSUCCESSFUL;
	KeStackAttachProcess(eprocess, &apcState);
	if (tagAddress && MmIsAddressValid(tagAddress) && MmIsAddressValid((PVOID)((ULONG64)tagAddress + readSize)))
	{
		memcpy(tmpMem, tagAddress, readSize);
		stat = STATUS_SUCCESS;
	}
	KeUnstackDetachProcess(&apcState);

	if (stat == STATUS_SUCCESS)
	{
		memcpy(readBuffer, tmpMem, readSize);
	}
	ExFreePool(tmpMem);
end:
	ObDereferenceObject(eprocess);
	return stat;
}

// 读方法4,MDL读,此方法也需要附加,会锁一下页
// 需要本驱动能有异常处理,倘若内存加载驱动没修复异常则不可用
NTSTATUS ReadMemory4(HANDLE pid, PVOID tagAddress, PVOID readBuffer, SIZE_T readSize)
{
	if (tagAddress >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) >= MmHighestUserAddress || ((ULONG64)tagAddress + readSize) < tagAddress)
	{
		return STATUS_ACCESS_VIOLATION;
	}
	if (readBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	PEPROCESS eprocess = NULL;
	KAPC_STATE apcState = { 0 };
	NTSTATUS stat = PsLookupProcessByProcessId(pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		return stat;
	}
	if (PsGetProcessExitStatus(eprocess) != PROCESS_RUNCODE)
	{
		stat = STATUS_INVALID_PARAMETER_1;
		goto end;
	}
	PVOID tmpMem = ExAllocatePool(NonPagedPool, readSize);
	if (tmpMem == NULL)
	{
		stat = STATUS_INVALID_ADDRESS;
		goto end;
	}
	RtlZeroMemory(tmpMem, readSize);

	stat = STATUS_UNSUCCESSFUL;
	KeStackAttachProcess(eprocess, &apcState);
	if (tagAddress && MmIsAddressValid(tagAddress) && MmIsAddressValid((PVOID)((ULONG64)tagAddress + readSize)))
	{
		PMDL mdl = NULL;
		PVOID mapAddr = MdlMapMemory(&mdl, tagAddress, readSize, UserMode);
		if (mapAddr)
		{
			memcpy(tmpMem, mapAddr, readSize);
			stat = STATUS_SUCCESS;
		}
		MdlUnMapMemory(mdl, mapAddr);
	}
	KeUnstackDetachProcess(&apcState);

	if (NT_SUCCESS(stat))
	{
		memcpy(readBuffer, tmpMem, readSize);
	}
	ExFreePool(tmpMem);
end:
	ObDereferenceObject(eprocess);
	return stat;
}

// 驱动读
NTSTATUS ReadMemory(HANDLE pid, PVOID tagAddress, PVOID readBuffer, SIZE_T readSize)
{
	// 1最快、2最兼容、3取中间
	// 快也快不了多少,最推荐2
	return ReadMemory2(pid, tagAddress, readBuffer, readSize);
}

// 驱动写
NTSTATUS WriteMemory(HANDLE pid, PVOID tagAddress, PVOID writeBuffer, SIZE_T writeSize)
{
	if (tagAddress >= MmHighestUserAddress || ((ULONG64)tagAddress + writeSize) >= MmHighestUserAddress || ((ULONG64)tagAddress + writeSize) < tagAddress)
	{
		return STATUS_ACCESS_VIOLATION;
	}
	if (writeBuffer == NULL)
	{
		return STATUS_INVALID_PARAMETER_3;
	}

	PEPROCESS eprocess = NULL;
	KAPC_STATE apcState = { 0 };
	NTSTATUS stat = PsLookupProcessByProcessId(pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		return stat;
	}
	if (PsGetProcessExitStatus(eprocess) != PROCESS_RUNCODE)
	{
		stat = STATUS_INVALID_PARAMETER_1;
		goto end;
	}

	// 1.上来就MmCopy
	SIZE_T retSize = 0;
	PEPROCESS srcEprocess = IoGetCurrentProcess();
	stat = CT_MmCopyVirtualMemory(srcEprocess, writeBuffer, eprocess, tagAddress, writeSize, UserMode, &retSize);
	if (NT_SUCCESS(stat) == TRUE)
	{
		goto end;
	}

	// 2.修改页属性,再MmCopy
	ULONG oldProtect = 0;
	KeStackAttachProcess(eprocess, &apcState);
	stat = CT_ZwProtectVirtualMemory(tagAddress, writeSize, PAGE_EXECUTE_READWRITE, &oldProtect);
	if (NT_SUCCESS(stat) == TRUE)
	{
		retSize = 0;
		stat = CT_MmCopyVirtualMemory(srcEprocess, writeBuffer, eprocess, tagAddress, writeSize, UserMode, &retSize);
		CT_ZwProtectVirtualMemory(tagAddress, writeSize, oldProtect, &oldProtect);
	}
	KeUnstackDetachProcess(&apcState);

	// 3.关写保护,再MmCopy
	if (NT_SUCCESS(stat) == FALSE)
	{
		ULONG64 oldCr0 = wpoff();
		retSize = 0;
		stat = CT_MmCopyVirtualMemory(srcEprocess, writeBuffer, eprocess, tagAddress, writeSize, UserMode, &retSize);
		wpon(oldCr0);
	}

end:
	ObDereferenceObject(eprocess);
	return stat;
}