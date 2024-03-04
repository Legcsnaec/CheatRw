#pragma once
#include <Windows.h>
#include <stdio.h>

#define RW_STATUS_COMMINITFAILURE           0xC0000111L
#define RW_STATUS_SUCCESS                   0x00000000L
#define RW_STATUS_UNSUCCESSFUL              0xC0000001L
#define RW_STATUS_NOT_IMPLEMENTED           0xC0000002L
#define RW_STATUS_ACCESS_VIOLATION          0xC0000005L
#define RW_STATUS_INVALID_CID               0xC000000BL
#define RW_STATUS_ACCESS_DENIED             0xC0000022L
#define RW_STATUS_INVALID_PARAMETER_1       0xC00000EFL
#define RW_STATUS_INVALID_PARAMETER_2       0xC00000F0L
#define RW_STATUS_INVALID_PARAMETER_3       0xC00000F1L
#define RW_STATUS_INVALID_ADDRESS           0xC0000141L
#define RW_STATUS_PARTIAL_COPY              0x8000000DL
#define RW_STATUS_INVALID_PARAMETER         STATUS_INVALID_PARAMETER

//================ 通信约定 ====================
typedef enum _COMM_NUMBER
{
    IsR3ToR0 = 0x1000,
    CMD_TEST_CONNECT,      // 用于测试驱动连通,三环在加载驱动前测试一下是否连通,防止重复加载
    CMD_READ_MEMORY,
    CMD_WRITE_MEMORY,
    CMD_GET_MODULER3,
    CMD_QUERY_MEMORY,
    CMD_PROTECT_HANDLE,
    CMD_REMOTE_CALL,
    CMD_KEYBOARD,
    CMD_MOUSE,
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

//自定义内存信息结构
typedef struct _MYMEMORY_BASIC_INFORMATION {
    ULONG64 BaseAddress;
    ULONG64 AllocationBase;
    ULONG64 AllocationProtect;
    ULONG64 RegionSize;
    ULONG64 State;
    ULONG64 Protect;
    ULONG64 Type;
} MYMEMORY_BASIC_INFORMATION, * PMYMEMORY_BASIC_INFORMATION;

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

// ================ 键鼠结构声明 ======================
// 键盘输入数据结构体
typedef struct _KEYBOARD_INPUT_DATA
{
    USHORT UnitId;
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    ULONG ExtraInformation;
} KEYBOARD_INPUT_DATA, * PKEYBOARD_INPUT_DATA;

// 键盘标记
#define KEY_MAKE  0
#define KEY_BREAK 1
#define KEY_E0    2
#define KEY_E1    4
#define KEY_TERMSRV_SET_LED 8
#define KEY_TERMSRV_SHADOW  0x10
#define KEY_TERMSRV_VKPACKET 0x20

#define KEY_DOWN                KEY_MAKE
#define KEY_UP                  KEY_BREAK
#define KEY_BLANK                -1

// 鼠标输入数据结构体
typedef struct _MOUSE_INPUT_DATA
{
    USHORT UnitId;
    USHORT Flags;
    union
    {
        ULONG Buttons;
        struct
        {
            USHORT  ButtonFlags;
            USHORT  ButtonData;
        };
    };
    ULONG RawButtons;
    LONG LastX;
    LONG LastY;
    ULONG ExtraInformation;
} MOUSE_INPUT_DATA, * PMOUSE_INPUT_DATA;

// 鼠标按钮标记
#define MOUSE_LEFT_BUTTON        0x0001
#define MOUSE_RIGHT_BUTTON       0x0002
#define MOUSE_LEFT_BUTTON_DOWN   0x0001     // Left Button changed to down.
#define MOUSE_LEFT_BUTTON_UP     0x0002     // Left Button changed to up.
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004     // Right Button changed to down.
#define MOUSE_RIGHT_BUTTON_UP    0x0008     // Right Button changed to up.
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010     // Middle Button changed to down.
#define MOUSE_MIDDLE_BUTTON_UP   0x0020     // Middle Button changed to up.

#define MOUSE_BUTTON_1_DOWN     MOUSE_LEFT_BUTTON_DOWN
#define MOUSE_BUTTON_1_UP       MOUSE_LEFT_BUTTON_UP
#define MOUSE_BUTTON_2_DOWN     MOUSE_RIGHT_BUTTON_DOWN
#define MOUSE_BUTTON_2_UP       MOUSE_RIGHT_BUTTON_UP
#define MOUSE_BUTTON_3_DOWN     MOUSE_MIDDLE_BUTTON_DOWN
#define MOUSE_BUTTON_3_UP       MOUSE_MIDDLE_BUTTON_UP

#define MOUSE_BUTTON_4_DOWN     0x0040
#define MOUSE_BUTTON_4_UP       0x0080
#define MOUSE_BUTTON_5_DOWN     0x0100
#define MOUSE_BUTTON_5_UP       0x0200

#define MOUSE_WHEEL             0x0400

// 鼠标指示标记
#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_VIRTUAL_DESKTOP    0x02       // the coordinates are mapped to the virtual desktop
#define MOUSE_ATTRIBUTES_CHANGED 0x04       // requery for mouse attributes

//================ 真实通信函数声明 ======================
NTSTATUS SendCommPacket(ULONG cmd, PVOID inData, SIZE_T inSize);