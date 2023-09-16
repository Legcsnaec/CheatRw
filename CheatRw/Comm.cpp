#include "Comm.h"
#include "CheatTools.h"
#include "SearchCode.h"

// Win10 NtConvertBetweenAuxiliaryCounterAndPerformanceCounter  -->  KdEnumerateDebuggingDevices����ʵ��HalPrivateDispatchTable�е�ֵ��
// 3������NtConvertBetweenAuxiliaryCounterAndPerformanceCounter���ߵ�0�������KdEnumerateDebuggingDevices��ȫ�ֱ�����
#define FConvertBetweenAuxiliary "488B05****75*488B05****E8"
#define KdEnumerateDebuggingDevicesOffset 3
PULONG64 KdEnumerateDebuggingAddr = 0;
ULONG64 OldKdEnumerateDebugging = 0;

// Win7	ExpDisQueryAttributeInformationȫ�ֱ���(3�����ߵ�0���ĺ���,Ҳ������Ҫע��ĺ���)
// ExRegisterAttributeInformationCallback ע�ắ����������ж�ExpDisQueryAttributeInformation ȫ�ֱ���
PULONG64 ExpDisQueryAttributeInfo = 0;	// AttributeInformationCallback[2]
RWCALL_BACK_FUNC OldCallBack = { 0 };


// ע��
NTSTATUS RegisterCallBack()
{
	NTSTATUS status = STATUS_SUCCESS;
	if (CheatTools::OS_WIN7 == CheatTools::RtlGetOsVersion() || CheatTools::OS_WIN7SP1 == CheatTools::RtlGetOsVersion())
	{
		status = RegisterCallBackWin7();
	}
	else
	{
		status = RegisterCallBackWin10();
	}
	return status;
}

// Win7 ע��
NTSTATUS RegisterCallBackWin7()
{
	NTSTATUS status = STATUS_SUCCESS;
	_ExRegisterAttributeInformationCallback registerCallBack = NULL;
	UNICODE_STRING funName = RTL_CONSTANT_STRING(L"ExRegisterAttributeInformationCallback");
	registerCallBack = (_ExRegisterAttributeInformationCallback)MmGetSystemRoutineAddress(&funName);

	if (registerCallBack != NULL)
	{
		// +16ƫ����ExpDisQueryAttributeInformationȫ�ֱ���offset
		ULONG offset = *(PULONG)((ULONG64)registerCallBack + 16);
		ULONG64 nextEip = ((ULONG64)registerCallBack + 20);
		ULONG tagetLowerAddr = ((nextEip << 32 >> 32) + offset);
		ExpDisQueryAttributeInfo = (PULONG64)((nextEip >> 32 << 32) + tagetLowerAddr);

		// ����ԭ��+����ظ�ֵ
		OldCallBack.ExpDisQueryAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[0];
		OldCallBack.ExpDisSetAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[1];
		ExpDisQueryAttributeInfo[0] = 0;
		ExpDisQueryAttributeInfo[1] = 0;

		// ����ExRegisterAttributeInformationCallback
		RWCALL_BACK_FUNC rwCallBack = { 0 };
		rwCallBack.ExpDisQueryAttributeInformation = RtlQueryAttributeInformation;
		rwCallBack.ExpDisSetAttributeInformation = RtlSetAttributeInformation;
		status = registerCallBack(&rwCallBack);

		KdPrint(("[info]: Comm_RegisterCallBackWin7 -- Win7ע��ص��ɹ�\r\n"));
	}
	else
	{
		KdPrint(("[Error]: Comm_RegisterCallBackWin7 -- δ�õ�ExRegisterAttributeInformationCallback��ַ\r\n"));
		status = STATUS_UNSUCCESSFUL;
	}
	return status;
}

// Win10 ע��
NTSTATUS RegisterCallBackWin10()
{
	NTSTATUS stat = STATUS_SUCCESS;
	ULONG64 globalVarAddr = NULL;
	globalVarAddr = SearchCode("ntoskrnl.exe", "PAGE", FConvertBetweenAuxiliary, KdEnumerateDebuggingDevicesOffset);
	if (globalVarAddr == NULL)
	{
		KdPrint(("[info]: Comm_RegisterCallBackWin10 -- ����������ʧ��\r\n"));
		return STATUS_UNSUCCESSFUL;
	}

	//��ʽ����һ�е�ַ(�Ͱ�λ) + ƫ�� = ������ַ(�Ͱ�λ)
	ULONG64 nextEip = globalVarAddr + 4;
	ULONG tagetLowerAddr = ((nextEip << 32 >> 32) + *(PULONG)globalVarAddr);
	KdEnumerateDebuggingAddr = (PULONG64)((nextEip >> 32 << 32) + tagetLowerAddr);
	if (!MmIsAddressValid(KdEnumerateDebuggingAddr))
	{
		KdPrint(("[info]: Comm_RegisterCallBackWin10 -- KdEnumerateDebugging��ַ����\r\n"));
		return STATUS_UNSUCCESSFUL;
	}
	OldKdEnumerateDebugging = KdEnumerateDebuggingAddr[0];
	KdEnumerateDebuggingAddr[0] = (ULONG64)NewKdEnumerateDebugging;
	KdPrint(("[info]: Comm_RegisterCallBackWin10 -- ע��ص��ɹ�\r\n"));
	return stat;
}

// ж��
VOID UnRegCallBack()
{
	if (CheatTools::OS_WIN7 == CheatTools::RtlGetOsVersion() || CheatTools::OS_WIN7SP1 == CheatTools::RtlGetOsVersion())
	{
		UnRegCallBackWin7();
	}
	else
	{
		UnRegCallBackWin10();
	}
}
VOID UnRegCallBackWin7()
{
	if (ExpDisQueryAttributeInfo != 0)
	{
		ExpDisQueryAttributeInfo[0] = (ULONG64)OldCallBack.ExpDisQueryAttributeInformation;
		ExpDisQueryAttributeInfo[1] = (ULONG64)OldCallBack.ExpDisSetAttributeInformation;
	}
	ExpDisQueryAttributeInfo = 0;
	KdPrint(("[info]: Comm_UnRegCallBackWin7 -- ж��Win7ͨ�Żص�\r\n"));
}
VOID UnRegCallBackWin10()
{
	if (OldKdEnumerateDebugging != 0)
	{
		KdEnumerateDebuggingAddr[0] = OldKdEnumerateDebugging;
	}
	OldKdEnumerateDebugging = 0;
	KdPrint(("[info]: Comm_UnRegCallBackWin10 -- ж��Win10ͨ�Żص�\r\n"));
}

// --------------------------------------------------------------------------------------

// Win7�ص�����
NTSTATUS RtlQueryAttributeInformation(HANDLE handle, PVOID arg)
{
	PPACKET packet = (PPACKET)arg;
	if (packet->commFlag == IsR3ToR0)
	{
		// �����Զ����ͨ��
		KdPrint(("[info]: Comm_RtlQueryAttributeInformation -- ExpDisQueryAttributeInformation�ص�����\r\n"));
		DispatchCallEntry(packet);
	}
	else
	{
		if (OldCallBack.ExpDisQueryAttributeInformation != 0)
		{
			return (OldCallBack.ExpDisQueryAttributeInformation)(handle, arg);
		}
	}
	return STATUS_SUCCESS;
}
NTSTATUS RtlSetAttributeInformation(HANDLE handle, PVOID arg)
{
	PPACKET packet = (PPACKET)arg;
	if (packet->commFlag == IsR3ToR0)
	{
		// �����Զ����ͨ��
		KdPrint(("[info]: Comm_RtlSetAttributeInformation -- ExpDisSetAttributeInformation�ص�����\r\n"));
		DispatchCallEntry(packet);
	}
	else
	{
		if (OldCallBack.ExpDisSetAttributeInformation != 0)
		{
			return (OldCallBack.ExpDisSetAttributeInformation)(handle, arg);
		}
	}
	return STATUS_SUCCESS;
}

// Win10�ص�����
NTSTATUS NewKdEnumerateDebugging(PVOID arg1, PVOID arg2, PVOID arg3)
{
	NTSTATUS status = STATUS_SUCCESS;
	PPACKET packet = (PPACKET)arg1;
	if (packet->commFlag = IsR3ToR0)
	{
		KdPrint(("[info]: Comm_NewKdEnumerateDebugging -- Win10���ܵ�����ͨ��\r\n"));
		DispatchCallEntry(packet);
	}
	else
	{
		if (OldKdEnumerateDebugging != 0)
		{
			return ((ULONG64(__fastcall*)(PVOID, PVOID, PVOID))OldKdEnumerateDebugging)(arg1, arg2, arg3);
		}
	}
	return status;
}

// ���ܵ��Ⱥ���
VOID DispatchCallEntry(PPACKET packet)
{
	ULONG FnId = packet->commFnID;
	switch (FnId)
	{
	case DriverRead:
	{
		KdPrint(("[info]: Comm_DispatchCallEntry -- ������\r\n"));
		packet->result = 200;
		break;
	}
	case DriverWrite:
	{
		KdPrint(("[info]: Comm_DispatchCallEntry -- д����\r\n"));
		packet->result = 300;
		break;
	}
	default:
	{
		KdPrint(("[info]: Comm_DispatchCallEntry -- ��Ч��ͨ��ID\r\n"));
		break;
	}
	}
}