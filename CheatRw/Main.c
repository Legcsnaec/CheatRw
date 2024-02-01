#include <ntifs.h>
#include "Comm.h"
#include "GetModule.h"
#include "ReadWrite.h"

// ���ܵ��Ⱥ���
VOID DispatchCallEntry(PPACKET packet)
{
	KdPrint(("Current Thread PreviousMode is:%d\r\n", ExGetPreviousMode()));
	switch (packet->CommFnID)
	{
	case CMD_DriverRead:
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
	case CMD_DriverWrite:
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
	case CMD_GetModuleR3:
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
	DbgPrint("unload ... \n\r");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pReg)
{
	UNREFERENCED_PARAMETER(pReg);

	NTSTATUS stat = STATUS_SUCCESS;

	stat = CommInitialize(DispatchCallEntry);

	pDrv->DriverUnload = DriverUnload;

	return stat;
}
