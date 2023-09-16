#pragma once
#include <Windows.h>
#include <stdio.h>

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
    InJectThread,
    COMM_NUMBER_SIZE
}COMM_NUMBER;

typedef struct _PACKET
{
    COMM_NUMBER commFlag;   // ͨ�ű�־
    COMM_NUMBER commFnID;   // ����ID
    PVOID data;             // ����
    ULONG length;           // ����
    ULONG result;           // ���
}PACKET, * PPACKET;

//================ δ�ĵ����ṹ ======================
//Win7 ����ͨ��
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

//Win10����ͨ��
typedef ULONG64(_fastcall* NtConvertBetweenProc)(char, PVOID, PVOID, PVOID);

//================ �������� ======================
ULONG CommWin7(ULONG cmd, PVOID inData, SIZE_T inSize);
ULONG CommWin10(ULONG cmd, PVOID inData, SIZE_T inSize);
