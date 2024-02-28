#include <time.h>
#include "ExportApi.h"
#include "LoadDriver.h"
#include "Comm.h"

char AZTable[62] =
{
	0,
};

void initTable()
{
	if (AZTable[0] != 0) return;
	int k = 0;
	for (char i = 'A'; i <= 'Z'; i++, k++)
	{
		AZTable[k] = i;
	}

	for (char i = 'a'; i <= 'z'; i++, k++)
	{
		AZTable[k] = i;
	}

	for (char i = '0'; i <= '9'; i++, k++)
	{
		AZTable[k] = i;
	}
}

char* GetRandName()
{
	static char* name = NULL;
	if (name) return name;

	initTable();

	name = (char*)malloc(20);

	memset(name, 0, 20);  //15 .sys 0

	time_t t = time(NULL);
	srand(t);

	int len = (rand() % 10) + 5;
	for (int i = 0; i < len; i++)
	{
		int index = rand() % sizeof(AZTable);
		name[i] = AZTable[index];
	}

	strcat_s(name, strlen(".sys") + 1, ".sys");

	return name;
}

char* GetRandServiceName()
{
	static char* name = NULL;
	if (name) return name;

	initTable();

	name = (char*)malloc(10);

	memset(name, 0, 10);  //15 .sys 0

	time_t t = time(NULL);
	srand(t);

	int len = (rand() % 4) + 5;
	for (int i = 0; i < len; i++)
	{
		int index = rand() % sizeof(AZTable);
		name[i] = AZTable[index];
	}

	return name;
}

EXTERN_C BOOLEAN WINAPI SH_Test()
{
	ULONG64 xx = 0;
	return 0;
}

EXTERN_C BOOLEAN WINAPI CtDriverLoad()
{
	LoadDriver Load;

	if (SH_Test())
	{
		return TRUE;
	}

	char bufPath[MAX_PATH] = { 0 };
	GetTempPathA(MAX_PATH, bufPath);

	char* driverName = GetRandName();
	char* serviceName = GetRandServiceName();
	strcat_s(bufPath, strlen(driverName) + 1, driverName);
	Load.installDriver(bufPath, serviceName);
	return SH_Test();
}

EXTERN_C VOID WINAPI CtDriverUnLoad()
{
	LoadDriver Load;

	char* serviceName = GetRandServiceName();

	Load.unload(serviceName);
}


EXTERN_C ULONG64 WINAPI CtGetModuleR3(DWORD pid, char* moduleName)
{
	return 0;
}


EXTERN_C BOOLEAN WINAPI CtReadMemory(DWORD pid, ULONG64 BaseAddress, PVOID Buffer, ULONG size)
{
	return 0;
}

EXTERN_C BOOLEAN WINAPI CtWriteMemory(DWORD pid, ULONG64 BaseAddress, PVOID Buffer, ULONG size)
{
	return 0;
}

EXTERN_C BOOLEAN WINAPI CtQueryMemory(DWORD pid, ULONG64 BaseAddress, PMMEMORY_BASIC_INFORMATION pinfo)
{
	return 0;
}

EXTERN_C BOOLEAN WINAPI CtProtectHandle(DWORD pid)
{
	return 0;
}

EXTERN_C BOOLEAN WINAPI CtRemoteCall(DWORD pid, PVOID shellcode, DWORD shellcodeSize)
{
	return 0;
}