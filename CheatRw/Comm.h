#pragma once
#include "Unrevealed.h"

//================ 通信约定 ====================
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
    COMM_NUMBER commFlag;   // 通信标志
    COMM_NUMBER commFnID;   // 功能ID
    ULONG64 content;        // 数据
    ULONG length;           // 长度
    ULONG result;           // 结果
}PACKET, * PPACKET;


//================ 未文档化 Win7注册回调 ======================
typedef NTSTATUS(*_AttributeInformationCallback)(HANDLE, PVOID);

typedef struct _RWCALL_BACK_FUN
{
    _AttributeInformationCallback ExpDisQueryAttributeInformation;
    _AttributeInformationCallback ExpDisSetAttributeInformation;
}RWCALL_BACK_FUNC, * PRWCALL_BACK_FUN;

typedef NTSTATUS(*_ExRegisterAttributeInformationCallback)(PRWCALL_BACK_FUN);



//================ 函数声明 ======================

NTSTATUS RegisterCallBack();		// 注册回调
NTSTATUS RegisterCallBackWin7();	// Win7 回调注册
NTSTATUS RegisterCallBackWin10();	// Win10 回调注册
VOID UnRegCallBack();
VOID UnRegCallBackWin7();
VOID UnRegCallBackWin10();

NTSTATUS RtlQueryAttributeInformation(HANDLE, PVOID);   // Win7回调函数
NTSTATUS RtlSetAttributeInformation(HANDLE, PVOID);     // Win7回调函数

NTSTATUS NewKdEnumerateDebugging(PVOID arg1, PVOID arg2, PVOID arg3);   // Win10回调函数

VOID DispatchCallEntry(PPACKET);    // 功能调度函数
