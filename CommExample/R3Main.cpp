#include <iostream>
#include "Comm.h"
using namespace std;

int main()
{
	NTSTATUS stat;

	R3ModuleInfo data = { 0 };
	data.ModuleName = (char*)"ntdll.dll";
	data.Pid = (HANDLE)4584;

	stat = SendCommPacket(CMD_GetModuleR3, &data, sizeof(R3ModuleInfo));
	printf("stat:%x   base:%llX    size:%llX\n", stat, data.ModuleBase, data.ModuleSize);

	DWORD readD = 0;
	ReadMemInfo data1 = { 0 };
	data1.Pid = (HANDLE)4584;
	data1.TagAddress = (LPVOID)0x7FF630181F10;
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
	data2.Pid = 4584;
	data2.TagAddress = 0x7FF630181F10;
	data2.WriteBuffer = (ULONG64)writeBuf;
	data2.WriteSize = 4;

	stat = SendCommPacket(CMD_DriverWrite, &data2, sizeof(WriteMemInfo));
	printf("stat:%x\n", stat);

	QueryMemInfo data3 = { 0 };
	data3.Pid = 4584;
	data3.BaseAddress = 0x2e10010;

	stat = SendCommPacket(CMD_QueryMemory, &data3, sizeof(QueryMemInfo));
	printf("stat:%x   AllocationBase:%llx   AllocationProtect:%llx   Protect:%llx   \n",
		stat, data3.MemBasicInfo.AllocationBase, data3.MemBasicInfo.AllocationProtect, data3.MemBasicInfo.Protect);





	system("pause");
	return 0;
}