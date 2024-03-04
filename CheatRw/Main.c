#include <ntifs.h>
#include "Comm.h"
#include "GetModule.h"
#include "ReadWrite.h"
#include "CheatTools.h"
#include "ProtectHandle.h"
#include "RemoteCall.h"
#include "km/kmclass.h"

// 功能调度函数
VOID DispatchCallEntry(PPACKET packet)
{
	KdPrint(("Current Thread PreviousMode is:%d\r\n", ExGetPreviousMode()));
	switch (packet->CommFnID)
	{
	case CMD_TEST_CONNECT:
	{
		// 用于测试驱动连通性 ,三环在要加载驱动前测试一下是否连通,防止重复加载驱动
		KdPrint(("[info]: Main_DispatchCallEntry -- 已连通\r\n"));
		packet->ResponseCode = STATUS_SUCCESS;
		break;
	}
	case CMD_READ_MEMORY:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- 读功能\r\n"));
		PReadMemInfo info = (PReadMemInfo)packet->Request;
		if (info)
		{
			packet->ResponseCode = ReadMemory((HANDLE)info->Pid, (PVOID)info->TagAddress, (PVOID)info->ReadBuffer, info->ReadSize);
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
		PWriteMemInfo info = (PWriteMemInfo)packet->Request;
		if (info)
		{
			packet->ResponseCode = WriteMemory((HANDLE)info->Pid, (PVOID)info->TagAddress, (PVOID)info->WriteBuffer, info->WriteSize);
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
		PR3ModuleInfo info = (PR3ModuleInfo)packet->Request;
		if (info)
		{
			ULONG64 moduleSize = 0;
			info->ModuleBase = GetModuleR3((HANDLE)info->Pid, (char*)info->ModuleName, &moduleSize);
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
		PQueryMemInfo info = (PQueryMemInfo)packet->Request;
		if (info)
		{
			packet->ResponseCode = QueryMemory((HANDLE)info->Pid, info->BaseAddress, &info->MemBasicInfo);
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
		PProtectHandleInfo info = (PProtectHandleInfo)packet->Request;
		if (info)
		{
			if (info->IsInstall == TRUE)
			{
				SetProtectPid((HANDLE)info->Pid);
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
		PRemoteCallInfo info = (PRemoteCallInfo)packet->Request;
		if (info)
		{
			packet->ResponseCode = RtlRemoteCall((HANDLE)info->Pid, (PVOID)info->ShellCodePtr, info->ShellCodeSize);
		}
		else
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	case CMD_KEYBOARD:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- 模拟键盘功能\r\n"));
		PKEYBOARD_INPUT_DATA info = (PKEYBOARD_INPUT_DATA)packet->Request;
		if (info)
		{
			ExecuteKeyboardCallback(info);
			packet->ResponseCode = STATUS_SUCCESS;
		}
		else
		{
			packet->ResponseCode = STATUS_UNSUCCESSFUL;
		}
		break;
	}
	case CMD_MOUSE:
	{
		KdPrint(("[info]: Main_DispatchCallEntry -- 模拟鼠标功能\r\n"));
		PMOUSE_INPUT_DATA info = (PMOUSE_INPUT_DATA)packet->Request;
		if (info)
		{
			ExecuteMouseCallback(info);
			packet->ResponseCode = STATUS_SUCCESS;
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
		CommUninitialize();
		goto end;
	}

	stat = InitKmClass();
	if (!NT_SUCCESS(stat))
	{
		CommUninitialize();
		DestoryCallback();
	}

end:
	if (pDrv)
	{
		pDrv->DriverUnload = DriverUnload;
	}
	return stat;
}
