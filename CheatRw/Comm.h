#pragma once
#include "Unrevealed.h"

//================ ͨ��Լ�� ====================
typedef enum _COMM_NUMBER
{
    IsR3ToR0 = 0x1000,
    CMD_DriverRead,
    CMD_DriverWrite,
    CMD_GetModuleR3,
    CMD_GetMMType,
    CMD_InstallProtect,
    CMD_UninstallProtect,
    CMD_RemoteCall
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER CommFlag;   // ͨ�ű�־
    COMM_NUMBER CommFnID;   // ����ID
    ULONG64 Request;        // �������ݰ�(��ӦҲ��)
    ULONG Length;           // ����
    ULONG ResponseCode;     // ��� 0���� �������쳣
}PACKET, * PPACKET;

typedef struct _R3ModuleInfo
{
    _In_ HANDLE pid;
    _In_ char* ModuleName;
    _Out_ ULONG64 ModuleSize;
    _Out_ ULONG64 ModuleBase;
}R3ModuleInfo, * PR3ModuleInfo;

//================ �������� ======================

NTSTATUS CommInitialize();		    // ͨ�ų�ʼ��-ע��ص�
VOID CommUninitialize();            // 
