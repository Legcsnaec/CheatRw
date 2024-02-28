#include <windows.h>
#include <Winsvc.h>
#include "LoadDriver.h"
//#include "dll.h"

int* sysData = nullptr;

LoadDriver::LoadDriver()
{

}

LoadDriver::~LoadDriver()
{

}

bool LoadDriver::load(std::string path, std::string serviceName)
{
	//打开服务管理
	SC_HANDLE hSc = NULL;
	hSc = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSc == NULL)
	{
		return false;
	}

	//创建服务
	SC_HANDLE hService = NULL;
	hService = CreateServiceA(
		hSc, serviceName.c_str(), serviceName.c_str(),
		SC_MANAGER_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE, path.c_str(), NULL, NULL, NULL, NULL, NULL);
	if (hService == NULL)
	{
		CloseServiceHandle(hSc);
		return false;
	}

	//启动服务
	if (!StartServiceA(hService, NULL, NULL))
	{
		CloseServiceHandle(hSc);
		CloseServiceHandle(hService);
		return false;
	}

	CloseServiceHandle(hSc);
	CloseServiceHandle(hService);
	return true;
}

bool LoadDriver::unload(std::string serviceName)
{
	SC_HANDLE hServiceMgr = OpenSCManagerA(0, 0, SC_MANAGER_ALL_ACCESS);
	SC_HANDLE hService = OpenServiceA(hServiceMgr, serviceName.c_str(), SERVICE_ALL_ACCESS);
	if (hServiceMgr == NULL || hService == NULL)
	{
		if (hServiceMgr) CloseServiceHandle(hServiceMgr);
		if (hService) CloseServiceHandle(hService);
		return FALSE;
	}

	//停止驱动
	SERVICE_STATUS serviceStatus = { 0 };
	ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);

	//卸载驱动
	DeleteService(hService);

	//关闭句柄
	CloseServiceHandle(hService);
	CloseServiceHandle(hServiceMgr);
}

HMODULE GetSelfModuleHandle()
{
	MEMORY_BASIC_INFORMATION mbi;
	return ((::VirtualQuery(GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0) ? (HMODULE)mbi.AllocationBase : NULL);
}

HMODULE LoadDriver::getDllBase()
{
	return GetSelfModuleHandle();
}

bool LoadDriver::installDriver(std::string path, std::string serviceName)
{
	HRSRC hResc;
	DWORD dwImageSize;
	HANDLE hFile;
	DWORD dwByteWrite;
	HGLOBAL	hResourecImage;
	CHAR str[512] = { 0 };

	// 或许是上次由于未知错误, 导致驱动卸载不干净, 这里卸载一次
	this->unload(serviceName.c_str());

	// 在自定义资源中释放出sys
	dwImageSize = sizeof(sysData);
	unsigned char* pMemory = (unsigned char*)malloc(dwImageSize);
	memcpy(pMemory, sysData, dwImageSize);
	for (ULONG i = 0; i < dwImageSize; i++)
	{
		pMemory[i] ^= 0xd8;
		pMemory[i] ^= 0xcd;
	}

	hFile = CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugStringA(path.c_str());
		return false;
	}
	if (!WriteFile(hFile, pMemory, dwImageSize, &dwByteWrite, NULL))
	{
		OutputDebugStringA(path.c_str());
		CloseHandle(hFile);
		return false;
	}
	if (dwByteWrite != dwImageSize)
	{
		OutputDebugStringA(path.c_str());
		CloseHandle(hFile);
		return false;
	}
	CloseHandle(hFile);

	// 安装驱动
	if (!this->load(path, serviceName))
	{
		DeleteFileA(path.c_str());
		return false;
	}
	DeleteFileA(path.c_str());
	return true;
}
