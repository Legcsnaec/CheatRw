#pragma once
#include "Unrevealed.h"

//================ 通信约定 ====================
// 32、64位环境结构大小一直,好满足x86进程与驱动通信
typedef enum _COMM_NUMBER
{
    IsR3ToR0 = 0x1000,
    CMD_TEST_CONNECT,
    CMD_READ_MEMORY,
    CMD_WRITE_MEMORY,
    CMD_GET_MODULER3,
    CMD_QUERY_MEMORY,
    CMD_PROTECT_HANDLE,
    CMD_REMOTE_CALL,
    COMM_NUMBER_SIZE
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER CommFlag;   // 通信标志
    COMM_NUMBER CommFnID;   // 功能ID
    ULONG Length;           // 长度
    ULONG ResponseCode;     // 结果 0正常 其他看异常
    ULONG64 Request;        // 请求数据包(响应也在)
}PACKET, * PPACKET;

typedef struct _R3ModuleInfo
{
    _In_  ULONG64 Pid;
    _In_  ULONG64 ModuleName;
    _Out_ ULONG64 ModuleSize;
    _Out_ ULONG64 ModuleBase;
}R3ModuleInfo, * PR3ModuleInfo;

typedef struct _ReadMemInfo
{
    _In_  ULONG64 Pid;
    _In_  ULONG64 TagAddress;
    _Out_ ULONG64 ReadBuffer;
    _In_  ULONG64 ReadSize;
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

// 通信派遣
typedef VOID(*DispatchCallEntryPfn)(PPACKET packet);

//================ 函数声明 ======================

NTSTATUS CommInitialize(DispatchCallEntryPfn callBack);     // 通信初始化-注册回调
VOID CommUninitialize(); 