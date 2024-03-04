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

EXTERN_C ULONG64 WINAPI CtGetModuleR3(IN DWORD_PTR pid, IN char* moduleName, OUT PULONG64 moduleSizeAddr);

EXTERN_C BOOLEAN WINAPI CtReadMemory(IN DWORD_PTR pid, IN ULONG64 baseAddress,OUT PVOID readBuffer,IN ULONG readSize);

EXTERN_C BOOLEAN WINAPI CtWriteMemory(IN DWORD_PTR pid, IN ULONG64 baseAddress, IN PVOID writeBuffer, IN ULONG writeSize);

EXTERN_C BOOLEAN WINAPI CtQueryMemory(IN DWORD_PTR pid, IN ULONG64 baseAddress, OUT PMMEMORY_BASIC_INFORMATION basicInfoAddr);

EXTERN_C BOOLEAN WINAPI CtProtectHandle(IN DWORD_PTR pid);

EXTERN_C BOOLEAN WINAPI CtRemoteCall(DWORD_PTR pid, PVOID shellcode, DWORD shellcodeSize);

// ---------------------------------- 键鼠相关导出 ----------------------------------

EXTERN_C BOOLEAN KeyDown(USHORT VirtualKey);
EXTERN_C BOOLEAN KeyUp(USHORT VirtualKey);
EXTERN_C BOOLEAN MouseLeftButtonDown();
EXTERN_C BOOLEAN MouseLeftButtonUp();
EXTERN_C BOOLEAN MouseRightButtonDown();
EXTERN_C BOOLEAN MouseRightButtonUp();
EXTERN_C BOOLEAN MouseMiddleButtonDown();
EXTERN_C BOOLEAN MouseMiddleButtonUp();
EXTERN_C BOOLEAN MouseMoveRELATIVE(LONG dx, LONG dy);
EXTERN_C BOOLEAN MouseMoveABSOLUTE(LONG dx, LONG dy);
