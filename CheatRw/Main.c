#include <ntifs.h>
#include "Comm.h"
#include "GetModule.h"
#include "ReadWrite.h"
#include "CheatTools.h"
#include "ProtectHandle.h"
#include "RemoteCall.h"

// ���ܵ��Ⱥ���
VOID DispatchCallEntry(PPACKET packet)
{
	KdPrint(("Current Thread PreviousMode is:%d\r\n", ExGetPreviousMode()));
	switch (packet->CommFnID)
	{
	case CMD_READ_MEMORY:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- ������\r\n"));
		PReadMemInfo info = packet->Request;
		if (info)
		{
			packet->ResponseCode = ReadMemory(info->Pid, info->TagAddress, info->ReadBuffer, info->ReadSize);
		}
		else
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	case CMD_WRITE_MEMORY:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- д����\r\n"));
		PWriteMemInfo info = packet->Request;
		if (info)
		{
			packet->ResponseCode = WriteMemory(info->Pid, info->TagAddress, info->WriteBuffer, info->WriteSize);
		}
		else
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	case CMD_GET_MODULER3:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- �õ�R3ģ���ַ�ʹ�С\r\n"));
		PR3ModuleInfo info = packet->Request;
		if (info)
		{
			ULONG64 moduleSize = 0;
			info->ModuleBase = GetModuleR3(info->Pid, info->ModuleName, &moduleSize);
			info->ModuleSize = moduleSize;
			packet->ResponseCode = 0;
		}
		if (!info || info->ModuleBase == 0)
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	case CMD_QUERY_MEMORY:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- ��ѯ�ڴ湦��\r\n"));
		PQueryMemInfo info = packet->Request;
		if (info)
		{
			packet->ResponseCode = QueryMemory(info->Pid, info->BaseAddress, &info->MemBasicInfo);
		}
		else
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	case CMD_PROTECT_HANDLE:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- �����������\r\n"));
		PProtectHandleInfo info = packet->Request;
		if (info)
		{
			if (info->IsInstall == TRUE)
			{
				SetProtectPid(info->Pid);
				packet->ResponseCode = STATUS_SUCCESS;
			}
			else
			{
				SetProtectPid(NULL);
				packet->ResponseCode = STATUS_SUCCESS;
			}
		}
		else
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	case CMD_REMOTE_CALL:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- Զ��call����\r\n"));
		PRemoteCallInfo info = packet->Request;
		if (info)
		{
			packet->ResponseCode = RtlRemoteCall(info->Pid, info->ShellCodePtr, info->ShellCodeSize);
		}
		else
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	default:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- ��Ч��ͨ��ID\r\n"));
		break;
	}
	}
}

VOID DriverUnload(PDRIVER_OBJECT pDrv)
{
	UNREFERENCED_PARAMETER(pDrv);

	CommUninitialize();

	DestoryCallback();

	DbgPrint("unload ... \n\r");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pReg)
{
	NTSTATUS stat = STATUS_SUCCESS;

	DbgPrint(" ------------- CheatRw DriverEntry ------------ \r\n");

	stat = CommInitialize(DispatchCallEntry);
	if (!NT_SUCCESS(stat))
	{
		goto end;
	}

	// ��Ϊ�����ص�����,����������
	// RtlByPassCallBackVerify(pDrv->DriverSection);
	
	stat = RegisterCallback();
	if (!NT_SUCCESS(stat))
	{
		goto end;
	}

end:
	if (pDrv)
	{
		pDrv->DriverUnload = DriverUnload;
	}
	return stat;
}
