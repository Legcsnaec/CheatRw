#include "Comm.h"
#include "CheatTools.h"
#include "SearchCode.h"

// Win10 NtConvertBetweenAuxiliaryCounterAndPerformanceCounter  -->  KdEnumerateDebuggingDevices（其实是HalPrivateDispatchTable中的值）
// 3环调用NtConvertBetweenAuxiliaryCounterAndPerformanceCounter，走到0环会调用KdEnumerateDebuggingDevices（全局变量）

#define FConvertBetweenAuxiliaryCode "488B05****75*488B05****E8"
#define KdEnumerateDebuggingDevicesOffset 3
#define ExpDisQueryAttributeInformationOffset 16

// Win7通信注册所需结构
typedef NTSTATUS(*_AttributeInformationCallback)(HANDLE, PVOID);
typedef struct _RWCALL_BACK_FUN
{
	_AttributeInformationCallback ExpDisQueryAttributeInformation;
	_AttributeInformationCallback ExpDisSetAttributeInformation;
}RWCALL_BACK_FUNC, * PRWCALL_BACK_FUN;
typedef NTSTATUS(*_ExRegisterAttributeInformationCallback)(PRWCALL_BACK_FUN);

// Win10通信注册所需变量
PULONG64 KdEnumerateDebuggingAddr = 0;
ULONG64 OldKdEnumerateDebugging = 0;

// Win7通信注册所需变量(3环会走到0环的函数,也是我们要注册的函数)
// ExRegisterAttributeInformationCallback注册函数，里面会判断ExpDisQueryAttributeInformation全局变量是否为0
PULONG64 ExpDisQueryAttributeInfo = 0;
RWCALL_BACK_FUNC OldCallBack = { 0 };

DispatchCallEntryPfn DispatchCallBack = NULL;

// 函数声明
NTSTATUS RegisterCallBackWin7();
NTSTATUS RegisterCallBackWin10();
VOID UnRegCallBackWin7();
VOID UnRegCallBackWin10();
NTSTATUS RtlQueryAttributeInformation(HANDLE, PVOID);                   // Win7通信回调函数
NTSTATUS RtlSetAttributeInformation(HANDLE, PVOID);                     // Win7通信回调函数
NTSTATUS NewKdEnumerateDebugging(PVOID arg1, PVOID arg2, PVOID arg3);   // Win10通信回调函数

// 通信初始化
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

// Win7 注册
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

		// 保存原有+清空重赋值
		OldCallBack.ExpDisQueryAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[0];
		OldCallBack.ExpDisSetAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[1];
		ExpDisQueryAttributeInfo[0] = 0;
		ExpDisQueryAttributeInfo[1] = 0;

		// 调用ExRegisterAttributeInformationCallback,其实就是修改全局变量ExpDisQueryAttributeInfo
		RWCALL_BACK_FUNC rwCallBack = { 0 };
		rwCallBack.ExpDisQueryAttributeInformation = RtlQueryAttributeInformation;
		rwCallBack.ExpDisSetAttributeInformation = RtlSetAttributeInformation;
		status = registerCallBack(&rwCallBack);

		KdPrint(("[info]: Comm_RegisterCallBackWin7 -- Win7注册回调成功\r\n"));
	}
	else
	{
		KdPrint(("[Error]: Comm_RegisterCallBackWin7 -- 未得到ExRegisterAttributeInformationCallback地址\r\n"));
		status = STATUS_UNSUCCESSFUL;
	}
	return status;
}

// Win10 注册
NTSTATUS RegisterCallBackWin10()
{
	NTSTATUS stat = STATUS_SUCCESS;
	ULONG64 globalVarAddr = 0;
	globalVarAddr = SearchCode("ntoskrnl.exe", "PAGE", FConvertBetweenAuxiliaryCode, KdEnumerateDebuggingDevicesOffset);
	if (globalVarAddr == 0)
	{
		KdPrint(("[info]: Comm_RegisterCallBackWin10 -- 搜索特征码失败\r\n"));
		return STATUS_UNSUCCESSFUL;
	}

	LARGE_INTEGER targetEip = { 0 };
	targetEip.QuadPart = globalVarAddr + 4;
	targetEip.LowPart = targetEip.LowPart + *(PULONG)globalVarAddr;
	KdEnumerateDebuggingAddr = (PULONG64)targetEip.QuadPart;
	if (!MmIsAddressValid(KdEnumerateDebuggingAddr))
	{
		KdPrint(("[info]: Comm_RegisterCallBackWin10 -- KdEnumerateDebugging地址错误\r\n"));
		return STATUS_UNSUCCESSFUL;
	}
	OldKdEnumerateDebugging = KdEnumerateDebuggingAddr[0];
	KdEnumerateDebuggingAddr[0] = (ULONG64)NewKdEnumerateDebugging;
	KdPrint(("[info]: Comm_RegisterCallBackWin10 -- 注册回调成功\r\n"));
	return stat;
}

// 卸载
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
	KdPrint(("[info]: Comm_UnRegCallBackWin7 -- 卸载Win7通信回调\r\n"));
}
VOID UnRegCallBackWin10()
{
	if (OldKdEnumerateDebugging != 0)
	{
		KdEnumerateDebuggingAddr[0] = OldKdEnumerateDebugging;
	}
	OldKdEnumerateDebugging = 0;
	KdPrint(("[info]: Comm_UnRegCallBackWin10 -- 卸载Win10通信回调\r\n"));
}

// --------------------------------------------------------------------------------------

// Win7回调函数
NTSTATUS RtlQueryAttributeInformation(HANDLE handle, PVOID arg)
{
	PPACKET packet = (PPACKET)arg;
	if (packet->CommFlag == IsR3ToR0)
	{
		KdPrint(("[info]: Comm_RtlQueryAttributeInformation -- ExpDisQueryAttributeInformation回调触发\r\n"));
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
	KdPrint(("[info]: Comm_RtlSetAttributeInformation -- ExpDisSetAttributeInformation回调触发\r\n"));
	return (OldCallBack.ExpDisSetAttributeInformation)(handle, arg);
}

// Win10回调函数
NTSTATUS NewKdEnumerateDebugging(PVOID arg1, PVOID arg2, PVOID arg3)
{
	PPACKET packet = (PPACKET)arg1;
	if (packet->CommFlag = IsR3ToR0)
	{
		KdPrint(("[info]: Comm_NewKdEnumerateDebugging -- Win10接受到三环通信\r\n"));
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