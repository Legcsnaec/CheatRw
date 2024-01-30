#pragma once
#include <Windows.h>
#include <stdio.h>

#define RW_STATUS_SUCCESS                   0x00000000L
#define RW_STATUS_UNSUCCESSFUL              0xC0000001L
#define RW_STATUS_COMMINITFAILURE           0xC0000002L
#define STATUS_ACCESS_VIOLATION             0xC0000005L
#define RW_STATUS_INVALID_CID               0xC000000BL
#define RW_STATUS_INVALID_PARAMETER_1       0xC00000EFL
#define RW_STATUS_INVALID_PARAMETER_2       0xC00000F0L
#define RW_STATUS_INVALID_PARAMETER_3       0xC00000F1L
#define RW_STATUS_INVALID_ADDRESS           0xC0000141L
#define RW_STATUS_INVALID_PARAMETER         STATUS_INVALID_PARAMETER

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
    CMD_RemoteCall,
    COMM_NUMBER_SIZE
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER CommFlag;   // 通信标志
    COMM_NUMBER CommFnID;   // 功能ID
    PVOID Request;          // 请求数据包(响应也在)
    ULONG Length;           // 请求包长度
    ULONG ResponseCode;     // 结果 200正常 500异常
}PACKET, * PPACKET;

// 要另外类型的通信只管在后面加结构体了
typedef struct _R3ModuleInfo
{
    _In_ HANDLE Pid;
    _In_ char* ModuleName;
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


//================ 未文档化结构 ======================
//Win7 驱动通信
typedef struct _IO_STATUS_BLOCK {
    union {
        PVOID Status;
        PVOID Pointer;
    };
    ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;

typedef ULONG(*NtQueryInformationFilePfn)(
    HANDLE FileHandle,
    PIO_STATUS_BLOCK IoStatusBlock,
    PVOID  FileInformation,
    ULONG Length,
    ULONG FileInformationClass
    );

//Win10驱动通信
typedef ULONG64(_fastcall* NtConvertBetweenPfn)(char, PVOID, PVOID, PVOID);

//================ 函数声明 ======================
NTSTATUS SendCommPacket(ULONG cmd, PVOID inData, SIZE_T inSize);