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

// 内存是否是安全的,通过是否有物理内存判断(未完成)
BOOLEAN MmIsAddressSafe(PVOID startAddress);

// 得到内核PE节的开始位置和结束位置
NTSTATUS RtlFindImageSection(IN PVOID imageBase, IN CHAR* sectionName, OUT PVOID* sectionStart, OUT PVOID* sectionEnd);

// 字符出现最后下标
int RtlStringLastIndexOf(PUNICODE_STRING fullPath, WCHAR ch);

// 删除字符串
VOID RtlDelSubStr(PWCHAR str, PWCHAR subStr);

// 分割字符串
VOID RtlSplitString(IN PUNICODE_STRING fullPath, OUT PWCHAR filePath, OUT PWCHAR fileName);

// 得到系统版本信息
BOOLEAN RtlGetVersionInfo(OUT RTL_OSVERSIONINFOEXW* info);
OS_VERSION RtlGetOsVersion();

// 过签名验证
ULONG RtlByPassCallBackVerify(PVOID pDrv);
VOID RtlResetCallBackVerify(PVOID ldr, ULONG oldFlags);

// 内核Sleep
NTSTATUS KeSleep(ULONG64 timeOut);

// 通过pid判断是否是32位进程
BOOLEAN PsIsWow64Process(HANDLE processId);

// 关写保护
ULONG64 wpoff();

// 开写保护
VOID wpon(ULONG64 mcr0);

// mdl映射地址
PVOID MdlMapMemory(OUT PMDL* mdl, IN PVOID tagAddress, IN ULONG mapSize, IN MODE preMode);

// mdl取消映射
VOID MdlUnMapMemory(IN PMDL mdl, IN PVOID mapBase);

// 通过驱动名得到驱动对象
NTSTATUS GetDriverObjectByName(IN PWCH driverName, OUT PDRIVER_OBJECT* driver);

// 得到进程的主线程
NTSTATUS GetMainThreadByEprocess(IN PEPROCESS eprocess, OUT PETHREAD* ethread);

// -------  接口设计  -------
// 
// MmCopyVirtualMemory接口封装一层,动态获取函数地址(过iat hook,"瓦洛兰特内存读写")
NTSTATUS NTAPI CT_MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);

// ZwProtectVirtualMemory接口封装一层,修改内存属性(有try语句异常处理)
NTSTATUS NTAPI CT_ZwProtectVirtualMemory(IN PVOID Address, IN SIZE_T SpaceSize, IN ULONG NewProtect, OUT ULONG* OldProtect);

// ObRegisterCallbacks接口封装一层,过签名验证(直接修改硬编码)
NTSTATUS NTAPI CT_ObRegisterCallbacks(IN POB_CALLBACK_REGISTRATION CallbackRegistration, OUT PVOID* RegistrationHandle);

// ZwGetNextThread接口封装一层,获取当前线程的下一线程,如果当前线程为NULL则获取主线程
NTSTATUS NTAPI CT_ZwGetNextThread(IN HANDLE ProcessHandle, IN HANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN ULONG HandleAttributes, IN ULONG Flags, OUT PHANDLE NewThreadHandle);

// PsSuspendThread接口封装一层,线程挂起
NTSTATUS NTAPI CT_PsSuspendThread(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);

// PsResumeThread接口封装一层,线程恢复
NTSTATUS NTAPI CT_PsResumeThread(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);

// 申请用户空间内存
NTSTATUS NTAPI CT_ZwAllocateVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T AllocSize, ULONG AllcType, ULONG Protect);

// 释放用户空间内存
NTSTATUS NTAPI CT_ZwFreeVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress);

