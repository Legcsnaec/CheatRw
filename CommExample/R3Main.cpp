#include <iostream>
#include "Comm.h"
using namespace std;

int main()
{
	NTSTATUS stat;

	R3ModuleInfo data = { 0 };
	data.ModuleName = (char*)"ntdll.dll";
	data.pid = (HANDLE)4632;

	stat = SendCommPacket(CMD_GetModuleR3, &data, 4);

	printf("stat:%x   base:%llX    size:%llX\n", stat, data.ModuleBase, data.ModuleSize);


	system("pause");
	return 0;
}