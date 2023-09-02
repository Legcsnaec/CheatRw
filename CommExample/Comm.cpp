#include "Comm.h"

// Win7Í¨ÐÅº¯Êý
ULONG CommWin7(PVOID packet)
{
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

	IO_STATUS_BLOCK ioStatus = { 0 };
	char buffer[0x100] = { 0 };
	memcpy(buffer, packet, sizeof(PACKET));
	ret = NtQueryInformationFile(hFile, &ioStatus, buffer, 0x100, 52);
	memcpy(packet, buffer, sizeof(PACKET));
	
	CloseHandle(hFile);

	return ret;
}