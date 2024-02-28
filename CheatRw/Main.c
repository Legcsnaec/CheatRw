#include <ntifs.h>
#include "Comm.h"
#include "GetModule.h"
#include "ReadWrite.h"
#include "CheatTools.h"
#include "ProtectHandle.h"
#include "RemoteCall.h"

// 功能调度函数
VOID DispatchCallEntry(PPACKET packet)
{
	KdPrint(("Current Thread PreviousMode is:%d\r\n", ExGetPreviousMode()));
	switch (packet->CommFnID)
	{
	case CMD_READ_MEMORY:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- 读功能\r\n"));
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
		KdPrint(("[info]: Main_DispatchCallEntry -- 写功能\r\n"));
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
		KdPrint(("[info]: Main_DispatchCallEntry -- 得到R3模块基址和大小\r\n"));
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
		KdPrint(("[info]: Main_DispatchCallEntry -- 查询内存功能\r\n"));
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
		KdPrint(("[info]: Main_DispatchCallEntry -- 句柄保护功能\r\n"));
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
		KdPrint(("[info]: Main_DispatchCallEntry -- 远程call功能\r\n"));
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
		KdPrint(("[info]: Main_DispatchCallEntry -- 无效的通信ID\r\n"));
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

	// 作为被隐藏的驱动,无驱动对象
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
