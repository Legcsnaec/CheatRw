#include "Comm.h"
#include "CheatTools.h"

// ExpDisQueryAttributeInformation全局变量，ExRegisterAttributeInformationCallback里面会判断
PULONG64 ExpDisQueryAttributeInfo = 0;
RWCALL_BACK_FUNC OldCallBack = { 0 };

// 回调注册
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

// Win7 回调注册
NTSTATUS RegisterCallBackWin7()
{
	NTSTATUS status = STATUS_SUCCESS;
	_ExRegisterAttributeInformationCallback registerCallBack = NULL;
	UNICODE_STRING funName = RTL_CONSTANT_STRING(L"ExRegisterAttributeInformationCallback");
	registerCallBack = (_ExRegisterAttributeInformationCallback)MmGetSystemRoutineAddress(&funName);

	if (registerCallBack != NULL)
	{
		// +16偏移是ExpDisQueryAttributeInformation全局变量offset
		ULONG offset = *(PULONG)((ULONG64)registerCallBack + 16);
		ULONG64 nextEip = ((ULONG64)registerCallBack + 20);
		ULONG tagetLowerAddr = ((nextEip << 32 >> 32) + offset);
		ExpDisQueryAttributeInfo = (PULONG64)((nextEip >> 32 << 32) + tagetLowerAddr);

		// 保存原有+清空重赋值
		OldCallBack.ExpDisQueryAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[0];
		OldCallBack.ExpDisSetAttributeInformation = (_AttributeInformationCallback)ExpDisQueryAttributeInfo[1];
		ExpDisQueryAttributeInfo[0] = 0;
		ExpDisQueryAttributeInfo[1] = 0;

		// 调用ExRegisterAttributeInformationCallback
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

// Win10 回调注册
NTSTATUS RegisterCallBackWin10()
{
	return STATUS_SUCCESS;
}

// Win7回调函数
NTSTATUS RtlQueryAttributeInformation(HANDLE handle, PVOID arg)
{
	PPACKET packet = (PPACKET)arg;
	if (packet->commFlag == IsR3ToR0)
	{
		// 是走自定义的通信
		KdPrint(("[info]: Comm_RtlQueryAttributeInformation -- ExpDisQueryAttributeInformation回调触发\r\n"));
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
		// 是走自定义的通信
		KdPrint(("[info]: Comm_RtlSetAttributeInformation -- ExpDisSetAttributeInformation回调触发\r\n"));
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

// Win10回调函数



// 功能调度函数
VOID DispatchCallEntry(PPACKET packet)
{
	ULONG FnId = packet->commFnID;
	switch (FnId)
	{
	case DriverRead:
	{
		KdPrint(("[info]: Comm_DispatchCallEntry -- 读功能\r\n"));
		packet->result = 200;
		break;
	}
	case DriverWrite:
	{
		KdPrint(("[info]: Comm_DispatchCallEntry -- 写功能\r\n"));
		packet->result = 300;
		break;
	}
	default:
	{
		KdPrint(("[info]: Comm_DispatchCallEntry -- 无效的通信ID\r\n"));
		break;
	}
	}
}

// 回调卸载
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
		ExpDisQueryAttributeInfo = 0;
		DbgPrint("[info]: Comm_UnRegCallBackWin7 -- 卸载Win7通信回调\r\n");
	}
}
VOID UnRegCallBackWin10()
{
	DbgPrint("[info]: Comm_UnRegCallBackWin10 -- 卸载Win10通信回调\r\n");
}
