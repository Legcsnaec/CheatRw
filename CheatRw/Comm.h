#pragma once
#include "Unrevealed.h"

//================ ͨ��Լ�� ====================
typedef enum _COMM_NUMBER
{
    IsR3ToR0 = 0x1000,
    CMD_TEST_CONNECT,
    CMD_READ_MEMORY,
    CMD_WRITE_MEMORY,
    CMD_GET_MODULER3,
    CMD_QUERY_MEMORY,
    CMD_PROTECT_HANDLE,
    CMD_REMOTE_CALL
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER CommFlag;   // ͨ�ű�־
    COMM_NUMBER CommFnID;   // ����ID
    PVOID Request;          // �������ݰ�(��ӦҲ��)
    ULONG Length;           // ����
    ULONG ResponseCode;     // ��� 0���� �������쳣
}PACKET, * PPACKET;

typedef struct _R3ModuleInfo
{
    _In_  HANDLE Pid;
    _In_  char* ModuleName;
    _Out_ ULONG64 ModuleSize;
    _Out_ ULONG64 ModuleBase;
}R3ModuleInfo, * PR3ModuleInfo;

typedef struct _ReadMemInfo
{
    _In_  HANDLE Pid;
    _In_  PVOID TagAddress;
    _Out_ PVOID ReadBuffer;
    _In_  SIZE_T ReadSize;
}ReadMemInfo, * PReadMemInfo;

typedef struct _WriteMemInfo
{
    _In_  ULONG64 Pid;
    _In_  ULONG64 TagAddress;
    _In_  ULONG64 WriteBuffer;
    _In_  ULONG64 WriteSize;
}WriteMemInfo, * PWriteMemInfo;

typedef struct _QueryMemInfo
{
    _In_  ULONG64 Pid;
    _In_  ULONG64 BaseAddress;
    _Out_ MYMEMORY_BASIC_INFORMATION MemBasicInfo;
}QueryMemInfo, * PQueryMemInfo;

typedef struct _ProtectHandleInfo
{
    _In_  ULONG64 Pid;
    _In_  BOOLEAN IsInstall;
}ProtectHandleInfo, * PProtectHandleInfo;

typedef struct _RemoteCallInfo
{
    _In_  ULONG64 Pid;
    _In_  ULONG64 ShellCodePtr;
    _In_  ULONG64 ShellCodeSize;
}RemoteCallInfo, * PRemoteCallInfo;

// ͨ����ǲ
typedef VOID(*DispatchCallEntryPfn)(PPACKET packet);

//================ �������� ======================

NTSTATUS CommInitialize(DispatchCallEntryPfn callBack);     // ͨ�ų�ʼ��-ע��ص�
VOID CommUninitialize(); 