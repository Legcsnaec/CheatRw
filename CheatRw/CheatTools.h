#pragma once
#include <ntifs.h>
#include <intrin.h>
#include <ntimage.h>
#include <ntddstor.h>
#include <fltKernel.h>
#include <minwindef.h>

typedef enum _OS_VERSION
{
	OS_UNKNOWN,
	OS_WINXP,
	OS_WINXPSP1,
	OS_WINXPSP2,
	OS_WINXPSP3,
	OS_WIN2003,
	OS_WIN2003SP1,
	OS_WIN2003SP2,
	OS_WINVISTA,
	OS_WINVISTASP1,
	OS_WINVISTASP2,
	OS_WIN7,		//7600
	OS_WIN7SP1,		//7601
	OS_WIN8,
	OS_WIN81,
	OS_WIN10_1507, //10240
	OS_WIN10_1511, //10586
	OS_WIN10_1607, //14393
	OS_WIN10_1703, //15063
	OS_WIN10_1709, //16299
	OS_WIN10_1803, //17134
	OS_WIN10_1809, //17763
	OS_WIN10_1903, //18362
	OS_WIN10_1909, //18363
	OS_WIN10_2004, //19041
	OS_WIN10_20H2, //19042
	OS_WIN10_21H1, //19043
	OS_WIN10_21H2, //19044
	OS_WIN10_22H2, //19044
	OS_WIN11_21H2, //22000
} OS_VERSION, * POS_VERSION;


PVOID MmGetSystemRoutineAddressEx(ULONG64 modBase, CHAR* searchFnName);
PVOID MmAllocateUserVirtualMemory(HANDLE processHandle, SIZE_T allocSize, ULONG allocationType, ULONG protect);
NTSTATUS MmFreeUserVirtualMemory(HANDLE processHandle, PVOID base);
BOOLEAN MmIsAddressSafe(PVOID startAddress);

PVOID RtlScanFeatureCode(PVOID begin, PVOID end, CHAR* featureCode);
NTSTATUS RtlProtectVirtualMemory(PVOID address, SIZE_T spaceSize, ULONG newProtect, ULONG* oldProtect);
NTSTATUS RtlFindImageSection(PVOID imageBase, CHAR* sectionName, OUT PVOID* sectionStart, OUT PVOID* sectionEnd);

OS_VERSION RtlGetOsVersion();
VOID RtlDelSubStr(PWCHAR str, PWCHAR subStr);
VOID RtlSplitString(PUNICODE_STRING fullPath, OUT PWCHAR filePath, OUT PWCHAR fileName);
BOOLEAN RtlGetVersionInfo(RTL_OSVERSIONINFOEXW* info);
ULONG RtlByPassCallBackVerify(PVOID pDrv);
VOID RtlResetCallBackVerify(PVOID ldr, ULONG oldFlags);

NTSTATUS KeSleep(ULONG64 timeOut);
BOOLEAN PsIsWow64Process(HANDLE processId);
