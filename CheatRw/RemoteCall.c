#include "RemoteCall.h"
#include "CheatTools.h"

typedef struct _FreeMemoryInfo
{
	WORK_QUEUE_ITEM WorkItem;
	HANDLE Pid;
	ULONG64 IsExecutedAddr;
	ULONG64 FreeSize;
}FreeMemoryInfo, * PFreeMemoryInfo;

// 64位shellcode
CHAR NormalShellCodeX64[] = {
	0x50, 														//	push  rax
	0x51,														//	push  rcx
	0x52,														//	push  rdx
	0x53, 														//	push  rbx	
	0x55, 														//	push  rbp
	0x56, 														//	push  rsi
	0x57, 														//	push  rdi
	0x41, 0x50, 												//	push  r8
	0x41, 0x51, 												//	push  r9
	0x41, 0x52, 												//	push  r10
	0x41, 0x53, 												//	push  r11
	0x41, 0x54, 												//	push  r12
	0x41, 0x55, 												//	push  r13
	0x41, 0x56, 												//	push  r14
	0x41, 0x57, 												//	push  r15
	0x48, 0xB8, 0x99, 0x89, 0x67, 0x45, 0x23, 0x01, 0x00,0x00, 	//	mov  rax,0x0000012345678999
	0x48, 0x81, 0xEC, 0xA8, 0x00, 0x00, 0x00, 					//	sub  rsp,0x00000000000000A8
	0xFF, 0xD0, 												//	call  rax
	0x48, 0x81, 0xC4, 0xA8, 0x00, 0x00, 0x00, 					//	add  rsp,0x00000000000000A8
	0x41, 0x5F, 												//	pop  r15
	0x41, 0x5E,													//	pop  r14
	0x41, 0x5D, 												//	pop  r13
	0x41, 0x5C, 												//	pop  r12
	0x41, 0x5B, 												//	pop  r11
	0x41, 0x5A, 												//	pop  r10
	0x41, 0x59, 												//	pop  r9
	0x41, 0x58, 												//	pop  r8
	0x5F, 														//	pop  rdi
	0x5E, 														//	pop  rsi
	0x5D, 														//	pop  rbp
	0x5B, 														//	pop  rbx
	0x5A,														//	pop  rdx
	0x59, 														//	pop  rcx
	0x48, 0xB8, 0x89, 0x67, 0x45, 0x23, 0x01, 0x00, 0x00, 0x00, //	mov  rax,0x0000000123456789
	0x48, 0xC7, 0x00, 0x01, 0x00, 0x00, 0x00,					// 	mov  qword ptr ds:[rax],0x0000000000000001
	0x58, 														//	pop  rax
	0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,							//	jmp  qword ptr ds:[PCHunter64.00000001403ABA27]
	0x00, 0x00,													//	add  byte ptr ds:[rax],al
	0x00, 0x00,													//	add  byte ptr ds:[rax],al
	0x00, 0x00,													//	add  byte ptr ds:[rax],al
	0x00, 0x00													//	add  byte ptr ds:[rax],al
};

// 32位shellcode
CHAR NormalShellCodeX86[] = {
	0x60,														//	pushad
	0xB8, 0x78, 0x56, 0x34, 0x12,								//	mov eax,12345678
	0x83, 0xEC, 0x40,											//	sub esp,40
	0xFF, 0xD0,													//	call eax                         
	0x83, 0xC4, 0x40,											//	add esp,40
	0xB8, 0x78, 0x56, 0x34, 0x12,								//	mov eax,12345678
	0xC7, 0x00,	0x01, 0x00, 0x00,0x00,							//	mov dword ptr ds:[eax],1
	0x61,														//	popad
	0xE9,0x00,0x00,0x00,0x00									//	jmp 
};

// 延迟工作项,用于释放用户空间
VOID ExFreeMemoryWorkItem(_In_ PVOID Parameter)
{
	PFreeMemoryInfo item = (PFreeMemoryInfo)Parameter;

	PEPROCESS eprocess = NULL;
	NTSTATUS stat = PsLookupProcessByProcessId(item->Pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		goto end;
	}
	if (PsGetProcessExitStatus(eprocess) != 0x103)
	{
		goto end;
	}

	int count = 0;
	SIZE_T retSize = 0;
	ULONG64 exeValue = 0;
	BOOLEAN isSuccess = FALSE;
	while (1)
	{
		if (count > 5) break;
		stat = MmCopyVirtualMemory(eprocess, item->IsExecutedAddr, IoGetCurrentProcess(), &exeValue, 8, KernelMode, &retSize);
		if (NT_SUCCESS(stat) && exeValue == 1)
		{
			isSuccess = TRUE;
			break;
		}
		KeSleep(200);
		count++;
	}

	KAPC_STATE kApcState = { 0 };
	KeStackAttachProcess(eprocess, &kApcState);
	if (isSuccess)
	{
		BOOLEAN isWow64Process = PsGetProcessWow64Process(eprocess) != NULL ? TRUE : FALSE;
		PVOID BaseAddr = (PVOID)(item->IsExecutedAddr - 0x500);
		stat = CT_ZwFreeVirtualMemory(NtCurrentProcess(), &BaseAddr);
	}
	KeUnstackDetachProcess(&kApcState);

end:
	if (item)
	{
		ExFreePool(item);
	}
	if (eprocess)
	{
		ObDereferenceObject(eprocess);
	}
}

// 远程call
NTSTATUS RtlRemoteCall(HANDLE pid, PVOID shellCodePtr, ULONG64 shellCodeSize)
{
	if (pid == 0 || shellCodePtr == NULL || shellCodeSize == 0 || shellCodePtr >= MmHighestUserAddress || ((ULONG64)shellCodePtr + shellCodeSize) >= MmHighestUserAddress)
	{
		return STATUS_INVALID_PARAMETER;
	}

	PETHREAD ethread = NULL;
	PEPROCESS eprocess = NULL;
	NTSTATUS stat = STATUS_UNSUCCESSFUL;
	stat = PsLookupProcessByProcessId(pid, &eprocess);
	if (!NT_SUCCESS(stat))
	{
		goto end;
	}
	if (PsGetProcessExitStatus(eprocess) != 0x103)
	{
		goto end;
	}

	GetMainThreadByEprocess(eprocess, &ethread);
	if (!ethread)
	{
		goto end;
	}

	BOOLEAN isWow64Process = FALSE;
	SIZE_T allocUserMemSize = 0;
	PVOID allocUserMemAddress = NULL;
	isWow64Process = PsGetProcessWow64Process(eprocess) != NULL ? TRUE : FALSE;
	allocUserMemSize = shellCodeSize + PAGE_SIZE;

	// 主线程劫持
	PUCHAR tmpShellcodeBuf = ExAllocatePool(PagedPool, shellCodeSize);
	if (tmpShellcodeBuf != NULL)
	{
		// 1.三环参数,执行的shellcode倒一手到0环
		RtlZeroMemory(tmpShellcodeBuf, shellCodeSize);
		RtlCopyMemory(tmpShellcodeBuf, shellCodePtr, shellCodeSize);
		KAPC_STATE kApcState = { 0 };
		KeStackAttachProcess(eprocess, &kApcState);

		// 2.申请用户空间,填入构造好的shellcode
		stat = CT_ZwAllocateVirtualMemory(NtCurrentProcess(), &allocUserMemAddress, &allocUserMemSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (NT_SUCCESS(stat) && allocUserMemAddress)
		{
			// 3.填入用户要执行的shellcode
			PUCHAR shellCodeBeginPtr = (PUCHAR)((ULONG64)allocUserMemAddress + PAGE_SIZE);
			RtlZeroMemory(allocUserMemAddress, allocUserMemSize);
			RtlCopyMemory(shellCodeBeginPtr, tmpShellcodeBuf, shellCodeSize);

			// 4.挂起线程,制作劫持入口shellcode,填入
			if (NT_SUCCESS(CT_PsSuspendThread(ethread, NULL)))
			{
				ULONG codeExedFlagOffset = isWow64Process ? sizeof(NormalShellCodeX86) : sizeof(NormalShellCodeX64);
				if (isWow64Process)
				{
					// 5.获取并修改wowcontext.eip(+0xbc为原有回三环eip),锁下页
					SIZE_T retProc = NULL;
					PUCHAR teb64 = (PUCHAR)PsGetThreadTeb(ethread);
					MmCopyVirtualMemory(eprocess, (PULONG64)(teb64 + 0x1488), eprocess, (PULONG64)(teb64 + 0x1488), 8, UserMode, &retProc);
					PUCHAR WowContext = (PUCHAR) * (PULONG64)(teb64 + 0x1488);

					*(PULONG32)&NormalShellCodeX86[2] = (ULONG32)shellCodeBeginPtr;
					*(PULONG32)&NormalShellCodeX86[15] = (ULONG32)allocUserMemAddress + codeExedFlagOffset;
					
					ULONG32 originEip  = *(PULONG32)(WowContext + 0xbc);
					ULONG offset = originEip - ((ULONG32)allocUserMemAddress + sizeof(NormalShellCodeX86));
					*(PULONG32)&NormalShellCodeX86[27] = offset;

					memcpy(allocUserMemAddress, NormalShellCodeX86, sizeof(NormalShellCodeX86));
					*(PULONG)(WowContext + 0xbc) = allocUserMemAddress;
				}
				else
				{
					// 5.获取并修改trapframe.rip
					ULONG64 initStackAddr = *(PULONG64)((PUCHAR)ethread + 0x28);
					PKTRAP_FRAME trapFrame = (PKTRAP_FRAME)(initStackAddr - sizeof(KTRAP_FRAME));

					*(PULONG64)&NormalShellCodeX64[25] = (ULONG64)shellCodeBeginPtr;
					*(PULONG64)&NormalShellCodeX64[73] = (ULONG64)allocUserMemAddress + codeExedFlagOffset;
					*(PULONG64)&NormalShellCodeX64[95] = trapFrame->Rip;

					memcpy(allocUserMemAddress, NormalShellCodeX64, sizeof(NormalShellCodeX64));
					trapFrame->Rip = allocUserMemAddress;
				}

				// 6.延迟线程任务释放用户空间
				// 需注意: 只有非设备驱动对象才能使用ExInitializeWorkItem,在其他情况下应该使用IoAllocateWorkItem
				PFreeMemoryInfo item = (PFreeMemoryInfo)ExAllocatePool(NonPagedPool, sizeof(FreeMemoryInfo));
				if (item)
				{
					item->Pid = pid;
					item->IsExecutedAddr = (ULONG64)allocUserMemAddress + codeExedFlagOffset;
					item->FreeSize = allocUserMemSize;
					ExInitializeWorkItem(&item->WorkItem, ExFreeMemoryWorkItem, item);
					ExQueueWorkItem(&item->WorkItem, DelayedWorkQueue);
				}
				CT_PsResumeThread(ethread, NULL);
			}
		}
		KeUnstackDetachProcess(&kApcState);
		ExFreePool(tmpShellcodeBuf);
	}

end:
	if (eprocess)
	{
		ObDereferenceObject(eprocess);
	}
	if (ethread)
	{
		ObDereferenceObject(ethread);
	}
	return stat;
}