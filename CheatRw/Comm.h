#pragma once
#include "Unrevealed.h"

//================ 通信约定 ====================
typedef enum _COMM_NUMBER
{
    IsR3ToR0 = 0x1000,
    CMD_DriverRead,
    CMD_DriverWrite,
    CMD_GetModuleR3,
    CMD_QueryMemory,
    CMD_ProtectHandle,
    CMD_RemoteCall
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER CommFlag;   // 通信标志
    COMM_NUMBER CommFnID;   // 功能ID
    PVOID Request;          // 请求数据包(响应也在)
    ULONG Length;           // 长度
    ULONG ResponseCode;     // 结果 0正常 其他看异常
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

// 通信派遣
typedef VOID(*DispatchCallEntryPfn)(PPACKET packet);

//================ 函数声明 ======================

NTSTATUS CommInitialize(DispatchCallEntryPfn callBack);     // 通信初始化-注册回调
VOID CommUninitialize(); 