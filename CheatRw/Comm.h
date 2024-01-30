#pragma once
#include "Unrevealed.h"

//================ 通信约定 ====================
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
    COMM_NUMBER CommFlag;   // 通信标志
    COMM_NUMBER CommFnID;   // 功能ID
    ULONG64 Request;        // 请求数据包(响应也在)
    ULONG Length;           // 长度
    ULONG ResponseCode;     // 结果 0正常 其他看异常
}PACKET, * PPACKET;

typedef struct _R3ModuleInfo
{
    _In_ HANDLE pid;
    _In_ char* ModuleName;
    _Out_ ULONG64 ModuleSize;
    _Out_ ULONG64 ModuleBase;
}R3ModuleInfo, * PR3ModuleInfo;

//================ 函数声明 ======================

NTSTATUS CommInitialize();		    // 通信初始化-注册回调
VOID CommUninitialize();            // 
