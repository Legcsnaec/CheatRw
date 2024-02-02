#include "ProtectHandle.h"
#include "SearchCode.h"
#include "CheatTools.h"

#define JMP_RCX "FFE1"
#define DRV_NAME L"\\Driver\\WMIxWDM"

PVOID RegCallbackHandle = NULL;
HANDLE ProtectProcessPid = NULL;

VOID SetProtectPid(HANDLE pid)
{
	ProtectProcessPid = pid;
}

OB_PREOP_CALLBACK_STATUS PobPreOperationCallback(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION OperationInformation)
{
	PEPROCESS eprocess = OperationInformation->Object;
	if (!eprocess || ProtectProcessPid == NULL)
	{
		return OB_PREOP_SUCCESS;
	}

	HANDLE curPid = PsGetCurrentProcessId();
	HANDLE tagPid = PsGetProcessId(eprocess);

	// 当前进程就是保护进程,自己打开自己
	if (curPid == ProtectProcessPid)
	{
		return OB_PREOP_SUCCESS;
	}
	// 目标进程不是保护进程
	if (tagPid != ProtectProcessPid)
	{
		return OB_PREOP_SUCCESS;
	}
	// 权限赋空
	if (OperationInformation->Operation == OB_OPERATION_HANDLE_CREATE)
	{
		OperationInformation->Parameters->CreateHandleInformation.DesiredAccess = 0;
		OperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess = 0;
	}
	else
	{
		OperationInformation->Parameters->DuplicateHandleInformation.DesiredAccess = 0;
		OperationInformation->Parameters->DuplicateHandleInformation.OriginalDesiredAccess = 0;
	}
	return OB_PREOP_SUCCESS;
}

NTSTATUS RegisterCallback()
{
	if (RegCallbackHandle != NULL)
	{
		return STATUS_UNSUCCESSFUL;
	}

	// 找到ntoskrnl下的跳板指令地址
	ULONG64 jmpRcx = SearchCode("ntoskrnl.exe", ".text", JMP_RCX, 0);
	if (jmpRcx == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	// 注册回调
	OB_OPERATION_REGISTRATION obOp = { 0 };
	obOp.ObjectType = PsProcessType;
	obOp.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	obOp.PreOperation = jmpRcx;		// 前回调
	obOp.PostOperation = NULL;		// 后回调

	OB_CALLBACK_REGISTRATION obCallRegster = { 0 };
	obCallRegster.Version = ObGetFilterVersion();
	obCallRegster.OperationRegistrationCount = 1;
	obCallRegster.RegistrationContext = PobPreOperationCallback;	// 参数为真正要执行的回调
	obCallRegster.OperationRegistration = &obOp;
	RtlInitUnicodeString(&obCallRegster.Altitude, L"8888");			// 海拔、高度,值越大回调越后执行

	// 注册句柄回调
	return CT_ObRegisterCallbacks(&obCallRegster, &RegCallbackHandle);
}

VOID DestoryCallback()
{
	// 取消句柄回调
	if (RegCallbackHandle)
	{
		ObUnRegisterCallbacks(RegCallbackHandle);
		RegCallbackHandle = NULL;
	}
}