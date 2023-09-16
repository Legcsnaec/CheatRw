#include "Comm.h"

// Win7通信函数
ULONG CommWin7(ULONG cmd,PVOID inData,SIZE_T inSize)
{
	if (cmd >= COMM_NUMBER_SIZE || cmd == IsR3ToR0) return 0;

	ULONG ret = 0;
	HMODULE hMod = NULL;
	HANDLE hFile = NULL;
	_NtQueryInformationFile NtQueryInformationFile = NULL;

	hMod = LoadLibraryW(L"ntdll.dll");
	if (hMod == NULL) return STATUS_INVALID_HANDLE;
	
	NtQueryInformationFile = (_NtQueryInformationFile)GetProcAddress(hMod, "NtQueryInformationFile");
	if (NtQueryInformationFile == NULL) return STATUS_INVALID_HANDLE;
	
	hFile = CreateFileA("tmp", GENERIC_ALL, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == NULL) return STATUS_INVALID_HANDLE;

	PACKET packet;
	packet.commFlag = IsR3ToR0;
	packet.commFnID = (COMM_NUMBER)cmd;
	packet.data = inData;
	packet.length = inSize;
	packet.result = 0;

	IO_STATUS_BLOCK ioStatus = { 0 };
	char buffer[0xE0] = { 0 };
	memcpy(buffer, &packet, sizeof(PACKET));
	ret = NtQueryInformationFile(hFile, &ioStatus, buffer, 0x100, 0x34);
	memcpy(&packet, buffer, sizeof(PACKET));
	
	CloseHandle(hFile);
	return packet.result;
}

//Win10通信
ULONG CommWin10(ULONG cmd, PVOID inData, SIZE_T inSize)
{
	if (cmd >= COMM_NUMBER_SIZE || cmd == IsR3ToR0) return 0;

	ULONG ret = 0;
	ULONG64 arg = 0;
	HMODULE hMod = NULL;
	NtConvertBetweenProc NtConvertBetween = NULL;

	hMod = LoadLibraryA("ntdll.dll");
	if (hMod == NULL)
	{
		return STATUS_INVALID_HANDLE;
	}
	NtConvertBetween = (NtConvertBetweenProc)GetProcAddress(hMod, "NtConvertBetweenAuxiliaryCounterAndPerformanceCounter");
	if (NtConvertBetween == NULL)
	{
		return STATUS_INVALID_HANDLE;
	}

	PACKET packet;
	PPACKET pPack = &packet;
	packet.commFlag = IsR3ToR0;
	packet.commFnID = (COMM_NUMBER)cmd;
	packet.data = inData;
	packet.length = inSize;
	packet.result = 0;

	NtConvertBetween(1, &pPack, &arg, &arg);
	return packet.result;
}
