#pragma once
#include <ntifs.h>
#include <intrin.h>
#include <ntimage.h>
#include <ntddstor.h>
#include <fltKernel.h>
#include <minwindef.h>

// 系统版本
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

// 64\32位版本,根据基址和名称得到函数地址,导出表解析
PVOID MmGetSystemRoutineAddressEx(ULONG64 modBase, CHAR* searchFnName);

// 申请用户空间内存
PVOID MmAllocateUserVirtualMemory(HANDLE processHandle, SIZE_T allocSize, ULONG allocationType, ULONG protect);

// 释放用户空间内存
NTSTATUS MmFreeUserVirtualMemory(HANDLE processHandle, PVOID base);

// 内存是否是安全的,通过是否有物理内存判断(未完成)
BOOLEAN MmIsAddressSafe(PVOID startAddress);

// 特征码搜索(未完成)
PVOID RtlScanFeatureCode(PVOID begin, PVOID end, CHAR* featureCode);

// 修改内存属性
NTSTATUS RtlProtectVirtualMemory(PVOID address, SIZE_T spaceSize, ULONG newProtect, ULONG* oldProtect);

// 得到内核PE节的开始位置和结束位置
NTSTATUS RtlFindImageSection(PVOID imageBase, CHAR* sectionName, OUT PVOID* sectionStart, OUT PVOID* sectionEnd);

// 删除字符串
VOID RtlDelSubStr(PWCHAR str, PWCHAR subStr);

// 分割字符串
VOID RtlSplitString(PUNICODE_STRING fullPath, OUT PWCHAR filePath, OUT PWCHAR fileName);

// 得到系统版本信息
BOOLEAN RtlGetVersionInfo(RTL_OSVERSIONINFOEXW* info);
OS_VERSION RtlGetOsVersion();

// 过签名验证
ULONG RtlByPassCallBackVerify(PVOID pDrv);
VOID RtlResetCallBackVerify(PVOID ldr, ULONG oldFlags);

// 内核Sleep
NTSTATUS KeSleep(ULONG64 timeOut);

// 通过pid判断是否是32位进程
BOOLEAN PsIsWow64Process(HANDLE processId);
