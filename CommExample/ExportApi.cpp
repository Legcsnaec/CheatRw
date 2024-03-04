#include <time.h>
#include "ExportApi.h"
#include "LoadDriver.h"
#include "Comm.h"

char AZTable[62] = {
	'A','B','C','D','E','F','G','H','I','J','K','M','N','L','O','P','Q','R',
	'S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j',
	'k','m','n','l','o','p','q','r','s','t','u','v','w','x','y','z','0','1',
	'2','3','4','5','6','7','8','9'
};

char* GetRandName()
{
	static char* name = NULL;
	if (name)
	{
		return name;
	}

	name = (char*)malloc(20);
	if (name)
	{
		memset(name, 0, 20);

		time_t t = time(NULL);
		srand(t);
		int len = (rand() % 10) + 5;

		for (int i = 0; i < len; i++)
		{
			int index = rand() % sizeof(AZTable);
			name[i] = AZTable[index];
		}
		strcat(name, ".sys");
	}
	return name;
}

char* GetRandServiceName()
{
	static char* name = NULL;
	if (name)
	{
		return name;
	}

	name = (char*)malloc(10);
	if (name)
	{
		memset(name, 0, 10);
		time_t t = time(NULL);
		srand(t);

		int len = (rand() % 4) + 5;
		for (int i = 0; i < len; i++)
		{
			int index = rand() % sizeof(AZTable);
			name[i] = AZTable[index];
		}
	}
	return name;
}

// ---------------------------------- 导出 ----------------------------------

EXTERN_C BOOLEAN WINAPI CtTestConnect()
{
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_TEST_CONNECT, NULL, 0);
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN WINAPI CtDriverLoad()
{
	LoadDriver Load;

	if (CtTestConnect())
	{
		return TRUE;
	}

	char bufPath[MAX_PATH] = { 0 };
	GetTempPathA(MAX_PATH, bufPath);

	char* driverName = GetRandName();
	char* serviceName = GetRandServiceName();
	strcat(bufPath, driverName);

	// 因为我加载器驱动直接返回的STATUS_UNSUCCESSFUL,所以StartService会返回失败
	// installDriver会返回false,驱动具体运行直接看CtTestConnect返回
	Load.installDriver(bufPath, serviceName);
	return CtTestConnect();
}

EXTERN_C VOID WINAPI CtDriverUnLoad()
{
	LoadDriver Load;
	char* serviceName = GetRandServiceName();
	Load.unload(serviceName);
}

EXTERN_C ULONG64 WINAPI CtGetModuleR3(IN DWORD_PTR pid, IN char* moduleName, OUT PULONG64 moduleSizeAddr)
{
	R3ModuleInfo data = { 0 };
	data.ModuleName = (ULONG64)moduleName;
	data.Pid = pid;
	SendCommPacket(CMD_GET_MODULER3, &data, sizeof(R3ModuleInfo));
	if (moduleSizeAddr)
	{
		*moduleSizeAddr = data.ModuleSize;
	}
	return data.ModuleBase;
}

EXTERN_C BOOLEAN WINAPI CtReadMemory(IN DWORD_PTR pid, IN ULONG64 baseAddress, OUT PVOID readBuffer, IN ULONG readSize)
{
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	ReadMemInfo data = { 0 };
	data.Pid = pid;
	data.TagAddress = baseAddress;
	data.ReadSize = readSize;
	data.ReadBuffer = (ULONG64)readBuffer;
	stat = SendCommPacket(CMD_READ_MEMORY, &data, sizeof(ReadMemInfo));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN WINAPI CtWriteMemory(IN DWORD_PTR pid, IN ULONG64 baseAddress, IN PVOID writeBuffer, IN ULONG writeSize)
{
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	WriteMemInfo data = { 0 };
	data.Pid = pid;
	data.TagAddress = baseAddress;
	data.WriteSize = writeSize;
	data.WriteBuffer = (ULONG64)writeBuffer;
	stat = SendCommPacket(CMD_WRITE_MEMORY, &data, sizeof(WriteMemInfo));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN WINAPI CtQueryMemory(IN DWORD_PTR pid, IN ULONG64 baseAddress, OUT PMMEMORY_BASIC_INFORMATION basicInfoAddr)
{
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	QueryMemInfo data = { 0 };
	data.Pid = pid;
	data.BaseAddress = baseAddress;
	if (basicInfoAddr)
	{
		memcpy(basicInfoAddr, &data.MemBasicInfo, sizeof(MMEMORY_BASIC_INFORMATION));
	}
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN WINAPI CtProtectHandle(IN DWORD_PTR pid)
{
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	ProtectHandleInfo data = { 0 };
	data.Pid = pid;
	data.IsInstall = TRUE;
	stat = SendCommPacket(CMD_PROTECT_HANDLE, &data, sizeof(ProtectHandleInfo));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN WINAPI CtRemoteCall(DWORD_PTR pid, PVOID shellcode, DWORD shellcodeSize)
{
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	RemoteCallInfo data = { 0 };
	data.Pid = pid;
	data.ShellCodePtr = (ULONG64)shellcode;
	data.ShellCodeSize = shellcodeSize;
	stat = SendCommPacket(CMD_REMOTE_CALL, &data, sizeof(RemoteCallInfo));
	return stat == RW_STATUS_SUCCESS;
}

// ---------------------------------- 键鼠相关导出 ----------------------------------

EXTERN_C BOOLEAN KeyDown(USHORT VirtualKey)
{
	KEYBOARD_INPUT_DATA  kid;
	DWORD dwOutput;
	memset(&kid, 0, sizeof(KEYBOARD_INPUT_DATA));
	kid.Flags = KEY_DOWN;
	kid.MakeCode = (USHORT)MapVirtualKey(VirtualKey, 0);

	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_KEYBOARD, &kid, sizeof(KEYBOARD_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN KeyUp(USHORT VirtualKey)
{
	KEYBOARD_INPUT_DATA  kid;
	DWORD dwOutput;
	memset(&kid, 0, sizeof(KEYBOARD_INPUT_DATA));
	kid.Flags = KEY_UP;
	kid.MakeCode = (USHORT)MapVirtualKey(VirtualKey, 0);

	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_KEYBOARD, &kid, sizeof(KEYBOARD_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseLeftButtonDown()
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.ButtonFlags = MOUSE_LEFT_BUTTON_DOWN;

	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseLeftButtonUp()
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.ButtonFlags = MOUSE_LEFT_BUTTON_UP;

	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseRightButtonDown()
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.ButtonFlags = MOUSE_RIGHT_BUTTON_DOWN;
	
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseRightButtonUp()
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.ButtonFlags = MOUSE_RIGHT_BUTTON_UP;
	
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseMiddleButtonDown()
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.ButtonFlags = MOUSE_MIDDLE_BUTTON_DOWN;
	
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseMiddleButtonUp()
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.ButtonFlags = MOUSE_MIDDLE_BUTTON_UP;
	
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseMoveRELATIVE(LONG dx, LONG dy)
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.Flags = MOUSE_MOVE_RELATIVE;
	mid.LastX = dx;
	mid.LastY = dy;

	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}

EXTERN_C BOOLEAN MouseMoveABSOLUTE(LONG dx, LONG dy)
{
	MOUSE_INPUT_DATA  mid;
	DWORD dwOutput;
	memset(&mid, 0, sizeof(MOUSE_INPUT_DATA));
	mid.Flags = MOUSE_MOVE_ABSOLUTE;
	mid.LastX = dx * 0xffff / GetSystemMetrics(SM_CXSCREEN);
	mid.LastY = dy * 0xffff / GetSystemMetrics(SM_CYSCREEN);

	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	stat = SendCommPacket(CMD_MOUSE, &mid, sizeof(MOUSE_INPUT_DATA));
	return stat == RW_STATUS_SUCCESS;
}