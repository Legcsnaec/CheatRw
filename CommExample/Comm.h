#pragma once
#include <Windows.h>
#include <stdio.h>

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
    InJectThread,
    COMM_NUMBER_SIZE
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER commFlag;   // 通信标志
    COMM_NUMBER commFnID;   // 功能ID
    PVOID data;             // 数据
    ULONG length;           // 长度
    ULONG result;           // 结果
}PACKET, * PPACKET;

//================ 未文档化结构 ======================
//Win7 驱动通信
typedef struct _IO_STATUS_BLOCK {
    union {
        PVOID Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef ULONG(*_NtQueryInformationFile)(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID  FileInformation,
    ULONG Length,
    ULONG FileInformationClass
    );

//Win10驱动通信
typedef ULONG64(_fastcall* NtConvertBetweenProc)(char, PVOID, PVOID, PVOID);

//================ 函数声明 ======================
ULONG CommWin7(ULONG cmd, PVOID inData, SIZE_T inSize);
ULONG CommWin10(ULONG cmd, PVOID inData, SIZE_T inSize);
