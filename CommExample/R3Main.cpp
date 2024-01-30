#include <iostream>
#include "Comm.h"
using namespace std;

int main()
{
	NTSTATUS stat;

	R3ModuleInfo data = { 0 };
	data.ModuleName = (char*)"ntdll.dll";
	data.Pid = (HANDLE)4316;

	stat = SendCommPacket(CMD_GetModuleR3, &data, sizeof(R3ModuleInfo));
	printf("stat:%x   base:%llX    size:%llX\n", stat, data.ModuleBase, data.ModuleSize);

	DWORD readD = 0;
	ReadMemInfo data1 = { 0 };
	data1.Pid = (HANDLE)4316;
	data1.TagAddress = (LPVOID)0x7FF6E35E1F10;
	data1.ReadSize = 4;
	data1.ReadBuffer = &readD;

	stat = SendCommPacket(CMD_DriverRead, &data1, sizeof(ReadMemInfo));
	printf("stat:%x   ReadBuffer:%x\n", stat, *(PDWORD)data1.ReadBuffer);

	system("pause");
	return 0;
}