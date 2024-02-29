#include "Comm.h"

// -- 自定义结构 --
// Win7 驱动通信
typedef struct _IO_STATUS_BLOCK
{
	union
	{
		PVOID Status;
		PVOID Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;
typedef ULONG(*NtQueryInformationFilePfn)(HANDLE FileHandle, PIO_STATUS_BLOCK IoStatusBlock, PVOID  FileInformation, ULONG Length, ULONG FileInformationClass);

// Win10驱动通信
typedef ULONG64(_fastcall* NtConvertBetweenPfn)(char, PVOID, PVOID, PVOID);

// 全局保存
HANDLE CommFile = NULL;
NtConvertBetweenPfn NtConvertBetween = NULL;				// WIN 10
NtQueryInformationFilePfn NtQueryInformationFile = NULL;	// WIN 7

// 获取内部版本号
USHORT GetOsBuildNumber()
{
	static USHORT osNumber = 0;
	if (osNumber) return osNumber;
#ifdef _WIN64
	ULONG64 peb = __readgsqword(0x60);
	osNumber = *(PUSHORT)(peb + 0x120);
#else
	DWORD peb = __readfsdword(0x30);
	osNumber = *(PUSHORT)(peb + 0xac);
#endif
	return osNumber;
}

// Win7通信函数
ULONG CommWin7(ULONG cmd, PVOID request,SIZE_T inSize)
{
	if (cmd >= COMM_NUMBER_SIZE || cmd == IsR3ToR0 || NtQueryInformationFile == NULL)
	{
		return RW_STATUS_INVALID_PARAMETER;
	}
	PACKET packet;
	packet.CommFlag = IsR3ToR0;
	packet.CommFnID = (COMM_NUMBER)cmd;
	packet.Request = (ULONG64)request;
	packet.Length = inSize;
	packet.ResponseCode = RW_STATUS_UNSUCCESSFUL;

	IO_STATUS_BLOCK ioStatus = { 0 };
	char buffer[0xE0] = { 0 };
	memcpy(buffer, &packet, sizeof(PACKET));
	NtQueryInformationFile(CommFile, &ioStatus, buffer, 0x100, 0x34);
	memcpy(&packet, buffer, sizeof(PACKET));
	return packet.ResponseCode;
}

//Win10通信
ULONG CommWin10(ULONG cmd, PVOID request, SIZE_T inSize)
{
	if (cmd >= COMM_NUMBER_SIZE || cmd == IsR3ToR0 || NtConvertBetween == NULL)
	{
		return RW_STATUS_INVALID_PARAMETER;
	}

	PACKET packet;
	PPACKET pPack = &packet;
	packet.CommFlag = IsR3ToR0;
	packet.CommFnID = (COMM_NUMBER)cmd;
	packet.Request = (ULONG64)request;
	packet.Length = inSize;
	packet.ResponseCode = RW_STATUS_UNSUCCESSFUL;

	ULONG64 arg = 0;
	NtConvertBetween(1, &pPack, &arg, &arg);
	return packet.ResponseCode;
}

// 通信初始化,会占一个文件句柄程序结束后释放
BOOL InitAppComm()
{
	if (NtConvertBetween || NtQueryInformationFile)
	{
		return TRUE;
	}

	HMODULE hMod = NULL;
	hMod = LoadLibraryW(L"ntdll.dll");
	if (hMod == NULL)
	{
		return FALSE;
	}

	USHORT number = GetOsBuildNumber();
	if (number == 7600 || number == 7601)
	{
		NtQueryInformationFile = (NtQueryInformationFilePfn)GetProcAddress(hMod, "NtQueryInformationFile");

		char bufPath[MAX_PATH] = { 0 };
		GetTempPathA(MAX_PATH, bufPath);
		strcat_s(bufPath, "\\1.txt");
		CommFile = CreateFileA(bufPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (NtQueryInformationFile == NULL || CommFile == NULL)
		{
			return FALSE;
		}
	}
	else
	{
		NtConvertBetween = (NtConvertBetweenPfn)GetProcAddress(hMod, "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter");
		if (NtConvertBetween == NULL)
		{
			return FALSE;
		}
	}
	return TRUE;
}

// 发送通信包
NTSTATUS SendCommPacket(ULONG cmd, PVOID inData, SIZE_T inSize)
{
	
	NTSTATUS stat = RW_STATUS_UNSUCCESSFUL;
	USHORT buildNumber = GetOsBuildNumber();
	if (InitAppComm())
	{
		if (buildNumber == 7600 || buildNumber == 7601)
		{
			stat = CommWin7(cmd, inData, inSize);
		}
		else
		{
			stat = CommWin10(cmd, inData, inSize);
		}
	}
	else
	{
		stat = RW_STATUS_COMMINITFAILURE;
	}
	return stat;
}