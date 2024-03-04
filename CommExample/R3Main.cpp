#include <iostream>
#include "Comm.h"
#include "ExportApi.h"
using namespace std;

unsigned char shellcodex86[] = { 0x6A,0x00,0x6A,0x00,0x6A,0x00,0x6A,0x00,0xB8,0x60,0x0D,0x8D,0x76,0xFF,0xD0,0xC3 };
unsigned char shellcodex64[] = { 0x4D,0x33,0xC0,0x4D,0x33,0xC9,0x48,0x33,0xC9,0x48,0x33,0xD2,0x48,0xB8,0x10,0xAC,0x0D,0x28,0xFD,0x7F,0x00,0x00,0xFF,0xD0,0xC3 };

int main()
{
	NTSTATUS stat;
	BOOLEAN ret = CtDriverLoad();
	if (ret)
	{
		std::cout << "driver is install !! " << endl;

		ULONG p;
		std::cout << "please input pid:" << endl;
		std::cin >> p;

		// æ‰±˙±£ª§≤‚ ‘
		ProtectHandleInfo data4 = { 0 };
		data4.Pid = p;
		data4.IsInstall = TRUE;
		stat = SendCommPacket(CMD_PROTECT_HANDLE, &data4, sizeof(ProtectHandleInfo));
		printf("stat:%x\n", stat);

		// º¸ Û≤‚ ‘
		MouseMoveABSOLUTE(35, 107);
		MouseLeftButtonDown();
		MouseLeftButtonUp();

		MouseLeftButtonDown();
		MouseLeftButtonUp();
	}
	
	system("pause");
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}