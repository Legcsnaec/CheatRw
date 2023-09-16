#pragma once
#include "Unrevealed.h"

//================ ͨ��Լ�� ====================
typedef enum _COMM_NUMBER
{
    IsR3ToR0 = 0x1000,
    DriverRead,
    DriverWrite,
    GetMMType,
    InstallProtect,
    UnstallProtect,
    ModifyHeaderFlag,
    RestoreHeaderFlag,
    InJectThread
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER commFlag;   // ͨ�ű�־
    COMM_NUMBER commFnID;   // ����ID
    ULONG64 content;        // ����
    ULONG length;           // ����
    ULONG result;           // ���
}PACKET, * PPACKET;


//================ δ�ĵ��� Win7ע��ص� ======================
typedef NTSTATUS(*_AttributeInformationCallback)(HANDLE, PVOID);

typedef struct _RWCALL_BACK_FUN
{
    _AttributeInformationCallback ExpDisQueryAttributeInformation;
    _AttributeInformationCallback ExpDisSetAttributeInformation;
}RWCALL_BACK_FUNC, * PRWCALL_BACK_FUN;

typedef NTSTATUS(*_ExRegisterAttributeInformationCallback)(PRWCALL_BACK_FUN);



//================ �������� ======================

NTSTATUS RegisterCallBack();		// ע��ص�
NTSTATUS RegisterCallBackWin7();	// Win7 �ص�ע��
NTSTATUS RegisterCallBackWin10();	// Win10 �ص�ע��
VOID UnRegCallBack();
VOID UnRegCallBackWin7();
VOID UnRegCallBackWin10();

NTSTATUS RtlQueryAttributeInformation(HANDLE, PVOID);   // Win7�ص�����
NTSTATUS RtlSetAttributeInformation(HANDLE, PVOID);     // Win7�ص�����

NTSTATUS NewKdEnumerateDebugging(PVOID arg1, PVOID arg2, PVOID arg3);   // Win10�ص�����

VOID DispatchCallEntry(PPACKET);    // ���ܵ��Ⱥ���
