#include "kmclass.h"
#include "../Unrevealed.h"

// �ص���������
typedef VOID(*MY_KEYBOARDCALLBACK) (PDEVICE_OBJECT  DeviceObject, PKEYBOARD_INPUT_DATA  InputDataStart, PKEYBOARD_INPUT_DATA  InputDataEnd, PULONG  InputDataConsumed);
typedef VOID(*MY_MOUSECALLBACK) (PDEVICE_OBJECT  DeviceObject, PMOUSE_INPUT_DATA  InputDataStart, PMOUSE_INPUT_DATA  InputDataEnd, PULONG  InputDataConsumed);

typedef struct _DEVICE_EXTENSION {

    PDEVICE_OBJECT       kbdDeviceObject;        //�������豸����
    PDEVICE_OBJECT       mouDeviceObject;        //������豸����
    MY_KEYBOARDCALLBACK  My_KbdCallback;         //KeyboardClassServiceCallback���� 
    MY_MOUSECALLBACK     My_MouCallback;         //MouseClassServiceCallback����
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

// ִ�м��̻ص�
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
        // ����Ϊdpc,��ʱ���ǰ������dpcִ����
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        if (MmIsAddressValid(g_KoMCallBack.KdbDeviceObject->DeviceExtension))
        {
            // DeviceExtension��С��Ϊ0x90
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

// ִ�����ص�
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
        // ����Ϊdpc,��ʱ���ǰ������dpcִ����
        KeRaiseIrql(DISPATCH_LEVEL, &irql);
        if (MmIsAddressValid(g_KoMCallBack.MouDeviceObject->DeviceExtension))
        {
            // DeviceExtension��С��Ϊ0x90
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

// ��ʼ��������
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

    // ͨ��USB���豸��ȡ��������
    RtlInitUnicodeString(&ObjectName, kmhidName);
    status = ObReferenceObjectByName(&ObjectName, OBJ_CASE_INSENSITIVE, NULL, FILE_READ_ACCESS, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&kmDriverObject);


    if (!NT_SUCCESS(status))
    {
        // ͨ��i8042prt��ȡ��������
        RtlInitUnicodeString(&ObjectName, L"\\Driver\\i8042prt");
        status = ObReferenceObjectByName(&ObjectName, OBJ_CASE_INSENSITIVE, NULL, FILE_READ_ACCESS, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&kmDriverObject);
        if (!NT_SUCCESS(status))
        {
            KdPrint(("Couldn't Get the i8042prt Driver Object\n"));
            return status;
        }
    }

    // ͨ��kmclass��ȡ�����������������
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
                        // ����ȡ�����豸���󱣴浽�Զ�����չ�豸�ṹ
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

// ��ѯ�õ��ص�����,���
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
        //�����������ҵ��Ķ˿��������豸��չ��ÿһ��ָ��  
        for (i = 0; i < 4096; i++, DeviceExt++)
        {
            PVOID tmp;
            if (!MmIsAddressValid(DeviceExt))
            {
                break;
            }
            //�ҵ������д�����ȫ�ֱ����У��������Ƿ��Ѿ������  
            //����Ѿ�����˾Ͳ��ü������ˣ�����ֱ���˳�  
            if (g_KoMCallBack.MouDeviceObject && g_KoMCallBack.MouseClassServiceCallback)
            {
                status = STATUS_SUCCESS;
                break;
            }
            //�ڶ˿��������豸��չ��ҵ����������豸��������������豸��������  
            tmp = *(PVOID*)DeviceExt;
            if (tmp == pTargetDeviceObject)
            {
                g_KoMCallBack.MouDeviceObject = pTargetDeviceObject;
                continue;
            }

            //������豸��չ���ҵ�һ����ַλ��KbdClass��������У��Ϳ�����Ϊ�����������Ҫ�ҵĻص�����  
            if ((tmp > KbdDriverStart) && (tmp < (UCHAR*)KbdDriverStart + KbdDriverSize) &&
                (MmIsAddressValid(tmp)))
            {
                //������ص�������¼����  
                g_KoMCallBack.MouseClassServiceCallback = (MY_MOUSECALLBACK)tmp;
                //g_KoMCallBack.MouSerCallAddr = (PVOID *)DeviceExt;
                status = STATUS_SUCCESS;
            }
        }
        if (status == STATUS_SUCCESS)
        {
            break;
        }
        //������һ���豸����������  
        pTargetDeviceObject = pTargetDeviceObject->NextDevice;
    }
    return status;
}

// ��ѯ�õ����ص�
NTSTATUS  SearchMouServiceCallBack()
{
    //�����õ���һ��ȫ�ֱ�������Щ����������ǹ���˼���  
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING uniNtNameString;
    PDEVICE_OBJECT pTargetDeviceObject = NULL;
    PDRIVER_OBJECT KbdDriverObject = NULL;
    PDRIVER_OBJECT KbdhidDriverObject = NULL;
    PDRIVER_OBJECT Kbd8042DriverObject = NULL;
    PDRIVER_OBJECT UsingDriverObject = NULL;
    PDEVICE_OBJECT UsingDeviceObject = NULL;

    PVOID UsingDeviceExt = NULL;

    //����Ĵ���������USB���̶˿���������������  
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

    //��PS/2���̵���������  
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

    //��������豸��û���ҵ�  
    if (!Kbd8042DriverObject && !KbdhidDriverObject)
    {
        return STATUS_SUCCESS;
    }

    //���USB���̺�PS/2����ͬʱ���ڣ�ʹ��USB���
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

    //����KbdDriverObject�µ��豸���� 
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

// ��ѯ�õ��ص�����,����ڶ˿��豸��չ���ҵ�һ����ַλ��kbd�����������У��Ϳ�����Ϊ������Ҫ�ҵĻص�����
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

        //�����������ҵ��Ķ˿��������豸��չ��ÿһ��ָ��  
        for (i = 0; i < 4096; i++, DeviceExt++)
        {
            PVOID tmp;
            if (!MmIsAddressValid(DeviceExt))
            {
                break;
            }

            //�ҵ������д�����ȫ�ֱ����У��������Ƿ��Ѿ������  
            //����Ѿ�����˾Ͳ��ü������ˣ�����ֱ���˳�  
            if (g_KoMCallBack.KdbDeviceObject && g_KoMCallBack.KeyboardClassServiceCallback)
            {
                status = STATUS_SUCCESS;
                break;
            }

            //�ڶ˿��������豸��չ��ҵ����������豸��������������豸��������  
            tmp = *(PVOID*)DeviceExt;
            if (tmp == pTargetDeviceObject)
            {
                g_KoMCallBack.KdbDeviceObject = pTargetDeviceObject;
                continue;
            }

            //������豸��չ���ҵ�һ����ַλ��KbdClass��������У��Ϳ�����Ϊ�����������Ҫ�ҵĻص�����  
            if ((tmp > KbdDriverStart) && (tmp < (UCHAR*)KbdDriverStart + KbdDriverSize) && (MmIsAddressValid(tmp)))
            {
                //������ص�������¼����  
                g_KoMCallBack.KeyboardClassServiceCallback = (MY_KEYBOARDCALLBACK)tmp;
            }
        }
        if (status == STATUS_SUCCESS)
        {
            break;
        }

        //������һ���豸����������  
        pTargetDeviceObject = pTargetDeviceObject->NextDevice;
    }
    return status;
}

// ������ѯkbdclass��������������Ļص�
NTSTATUS  SearchKdbServiceCallBack()
{
    //�����õ���һ��ȫ�ֱ�������Щ����������ǹ���˼���  
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING uniNtNameString;
    PDEVICE_OBJECT pTargetDeviceObject = NULL;
    PDRIVER_OBJECT KbdDriverObject = NULL;
    PDRIVER_OBJECT KbdhidDriverObject = NULL;
    PDRIVER_OBJECT Kbd8042DriverObject = NULL;
    PDRIVER_OBJECT UsingDriverObject = NULL;
    PDEVICE_OBJECT UsingDeviceObject = NULL;

    PVOID UsingDeviceExt = NULL;

    //����Ĵ���������USB���̶˿���������������  
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

    //��PS/2���̵���������  
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

    //��δ��뿼����һ�����������õ���������USB���̺�PS/2����ͬʱ���ڣ���PS/2����
    //��������豸��û���ҵ�  
    if (!Kbd8042DriverObject && !KbdhidDriverObject)
    {
        return STATUS_SUCCESS;
    }

    //�ҵ����ʵ��������󣬲�����USB����PS/2������һ��Ҫ�ҵ�һ��   
    UsingDriverObject = Kbd8042DriverObject ? Kbd8042DriverObject : KbdhidDriverObject;

    //��������������
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

    //����KbdDriverObject�µ��豸���� 
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

    //����ɹ��ҵ��ˣ��Ͱ���������滻�������Լ��Ļص�����  
    if (g_KoMCallBack.KdbDeviceObject && g_KoMCallBack.KeyboardClassServiceCallback)
    {
        KdPrint(("Find keyboradClassServiceCallback\n"));
    }
    return status;
}