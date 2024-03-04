#pragma once
#include <ntifs.h>

#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS   0
#endif

#ifndef KEYBOARD_DEVICE
#define KEYBOARD_DEVICE   0
#endif

#ifndef MOUSE_DEVICE
#define MOUSE_DEVICE      1
#endif

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

// --------------------------------------
// 对外的接口
// 
// 执行键盘回调
VOID ExecuteKeyboardCallback(PKEYBOARD_INPUT_DATA inputData);

// 执行鼠标回调
VOID ExecuteMouseCallback(PMOUSE_INPUT_DATA inputData);

// 初始化键鼠功能
NTSTATUS InitKmClass();