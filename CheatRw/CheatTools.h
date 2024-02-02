#pragma once
#include <ntifs.h>
#include <intrin.h>
#include <ntimage.h>
#include <ntddstor.h>
#include <fltKernel.h>
#include <minwindef.h>

// ϵͳ�汾
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

// 64\32λ�汾,���ݻ�ַ�����Ƶõ�������ַ,���������
PVOID MmGetSystemRoutineAddressEx(ULONG64 modBase, CHAR* searchFnName);

// �����û��ռ��ڴ�
PVOID MmAllocateUserVirtualMemory(HANDLE processHandle, SIZE_T allocSize, ULONG allocationType, ULONG protect);

// �ͷ��û��ռ��ڴ�
NTSTATUS MmFreeUserVirtualMemory(HANDLE processHandle, PVOID base);

// �ڴ��Ƿ��ǰ�ȫ��,ͨ���Ƿ��������ڴ��ж�(δ���)
BOOLEAN MmIsAddressSafe(PVOID startAddress);

// ����������(δ���)
PVOID RtlScanFeatureCode(PVOID begin, PVOID end, CHAR* featureCode);

// �õ��ں�PE�ڵĿ�ʼλ�úͽ���λ��
NTSTATUS RtlFindImageSection(IN PVOID imageBase, IN CHAR* sectionName, OUT PVOID* sectionStart, OUT PVOID* sectionEnd);

// ɾ���ַ���
VOID RtlDelSubStr(PWCHAR str, PWCHAR subStr);

// �ָ��ַ���
VOID RtlSplitString(IN PUNICODE_STRING fullPath, OUT PWCHAR filePath, OUT PWCHAR fileName);

// �õ�ϵͳ�汾��Ϣ
BOOLEAN RtlGetVersionInfo(OUT RTL_OSVERSIONINFOEXW* info);
OS_VERSION RtlGetOsVersion();

// ��ǩ����֤
ULONG RtlByPassCallBackVerify(PVOID pDrv);
VOID RtlResetCallBackVerify(PVOID ldr, ULONG oldFlags);

// �ں�Sleep
NTSTATUS KeSleep(ULONG64 timeOut);

// ͨ��pid�ж��Ƿ���32λ����
BOOLEAN PsIsWow64Process(HANDLE processId);

// ��д����
ULONG64 wpoff();

// ��д����
VOID wpon(ULONG64 mcr0);

// mdlӳ���ַ
PVOID MdlMapMemory(OUT PMDL* mdl, IN PVOID tagAddress, IN SIZE_T mapSize, IN MODE preMode);

// mdlȡ��ӳ��
VOID MdlUnMapMemory(IN PMDL mdl, IN PVOID mapBase);

// ͨ���������õ���������
NTSTATUS GetDriverObjectByName(IN PWCH driverName, OUT PDRIVER_OBJECT* driver);

// -------  �ӿ����  -------
// 
// MmCopyVirtualMemory�ӿڷ�װһ��,��̬��ȡ������ַ(��iat hook "�������ڴ��д")
NTSTATUS NTAPI CT_MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize);

// ZwProtectVirtualMemory�ӿڷ�װһ��,�޸��ڴ�����(��try����쳣����)
NTSTATUS CT_ZwProtectVirtualMemory(IN PVOID Address, IN SIZE_T SpaceSize, IN ULONG NewProtect, OUT ULONG* OldProtect);

// ObRegisterCallbacks�ӿڷ�װһ��,��ǩ����֤(ֱ���޸�Ӳ����)
NTSTATUS CT_ObRegisterCallbacks(IN POB_CALLBACK_REGISTRATION CallbackRegistration, OUT PVOID* RegistrationHandle);