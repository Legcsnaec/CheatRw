#include <windows.h>
#include <Winsvc.h>
#include "LoadDriver.h"
#include "dll.h"

LoadDriver::LoadDriver()
{

}

LoadDriver::~LoadDriver()
{

}

bool LoadDriver::load(std::string path, std::string serviceName)
{
	//�򿪷������
	SC_HANDLE hSc = NULL;
	hSc = OpenSCManagerA(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSc == NULL)
	{
		return false;
	}

	//��������
	SC_HANDLE hService = NULL;
	hService = CreateServiceA(
		hSc, serviceName.c_str(), serviceName.c_str(),
		SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE, path.c_str(), NULL, NULL, NULL, NULL, NULL);
	if (hService == NULL)
	{
		CloseServiceHandle(hSc);
		return false;
	}

	//��������
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

	//ֹͣ����
	SERVICE_STATUS serviceStatus = { 0 };
	ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);

	//ж������
	DeleteService(hService);

	//�رվ��
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
	HANDLE hFile;
	DWORD dwByteWrite;
	DWORD dwImageSize;
	CHAR str[512] = { 0 };
	bool isSuccess = true;

	// �������ϴ�����δ֪����, ��������ж�ز��ɾ�, ����ж��һ��
	this->unload(serviceName.c_str());

	// ���Զ�����Դ���ͷų�sys
	dwImageSize = sizeof(sysData);
	unsigned char* pMemory = (unsigned char*)malloc(dwImageSize);
	if (pMemory)
	{
		memcpy(pMemory, sysData, dwImageSize);
		for (ULONG i = 0; i < dwImageSize; i++)
		{
			pMemory[i] ^= XOR_ENCODE_KEY;
		}

		hFile = CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			if (WriteFile(hFile, pMemory, dwImageSize, &dwByteWrite, NULL))
			{
				CloseHandle(hFile);
				if (!this->load(path, serviceName))
				{
					isSuccess = false;
				}
				// ��װ��������win10����ɾ����,��������д��ǿ����ɾ��
				DeleteFileA(path.c_str());
			}
		}
		free(pMemory);
	}
	return isSuccess;
}
