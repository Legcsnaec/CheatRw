#include "Comm.h"
#include "CheatTools.h"
#include "SearchCode.h"

// Win10 NtConvertBetweenAuxiliaryCounterAndPerformanceCounter  -->  KdEnumerateDebuggingDevices����ʵ��HalPrivateDispatchTable�е�ֵ��
// 3������NtConvertBetweenAuxiliaryCounterAndPerformanceCounter���ߵ�0�������KdEnumerateDebuggingDevices��ȫ�ֱ�����

#define FConvertBetweenAuxiliaryCode "488B05****75*488B05****E8"
#define KdEnumerateDebuggingDevicesOffset 3
#define ExpDisQueryAttributeInformationOffset 16

// Win7ͨ��ע������ṹ
typedef NTSTATUS(*_AttributeInformationCallback)(HANDLE, PVOID);
typedef struct _RWCALL_BACK_FUN
{
	_AttributeInformationCallback ExpDisQueryAttributeInformation;
	_AttributeInformationCallback ExpDisSetAttributeInformation;
}RWCALL_BACK_FUNC, * PRWCALL_BACK_FUN;
typedef NTSTATUS(*_ExRegisterAttributeInformationCallback)(PRWCALL_BACK_FUN);

// Win10ͨ��ע���������
PULONG64 KdEnumerateDebuggingAddr = 0;
ULONG64 OldKdEnumerateDebugging = 0;

// Win7ͨ��ע���������(3�����ߵ�0���ĺ���,Ҳ������Ҫע��ĺ���)
// ExRegisterAttributeInformationCallbackע�ắ����������ж�ExpDisQueryAttributeInformationȫ�ֱ����Ƿ�Ϊ0
PULONG64 ExpDisQueryAttributeInfo = 0;
RWCALL_BACK_FUNC OldCallBack = { 0 };

DispatchCallEntryPfn DispatchCallBack = NULL;

// ��������
NTSTATUS RegisterCallBackWin7();
NTSTATUS RegisterCallBackWin10();
VOID UnRegCallBackWin7();
VOID UnRegCallBackWin10();
NTSTATUS RtlQueryAttributeInformation(HANDLE, PVOID);                   // Win7ͨ�Żص�����
NTSTATUS RtlSetAttributeInformation(HANDLE, PVOID);                     // Win7ͨ�Żص�����
NTSTATUS NewKdEnumerateDebugging(PVOID arg1, PVOID arg2, PVOID arg3);   // Win10ͨ�Żص�����

// ͨ�ų�ʼ��
NTSTATUS CommInitialize(DispatchCallEntryPfn callBack)
{
	NTSTATUS status = STATUS_SUCCESS;
	if (DispatchCallBack == NULL)
	{
		DispatchCallBack = callBack;
	}
	if (OS_WIN7 == RtlGetOsVersion() || OS_WIN7SP1 == RtlGetOsVersion())
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
		ULONG offset = *(PULONG)((ULONG64)registerCallBack + ExpDisQueryAttributeInformationOffset);
		LARGE_INTEGER targetEip = { 0 };
		targetEip.QuadPart = ((ULONG64)registerCallBack + ExpDisQueryAttributeInformationOffset + 4);
		targetEip.LowPart = targetEip.LowPart + offset;
		ExpDisQueryAttributeInfo = (PULONG64)targetEip.QuadPart;

		// ����ԭ��+����ظ�ֵ
		OldCallBack.ExpDisQueryAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[0];
		OldCallBack.ExpDisSetAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[1];
		ExpDisQueryAttributeInfo[0] = 0;
		ExpDisQueryAttributeInfo[1] = 0;

		// ����ExRegisterAttributeInformationCallback,��ʵ�����޸�ȫ�ֱ���ExpDisQueryAttributeInfo
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
	ULONG64 globalVarAddr = 0;
	globalVarAddr = SearchCode("ntoskrnl.exe", "PAGE", FConvertBetweenAuxiliaryCode, KdEnumerateDebuggingDevicesOffset);
	if (globalVarAddr == 0)
	{
		KdPrint(("[info]: Comm_RegisterCallBackWin10 -- ����������ʧ��\r\n"));
		return STATUS_UNSUCCESSFUL;
	}

	LARGE_INTEGER targetEip = { 0 };
	targetEip.QuadPart = globalVarAddr + 4;
	targetEip.LowPart = targetEip.LowPart + *(PULONG)globalVarAddr;
	KdEnumerateDebuggingAddr = (PULONG64)targetEip.QuadPart;
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
VOID CommUninitialize()
{
	if (OS_WIN7 == RtlGetOsVersion() || OS_WIN7SP1 == RtlGetOsVersion())
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
	if (packet->CommFlag == IsR3ToR0)
	{
		KdPrint(("[info]: Comm_RtlQueryAttributeInformation -- ExpDisQueryAttributeInformation�ص�����\r\n"));
		if (DispatchCallBack)
		{
			DispatchCallBack(packet);
		}
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
	KdPrint(("[info]: Comm_RtlSetAttributeInformation -- ExpDisSetAttributeInformation�ص�����\r\n"));
	return (OldCallBack.ExpDisSetAttributeInformation)(handle, arg);
}

// Win10�ص�����
NTSTATUS NewKdEnumerateDebugging(PVOID arg1, PVOID arg2, PVOID arg3)
{
	PPACKET packet = (PPACKET)arg1;
	if (packet->CommFlag = IsR3ToR0)
	{
		KdPrint(("[info]: Comm_NewKdEnumerateDebugging -- Win10���ܵ�����ͨ��\r\n"));
		if (DispatchCallBack)
		{
			DispatchCallBack(packet);
		}
	}
	else
	{
		if (OldKdEnumerateDebugging != 0)
		{
			return ((NTSTATUS(__fastcall*)(PVOID, PVOID, PVOID))OldKdEnumerateDebugging)(arg1, arg2, arg3);
		}
	}
	return STATUS_SUCCESS;
}