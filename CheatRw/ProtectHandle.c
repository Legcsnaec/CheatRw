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

	// ��ǰ���̾��Ǳ�������,�Լ����Լ�
	if (curPid == ProtectProcessPid)
	{
		return OB_PREOP_SUCCESS;
	}
	// Ŀ����̲��Ǳ�������
	if (tagPid != ProtectProcessPid)
	{
		return OB_PREOP_SUCCESS;
	}
	// Ȩ�޸���
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

	// �ҵ�ntoskrnl�µ�����ָ���ַ
	ULONG64 jmpRcx = SearchCode("ntoskrnl.exe", ".text", JMP_RCX, 0);
	if (jmpRcx == 0)
	{
		return STATUS_UNSUCCESSFUL;
	}

	// ע��ص�
	OB_OPERATION_REGISTRATION obOp = { 0 };
	obOp.ObjectType = PsProcessType;
	obOp.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	obOp.PreOperation = jmpRcx;		// ǰ�ص�
	obOp.PostOperation = NULL;		// ��ص�

	OB_CALLBACK_REGISTRATION obCallRegster = { 0 };
	obCallRegster.Version = ObGetFilterVersion();
	obCallRegster.OperationRegistrationCount = 1;
	obCallRegster.RegistrationContext = PobPreOperationCallback;	// ����Ϊ����Ҫִ�еĻص�
	obCallRegster.OperationRegistration = &obOp;
	RtlInitUnicodeString(&obCallRegster.Altitude, L"8888");			// ���Ρ��߶�,ֵԽ��ص�Խ��ִ��

	// ע�����ص�
	return CT_ObRegisterCallbacks(&obCallRegster, &RegCallbackHandle);
}

VOID DestoryCallback()
{
	// ȡ������ص�
	if (RegCallbackHandle)
	{
		ObUnRegisterCallbacks(RegCallbackHandle);
		RegCallbackHandle = NULL;
	}
}