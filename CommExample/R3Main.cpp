#include <iostream>
#include "Comm.h"

using namespace std;

unsigned char shellcodex86[] = { 0x6A,0x00,0x6A,0x00,0x6A,0x00,0x6A,0x00,0xB8,0x60,0x0D,0x8D,0x76,0xFF,0xD0,0xC3 };
unsigned char shellcodex64[] = { 0x4D,0x33,0xC0,0x4D,0x33,0xC9,0x48,0x33,0xC9,0x48,0x33,0xD2,0x48,0xB8,0x10,0xAC,0x0D,0x28,0xFD,0x7F,0x00,0x00,0xFF,0xD0,0xC3 };

int main()
{
	NTSTATUS stat;

	ULONG64 pid = 4560;
	ULONG64 address = 0x7FF79A3F22F0;
	/*
	R3ModuleInfo data = { 0 };
	data.ModuleName = (char*)"ntdll.dll";
	data.Pid = (HANDLE)pid;

	stat = SendCommPacket(CMD_GetModuleR3, &data, sizeof(R3ModuleInfo));
	printf("stat:%x   base:%llX    size:%llX\n", stat, data.ModuleBase, data.ModuleSize);

	DWORD readD = 0;
	ReadMemInfo data1 = { 0 };
	data1.Pid = (HANDLE)pid;
	data1.TagAddress = (LPVOID)address;
	data1.ReadSize = 4;
	data1.ReadBuffer = &readD;

	clock_t start = clock();
	for (size_t i = 0; i < 10; i++)
	{
		stat = SendCommPacket(CMD_DriverRead, &data1, sizeof(ReadMemInfo));
		//printf("stat:%x   ReadBuffer:%x\n", stat, *(PDWORD)data1.ReadBuffer);
	}
	clock_t end = clock();
	printf("time:%d\n", end - start);
	printf("stat:%x   ReadBuffer:%x\n", stat, *(PDWORD)data1.ReadBuffer);

	char writeBuf[] = { 'a','b','c' };
	WriteMemInfo data2 = { 0 };
	data2.Pid = pid;
	data2.TagAddress = address;
	data2.WriteBuffer = (ULONG64)writeBuf;
	data2.WriteSize = 4;

	stat = SendCommPacket(CMD_DriverWrite, &data2, sizeof(WriteMemInfo));
	printf("stat:%x\n", stat);

	QueryMemInfo data3 = { 0 };
	data3.Pid = pid;
	data3.BaseAddress = 0x7FFE1000;

	stat = SendCommPacket(CMD_QueryMemory, &data3, sizeof(QueryMemInfo));
	printf("stat:%x   AllocationBase:%llx   AllocationProtect:%llx   Protect:%llx   \n",
		stat, data3.MemBasicInfo.AllocationBase, data3.MemBasicInfo.AllocationProtect, data3.MemBasicInfo.Protect);

	ProtectHandleInfo data4 = { 0 };
	data4.Pid = pid;
	data4.IsInstall = TRUE;
	stat = SendCommPacket(CMD_ProtectHandle, &data4, sizeof(ProtectHandleInfo));
	printf("stat:%x\n", stat);
	*/

	ULONG p;
	std::cout << "please input pid:" << endl;
	std::cin >> p;

	ProtectHandleInfo data4 = { 0 };
	data4.Pid = p;
	data4.IsInstall = TRUE;
	stat = SendCommPacket(CMD_PROTECT_HANDLE, &data4, sizeof(ProtectHandleInfo));
	printf("stat:%x\n", stat);

	/*RemoteCallInfo data5 = { 0 };
	data5.Pid = p;
	data5.ShellCodePtr = (ULONG64)shellcodex86;
	data5.ShellCodeSize = sizeof(shellcodex86);
	stat = SendCommPacket(CMD_REMOTE_CALL, &data5, sizeof(RemoteCallInfo));
	printf("stat:%x\n", stat);*/

	system("pause");
	return 0;
}