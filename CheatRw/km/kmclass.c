#include "kmclass.h"
#include "../Unrevealed.h"

// 回调函数声明
typedef VOID(*MY_KEYBOARDCALLBACK) (PDEVICE_OBJECT  DeviceObject, PKEYBOARD_INPUT_DATA  InputDataStart, PKEYBOARD_INPUT_DATA  InputDataEnd, PULONG  InputDataConsumed);
typedef VOID(*MY_MOUSECALLBACK) (PDEVICE_OBJECT  DeviceObject, PMOUSE_INPUT_DATA  InputDataStart, PMOUSE_INPUT_DATA  InputDataEnd, PULONG  InputDataConsumed);

typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT       kbdDeviceObject;        //键盘类设备对象
    PDEVICE_OBJECT       mouDeviceObject;        //鼠标类设备对象
    MY_KEYBOARDCALLBACK  My_KbdCallback;         //KeyboardClassServiceCallback函数 
    MY_MOUSECALLBACK     My_MouCallback;         //MouseClassServiceCallback函数
    // ... 
}DEVICE_EXTENSION, * PDEVICE_EXTENSION;

struct
{
    PDEVICE_OBJECT KdbDeviceObject;
    MY_KEYBOARDCALLBACK KeyboardClassServiceCallback;

    PDEVICE_OBJECT MouDeviceObject;
    MY_MOUSECALLBACK MouseClassServiceCallback;
}g_KoMCallBack;

// ---------------------------------------------------------

#ifdef __cplusplus
extern "C"
#endif

// 执行键盘回调
VOID ExecuteKeyboardCallback(PKEYBOARD_INPUT_DATA inputData)
{
    KIRQL irql;
    ULONG InputDataConsumed = 1;
    KEYBOARD_INPUT_DATA InData[2] = { 0 };
    PKEYBOARD_INPUT_DATA KbdInputDataStart, KbdInputDataEnd;
    memset(&InData[0], 0, sizeof(KEYBOARD_INPUT_DATA) * 2);
    memcpy(&InData[0], inputData, sizeof(KEYBOARD_INPUT_DATA));

    KbdInputDataStart = &InData[0];
    KbdInputDataEnd = &InData[1];

    if (g_KoMCallBack.KeyboardClassServiceCallback)
    {
        KeEnterCriticalRegion();
        // 提升为dpc,这时会把前面插入的dpc执行完
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        if (MmIsAddressValid(g_KoMCallBack.KdbDeviceObject->DeviceExtension))
        {
            // DeviceExtension大小定为0x90
            PULONG64 spin = (PULONG64)((PUCHAR)g_KoMCallBack.KdbDeviceObject->DeviceExtension + 0x90);
            if (MmIsAddressValid(spin))
            {
                g_KoMCallBack.KeyboardClassServiceCallback(g_KoMCallBack.KdbDeviceObject, KbdInputDataStart, KbdInputDataEnd, &InputDataConsumed);
            }
        }
        KeLowerIrql(irql);
        KeLeaveCriticalRegion();
    }
}

// 执行鼠标回调
VOID ExecuteMouseCallback(PMOUSE_INPUT_DATA inputData)
{
    KIRQL irql;
    ULONG InputDataConsumed = 1;
    MOUSE_INPUT_DATA InData[2] = { 0 };
    PMOUSE_INPUT_DATA KMoInputDataStart, KMoInputDataEnd;
    memset(&InData[0], 0, sizeof(MOUSE_INPUT_DATA) * 2);
    memcpy(&InData[0], inputData, sizeof(MOUSE_INPUT_DATA));

    KMoInputDataStart = &InData[0];
    KMoInputDataEnd = &InData[1];

    if (g_KoMCallBack.MouseClassServiceCallback)
    {
        KeEnterCriticalRegion();
        // 提升为dpc,这时会把前面插入的dpc执行完
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        if (MmIsAddressValid(g_KoMCallBack.MouDeviceObject->DeviceExtension))
        {
            // DeviceExtension大小定为0x90
            PULONG64 spin = (PULONG64)((PUCHAR)g_KoMCallBack.MouDeviceObject->DeviceExtension + 0x90);
            if (MmIsAddressValid(spin))
            {
                g_KoMCallBack.MouseClassServiceCallback(g_KoMCallBack.MouDeviceObject, KMoInputDataStart, KMoInputDataEnd, &InputDataConsumed);
            }
        }
        KeLowerIrql(irql);
        KeLeaveCriticalRegion();
    }
}

// 初始化键鼠功能
NTSTATUS  SearchMouServiceCallBack();
NTSTATUS  SearchKdbServiceCallBack();
NTSTATUS InitKmClass()
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    // status = GetKmclassInfo(DeviceObject, KEYBOARD_DEVICE);
    status = SearchKdbServiceCallBack();
    if (!NT_SUCCESS(status))
    {
        KdPrint(("KEYBOARD_DEVICE ERROR, error = 0x%08lx\n", status));
        return status;
    }

    // status = GetKmclassInfo(DeviceObject, MOUSE_DEVICE);
    status = SearchMouServiceCallBack();
    if (!NT_SUCCESS(status))
    {
        KdPrint(("MOUSE_DEVICE ERROR, error = 0x%08lx\n", status));
        return status;
    }
}

NTSTATUS GetKmclassInfo(PDEVICE_OBJECT DeviceObject, USHORT Index)
{
    NTSTATUS           status;
    UNICODE_STRING     ObjectName;
    PCWSTR             kmhidName, kmclassName, kmName;
    PVOID              kmDriverStart;
    ULONG              kmDriverSize;
    PVOID* TargetDeviceObject;
    PVOID* TargetclassCallback;
    PDEVICE_EXTENSION  deviceExtension;
    PDRIVER_OBJECT     kmDriverObject = NULL;
    PDRIVER_OBJECT     kmclassDriverObject = NULL;

    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    switch (Index)
    {
    case KEYBOARD_DEVICE:
        kmName = L"kbd";
        kmhidName = L"\\Driver\\kbdhid";
        kmclassName = L"\\Driver\\kbdclass";
        TargetDeviceObject = (PVOID*)&(deviceExtension->kbdDeviceObject);
        TargetclassCallback = (PVOID*)&(deviceExtension->My_KbdCallback);
        break;
    case MOUSE_DEVICE:
        kmName = L"mou";
        kmhidName = L"\\Driver\\mouhid";
        kmclassName = L"\\Driver\\mouclass";
        TargetDeviceObject = (PVOID*)&(deviceExtension->mouDeviceObject);
        TargetclassCallback = (PVOID*)&(deviceExtension->My_MouCallback);
        break;
    default:
        return STATUS_INVALID_PARAMETER;
    }

    // 通过USB类设备获取驱动对象
    RtlInitUnicodeString(&ObjectName, kmhidName);
    status = ObReferenceObjectByName(&ObjectName, OBJ_CASE_INSENSITIVE, NULL, FILE_READ_ACCESS, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&kmDriverObject);


    if (!NT_SUCCESS(status))
    {
        // 通过i8042prt获取驱动对象
        RtlInitUnicodeString(&ObjectName, L"\\Driver\\i8042prt");
        status = ObReferenceObjectByName(&ObjectName, OBJ_CASE_INSENSITIVE, NULL, FILE_READ_ACCESS, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&kmDriverObject);
        if (!NT_SUCCESS(status))
        {
            KdPrint(("Couldn't Get the i8042prt Driver Object\n"));
            return status;
        }
    }

    // 通过kmclass获取键盘鼠标类驱动对象
    RtlInitUnicodeString(&ObjectName, kmclassName);
    status = ObReferenceObjectByName(&ObjectName, OBJ_CASE_INSENSITIVE, NULL, FILE_READ_ACCESS, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&kmclassDriverObject);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Couldn't Get the kmclass Driver Object\n"));
        return status;
    }
    else
    {
        kmDriverStart = kmclassDriverObject->DriverStart;
        kmDriverSize = kmclassDriverObject->DriverSize;
    }

    ULONG             DeviceExtensionSize;
    PULONG            kmDeviceExtension;
    PDEVICE_OBJECT    kmTempDeviceObject;
    PDEVICE_OBJECT    kmclassDeviceObject;
    PDEVICE_OBJECT    kmDeviceObject = kmDriverObject->DeviceObject;
    while (kmDeviceObject)
    {
        kmTempDeviceObject = kmDeviceObject;
        while (kmTempDeviceObject)
        {
            kmDeviceExtension = (PULONG)kmTempDeviceObject->DeviceExtension;
            kmclassDeviceObject = kmclassDriverObject->DeviceObject;
            DeviceExtensionSize = ((ULONG)kmTempDeviceObject->DeviceObjectExtension - (ULONG)kmTempDeviceObject->DeviceExtension) / 4;
            while (kmclassDeviceObject)
            {
                for (ULONG i = 0; i < DeviceExtensionSize; i++)
                {
                    if (kmDeviceExtension[i] == (ULONG)kmclassDeviceObject &&
                        kmDeviceExtension[i + 1] > (ULONG)kmDriverStart &&
                        kmDeviceExtension[i + 1] < (ULONG)kmDriverStart + kmDriverSize)
                    {
                        // 将获取到的设备对象保存到自定义扩展设备结构
                        *TargetDeviceObject = (PVOID)kmDeviceExtension[i];
                        *TargetclassCallback = (PVOID)kmDeviceExtension[i + 1];
                        KdPrint(("%SDeviceObject == 0x%x\n", kmName, kmDeviceExtension[i]));
                        KdPrint(("%SClassServiceCallback == 0x%x\n", kmName, kmDeviceExtension[i + 1]));
                        return STATUS_SUCCESS;
                    }
                }
                kmclassDeviceObject = kmclassDeviceObject->NextDevice;
            }
            kmTempDeviceObject = kmTempDeviceObject->AttachedDevice;
        }
        kmDeviceObject = kmDeviceObject->NextDevice;
    }
    return STATUS_UNSUCCESSFUL;
}

// 查询得到回调函数,鼠标
NTSTATUS SearchServiceFromMouExt(PDRIVER_OBJECT MouDriverObject, PDEVICE_OBJECT pPortDev)
{
    PDEVICE_OBJECT pTargetDeviceObject = NULL;
    UCHAR* DeviceExt;
    int i = 0;
    NTSTATUS status;
    PVOID KbdDriverStart;
    ULONG KbdDriverSize = 0;
    PDEVICE_OBJECT  pTmpDev;
    UNICODE_STRING  kbdDriName;

    KbdDriverStart = MouDriverObject->DriverStart;
    KbdDriverSize = MouDriverObject->DriverSize;

    status = STATUS_UNSUCCESSFUL;

    RtlInitUnicodeString(&kbdDriName, L"\\Driver\\mouclass");
    pTmpDev = pPortDev;
    while (pTmpDev->AttachedDevice != NULL)
    {
        KdPrint(("Att:  0x%x", pTmpDev->AttachedDevice));
        KdPrint(("Dri Name : %wZ", &pTmpDev->AttachedDevice->DriverObject->DriverName));
        if (RtlCompareUnicodeString(&pTmpDev->AttachedDevice->DriverObject->DriverName,
            &kbdDriName, TRUE) == 0)
        {
            KdPrint(("Find Object Device: "));
            break;
        }
        pTmpDev = pTmpDev->AttachedDevice;
    }
    if (pTmpDev->AttachedDevice == NULL)
    {
        return status;
    }
    pTargetDeviceObject = MouDriverObject->DeviceObject;
    while (pTargetDeviceObject)
    {
        if (pTmpDev->AttachedDevice != pTargetDeviceObject)
        {
            pTargetDeviceObject = pTargetDeviceObject->NextDevice;
            continue;
        }
        DeviceExt = (UCHAR*)pTmpDev->DeviceExtension;
        g_KoMCallBack.MouDeviceObject = NULL;
        //遍历我们先找到的端口驱动的设备扩展的每一个指针  
        for (i = 0; i < 4096; i++, DeviceExt++)
        {
            PVOID tmp;
            if (!MmIsAddressValid(DeviceExt))
            {
                break;
            }
            //找到后会填写到这个全局变量中，这里检查是否已经填好了  
            //如果已经填好了就不用继续找了，可以直接退出  
            if (g_KoMCallBack.MouDeviceObject && g_KoMCallBack.MouseClassServiceCallback)
            {
                status = STATUS_SUCCESS;
                break;
            }
            //在端口驱动的设备扩展里，找到了类驱动设备对象，填好类驱动设备对象后继续  
            tmp = *(PVOID*)DeviceExt;
            if (tmp == pTargetDeviceObject)
            {
                g_KoMCallBack.MouDeviceObject = pTargetDeviceObject;
                continue;
            }

            //如果在设备扩展中找到一个地址位于KbdClass这个驱动中，就可以认为，这就是我们要找的回调函数  
            if ((tmp > KbdDriverStart) && (tmp < (UCHAR*)KbdDriverStart + KbdDriverSize) &&
                (MmIsAddressValid(tmp)))
            {
                //将这个回调函数记录下来  
                g_KoMCallBack.MouseClassServiceCallback = (MY_MOUSECALLBACK)tmp;
                //g_KoMCallBack.MouSerCallAddr = (PVOID *)DeviceExt;
                status = STATUS_SUCCESS;
            }
        }
        if (status == STATUS_SUCCESS)
        {
            break;
        }
        //换成下一个设备，继续遍历  
        pTargetDeviceObject = pTargetDeviceObject->NextDevice;
    }
    return status;
}

// 查询得到鼠标回调
NTSTATUS  SearchMouServiceCallBack()
{
    //定义用到的一组全局变量，这些变量大多数是顾名思义的  
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING uniNtNameString;
    PDEVICE_OBJECT pTargetDeviceObject = NULL;
    PDRIVER_OBJECT KbdDriverObject = NULL;
    PDRIVER_OBJECT KbdhidDriverObject = NULL;
    PDRIVER_OBJECT Kbd8042DriverObject = NULL;
    PDRIVER_OBJECT UsingDriverObject = NULL;
    PDEVICE_OBJECT UsingDeviceObject = NULL;

    PVOID UsingDeviceExt = NULL;

    //这里的代码用来打开USB键盘端口驱动的驱动对象  
    RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\mouhid");
    status = ObReferenceObjectByName(&uniNtNameString, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&KbdhidDriverObject);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Couldn't get the USB Mouse Object\n"));
    }
    else
    {
        ObDereferenceObject(KbdhidDriverObject);
        KdPrint(("get the USB Mouse Object\n"));
    }

    //打开PS/2键盘的驱动对象  
    RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\i8042prt");
    status = ObReferenceObjectByName(&uniNtNameString, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&Kbd8042DriverObject);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Couldn't get the PS/2 Mouse Object %08x\n", status));
    }
    else
    {
        ObDereferenceObject(Kbd8042DriverObject);
        KdPrint(("get the PS/2 Mouse Object\n"));
    }

    //如果两个设备都没有找到  
    if (!Kbd8042DriverObject && !KbdhidDriverObject)
    {
        return STATUS_SUCCESS;
    }

    //如果USB键盘和PS/2键盘同时存在，使用USB鼠标
    if (KbdhidDriverObject)
    {
        UsingDriverObject = KbdhidDriverObject;
    }
    else
    {
        UsingDriverObject = Kbd8042DriverObject;
    }

    RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\mouclass");
    status = ObReferenceObjectByName(&uniNtNameString, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&KbdDriverObject);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("MyAttach: Coundn't get the Mouse driver Object\n"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        ObDereferenceObject(KbdDriverObject);
    }

    //遍历KbdDriverObject下的设备对象 
    UsingDeviceObject = UsingDriverObject->DeviceObject;
    while (UsingDeviceObject)
    {
        status = SearchServiceFromMouExt(KbdDriverObject, UsingDeviceObject);
        if (status == STATUS_SUCCESS)
        {
            break;
        }
        UsingDeviceObject = UsingDeviceObject->NextDevice;
    }
    if (g_KoMCallBack.MouDeviceObject && g_KoMCallBack.MouseClassServiceCallback)
    {
        KdPrint(("Find MouseClassServiceCallback\n"));
    }
    return status;
}

// 查询得到回调函数,如果在端口设备扩展中找到一个地址位于kbd键盘类驱动中，就可以认为是我们要找的回调函数
NTSTATUS SearchServiceFromKdbExt(PDRIVER_OBJECT KbdDriverObject, PDEVICE_OBJECT pPortDev)
{
    PDEVICE_OBJECT pTargetDeviceObject = NULL;
    UCHAR* DeviceExt;

    int i = 0;
    NTSTATUS status;
    PVOID KbdDriverStart;
    ULONG KbdDriverSize = 0;
    PDEVICE_OBJECT  pTmpDev;
    UNICODE_STRING  kbdDriName;

    KbdDriverStart = KbdDriverObject->DriverStart;
    KbdDriverSize = KbdDriverObject->DriverSize;

    status = STATUS_UNSUCCESSFUL;
    RtlInitUnicodeString(&kbdDriName, L"\\Driver\\kbdclass");
    pTmpDev = pPortDev;
    while (pTmpDev->AttachedDevice != NULL)
    {
        KdPrint(("Att:  0x%llx", (ULONG64)pTmpDev->AttachedDevice));
        KdPrint(("Dri Name : %wZ", &pTmpDev->AttachedDevice->DriverObject->DriverName));
        if (RtlCompareUnicodeString(&pTmpDev->AttachedDevice->DriverObject->DriverName, &kbdDriName, TRUE) == 0)
        {
            break;
        }
        pTmpDev = pTmpDev->AttachedDevice;
    }
    if (pTmpDev->AttachedDevice == NULL)
    {
        return status;
    }

    pTargetDeviceObject = KbdDriverObject->DeviceObject;
    while (pTargetDeviceObject)
    {
        if (pTmpDev->AttachedDevice != pTargetDeviceObject)
        {
            pTargetDeviceObject = pTargetDeviceObject->NextDevice;
            continue;
        }
        DeviceExt = (UCHAR*)pTmpDev->DeviceExtension;
        g_KoMCallBack.KdbDeviceObject = NULL;

        //遍历我们先找到的端口驱动的设备扩展的每一个指针  
        for (i = 0; i < 4096; i++, DeviceExt++)
        {
            PVOID tmp;
            if (!MmIsAddressValid(DeviceExt))
            {
                break;
            }

            //找到后会填写到这个全局变量中，这里检查是否已经填好了  
            //如果已经填好了就不用继续找了，可以直接退出  
            if (g_KoMCallBack.KdbDeviceObject && g_KoMCallBack.KeyboardClassServiceCallback)
            {
                status = STATUS_SUCCESS;
                break;
            }

            //在端口驱动的设备扩展里，找到了类驱动设备对象，填好类驱动设备对象后继续  
            tmp = *(PVOID*)DeviceExt;
            if (tmp == pTargetDeviceObject)
            {
                g_KoMCallBack.KdbDeviceObject = pTargetDeviceObject;
                continue;
            }

            //如果在设备扩展中找到一个地址位于KbdClass这个驱动中，就可以认为，这就是我们要找的回调函数  
            if ((tmp > KbdDriverStart) && (tmp < (UCHAR*)KbdDriverStart + KbdDriverSize) && (MmIsAddressValid(tmp)))
            {
                //将这个回调函数记录下来  
                g_KoMCallBack.KeyboardClassServiceCallback = (MY_KEYBOARDCALLBACK)tmp;
            }
        }
        if (status == STATUS_SUCCESS)
        {
            break;
        }

        //换成下一个设备，继续遍历  
        pTargetDeviceObject = pTargetDeviceObject->NextDevice;
    }
    return status;
}

// 搜索查询kbdclass键盘类驱动对象的回调
NTSTATUS  SearchKdbServiceCallBack()
{
    //定义用到的一组全局变量，这些变量大多数是顾名思义的  
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING uniNtNameString;
    PDEVICE_OBJECT pTargetDeviceObject = NULL;
    PDRIVER_OBJECT KbdDriverObject = NULL;
    PDRIVER_OBJECT KbdhidDriverObject = NULL;
    PDRIVER_OBJECT Kbd8042DriverObject = NULL;
    PDRIVER_OBJECT UsingDriverObject = NULL;
    PDEVICE_OBJECT UsingDeviceObject = NULL;

    PVOID UsingDeviceExt = NULL;

    //这里的代码用来打开USB键盘端口驱动的驱动对象  
    RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\kbdhid");
    status = ObReferenceObjectByName(&uniNtNameString, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&KbdhidDriverObject);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Couldn't get the USB driver Object\n"));
    }
    else
    {
        ObDereferenceObject(KbdhidDriverObject);
        KdPrint(("get the USB driver Object\n"));
    }

    //打开PS/2键盘的驱动对象  
    RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\i8042prt");
    status = ObReferenceObjectByName(&uniNtNameString, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&Kbd8042DriverObject);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Couldn't get the PS/2 driver Object %08x\n", status));
    }
    else
    {
        ObDereferenceObject(Kbd8042DriverObject);
        KdPrint(("get the PS/2 driver Object\n"));
    }

    //这段代码考虑有一个键盘起作用的情况。如果USB键盘和PS/2键盘同时存在，用PS/2键盘
    //如果两个设备都没有找到  
    if (!Kbd8042DriverObject && !KbdhidDriverObject)
    {
        return STATUS_SUCCESS;
    }

    //找到合适的驱动对象，不管是USB还是PS/2，反正一定要找到一个   
    UsingDriverObject = Kbd8042DriverObject ? Kbd8042DriverObject : KbdhidDriverObject;

    //键盘类驱动对象
    RtlInitUnicodeString(&uniNtNameString, L"\\Driver\\kbdclass");
    status = ObReferenceObjectByName(&uniNtNameString, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&KbdDriverObject);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("MyAttach: Coundn't get the kbd driver Object\n"));
        return STATUS_UNSUCCESSFUL;
    }
    else
    {
        ObDereferenceObject(KbdDriverObject);
    }

    //遍历KbdDriverObject下的设备对象 
    UsingDeviceObject = UsingDriverObject->DeviceObject;
    while (UsingDeviceObject)
    {
        status = SearchServiceFromKdbExt(KbdDriverObject, UsingDeviceObject);
        if (status == STATUS_SUCCESS)
        {
            break;
        }
        UsingDeviceObject = UsingDeviceObject->NextDevice;
    }

    //如果成功找到了，就把这个函数替换成我们自己的回调函数  
    if (g_KoMCallBack.KdbDeviceObject && g_KoMCallBack.KeyboardClassServiceCallback)
    {
        KdPrint(("Find keyboradClassServiceCallback\n"));
    }
    return status;
}