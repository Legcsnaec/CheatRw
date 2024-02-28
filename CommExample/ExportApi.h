#pragma once
#include <Windows.h>

typedef struct _MMEMORY_BASIC_INFORMATION 
{
	ULONG64 BaseAddress;
	ULONG64 AllocationBase;
	ULONG64 AllocationProtect;
	ULONG64 RegionSize;
	ULONG64 State;
	ULONG64 Protect;
	ULONG64 Type;
} MMEMORY_BASIC_INFORMATION, * PMMEMORY_BASIC_INFORMATION;

EXTERN_C BOOLEAN WINAPI CtDriverLoad();

EXTERN_C VOID WINAPI CtDriverUnLoad();

EXTERN_C ULONG64 WINAPI CtGetModuleR3(DWORD pid, char* moduleName);

EXTERN_C BOOLEAN WINAPI CtReadMemory(DWORD pid, ULONG64 BaseAddress, PVOID Buffer, ULONG size);

EXTERN_C BOOLEAN WINAPI CtWriteMemory(DWORD pid, ULONG64 BaseAddress, PVOID Buffer, ULONG size);

EXTERN_C BOOLEAN WINAPI CtQueryMemory(DWORD pid, ULONG64 BaseAddress, PMMEMORY_BASIC_INFORMATION pinfo);

EXTERN_C BOOLEAN WINAPI CtProtectHandle(DWORD pid);

EXTERN_C BOOLEAN WINAPI CtRemoteCall(DWORD pid, PVOID shellcode, DWORD shellcodeSize);