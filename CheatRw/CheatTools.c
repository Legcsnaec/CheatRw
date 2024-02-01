#include "CheatTools.h"
#include "Unrevealed.h"

// 64\32λ�汾,���ݻ�ַ�����Ƶõ�������ַ,���������
PVOID MmGetSystemRoutineAddressEx(ULONG64 modBase, CHAR* searchFnName)
{
	if (modBase == NULL || searchFnName == NULL)  return NULL;
	SIZE_T funcAddr = 0;

	do
	{
		PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)modBase;
		PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(modBase + pDosHdr->e_lfanew);
		PIMAGE_FILE_HEADER pFileHdr = &pNtHdr->FileHeader;
		PIMAGE_OPTIONAL_HEADER64 pOphtHdr64 = NULL;
		PIMAGE_OPTIONAL_HEADER32 pOphtHdr32 = NULL;

		if (pFileHdr->Machine == IMAGE_FILE_MACHINE_I386) pOphtHdr32 = (PIMAGE_OPTIONAL_HEADER32)&pNtHdr->FileHeader;
		else pOphtHdr64 = (PIMAGE_OPTIONAL_HEADER64)&pNtHdr->FileHeader;

		ULONG VirtualAddress = 0;
		if (pOphtHdr64 != NULL) VirtualAddress = pOphtHdr64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		else VirtualAddress = pOphtHdr32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

		// ���� PE 64λ/32λ �õ�������
		PIMAGE_EXPORT_DIRECTORY pExportTable = (IMAGE_EXPORT_DIRECTORY*)(modBase + VirtualAddress);
		if (NULL == pExportTable) break;

		PULONG pAddrFns = (PULONG)(modBase + pExportTable->AddressOfFunctions);
		PULONG pAddrNames = (PULONG)(modBase + pExportTable->AddressOfNames);
		PUSHORT pAddrNameOrdinals = (PUSHORT)(modBase + pExportTable->AddressOfNameOrdinals);

		ULONG funcOrdinal, i;
		char* funcName;
		for (ULONG i = 0; i < pExportTable->NumberOfNames; ++i)
		{
			funcName = (char*)(modBase + pAddrNames[i]);
			// ��3����
			if (modBase < MmUserProbeAddress)
			{
				__try
				{
					if (!_strnicmp(searchFnName, funcName, strlen(searchFnName)))
					{
						if (funcName[strlen(searchFnName)] == NULL)
						{
							funcOrdinal = pExportTable->Base + pAddrNameOrdinals[i] - 1;
							funcAddr = modBase + pAddrFns[funcOrdinal];
							break;
						}
					}
				}
				__except (1) { continue; }
			}
			// ��0����
			else
			{
				if (MmIsAddressValid(funcName) && MmIsAddressValid(funcName + strlen(searchFnName)))
				{
					if (!_strnicmp(searchFnName, funcName, strlen(searchFnName)))
					{
						if (funcName[strlen(searchFnName)] == NULL)
						{
							funcOrdinal = pExportTable->Base + pAddrNameOrdinals[i] - 1;
							funcAddr = modBase + pAddrFns[funcOrdinal];
							break;
						}
					}
				}
			}
		}
	} while (0);

	return (PVOID)funcAddr;
}

// �����û��ռ��ڴ�
PVOID MmAllocateUserVirtualMemory(HANDLE processHandle, SIZE_T allocSize, ULONG allcType, ULONG protect)
{
	PVOID Result = NULL;
	NTSTATUS(NTAPI * RtlAllocateVirtualMemory)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
	if (ExGetPreviousMode() == KernelMode)
	{
		// ����SSDT��
		(*(PVOID*)(&RtlAllocateVirtualMemory)) = ((PVOID)(NtAllocateVirtualMemory));
	}
	else
	{
		(*(PVOID*)(&RtlAllocateVirtualMemory)) = ((PVOID)(ZwAllocateVirtualMemory));
	}
	__try 
	{
		RtlAllocateVirtualMemory(processHandle, &Result, NULL, &allocSize, allcType, protect);
	}
	__except (1) { Result = NULL; }
	return Result;
}

// �ͷ��û��ռ��ڴ�
NTSTATUS MmFreeUserVirtualMemory(HANDLE processHandle, PVOID base)
{
	SIZE_T size = 0;
	NTSTATUS(NTAPI* RtlFreeVirtualMemory)(HANDLE, PVOID*, PSIZE_T, ULONG);
	if (ExGetPreviousMode() == KernelMode)
	{
		(*(PVOID*)(&RtlFreeVirtualMemory)) = ((PVOID)(NtFreeVirtualMemory));
	}
	else
	{
		(*(PVOID*)(&RtlFreeVirtualMemory)) = ((PVOID)(ZwFreeVirtualMemory));
	}
	__try
	{
		RtlFreeVirtualMemory(processHandle, &base, &size, MEM_RELEASE);
	}
	__except (1) { return STATUS_ACCESS_VIOLATION; }
	return STATUS_SUCCESS;
}

// �ڴ��Ƿ��ǰ�ȫ��,ͨ���Ƿ��������ڴ��ж�
BOOLEAN MmIsAddressSafe(PVOID startAddress)
{
	
	return FALSE;
}

// �ָ��ַ���
VOID RtlSplitString(IN PUNICODE_STRING fullPath, OUT PWCHAR filePath, OUT PWCHAR fileName)
{
	PWCHAR pathBuffer = fullPath->Buffer;
	int len = wcslen(pathBuffer) - 1;
	int start = 0;
	int i = start, j = len;

	// �ļ���
	while (pathBuffer[i] != '\\')
	{
		fileName[j] = pathBuffer[i];
		i++;
		j--;
	}

	// ��ת(��֪��Ϊɶ)
	WCHAR tmp;
	int k = 0;
	int lenght = wcslen(fileName);
	for (j = lenght - 1; k < j; k++, j--)
	{
		tmp = fileName[k];
		fileName[k] = fileName[j];
		fileName[j] = tmp;
	}

	// �ļ���·��
	j = 0;
	for (i = 0; i < len - lenght; i++)
	{
		filePath[j] = pathBuffer[i];
		j++;
	}
}

// ���ַ�����ɾ�������ַ��Ӵ� ˫ָ��ʵ��
VOID RtlDelSubStr(PWCHAR str, const PWCHAR subStr)
{
	PWCHAR readPos = str;
	PWCHAR writePos = str;
	ULONG chLen = wcslen(subStr);

	while (*readPos != L'\0') 
	{
		if (wcsncmp(readPos, subStr, chLen) == 0) 
		{
			// �����Ӵ�
			readPos += chLen;	  
		}
		else 
		{
			// �����ַ�
			*writePos = *readPos; 
			++writePos;
			++readPos;
		}
	}
	*writePos = L'\0';
	return;
}

// �õ�ϵͳ�汾��Ϣ
BOOLEAN RtlGetVersionInfo(OUT RTL_OSVERSIONINFOEXW* info)
{
	RtlZeroMemory(info, sizeof(info));
	info->dwOSVersionInfoSize = sizeof(info);
	return NT_SUCCESS(RtlGetVersion((RTL_OSVERSIONINFOW*)(info)));
}

// �õ������ϵͳ�汾
OS_VERSION RtlGetOsVersion()
{
	RTL_OSVERSIONINFOEXW info;
	if (!RtlGetVersionInfo(&info)) return OS_UNKNOWN;

	if (info.dwMajorVersion == 5)
	{
		if (info.dwMinorVersion == 1)
		{
			if (info.wServicePackMajor == 1) return OS_WINXPSP1;
			if (info.wServicePackMajor == 2) return OS_WINXPSP2;
			if (info.wServicePackMajor == 3) return OS_WINXPSP3;
			return OS_WINXP;
		}
		if (info.dwMinorVersion == 2)
		{
			if (info.wServicePackMajor == 1) return OS_WIN2003SP1;
			if (info.wServicePackMajor == 2) return OS_WIN2003SP2;
			return OS_WIN2003;
		}
	}
	else if (info.dwMajorVersion == 6)
	{
		if (info.dwMinorVersion == 0)
		{
			if (info.wServicePackMajor == 1) return OS_WINVISTASP1;
			if (info.wServicePackMajor == 2) return OS_WINVISTASP2;
			return OS_WINVISTA;
		}
		if (info.dwMinorVersion == 1)
		{
			if (info.wServicePackMajor == 1) return OS_WIN7SP1;
			return OS_WIN7;
		}
		if (info.dwMinorVersion == 2)
		{
			return OS_WIN8;
		}
		if (info.dwMinorVersion == 3)
		{
			return OS_WIN81;
		}
	}
	else if (info.dwMajorVersion == 10)
	{
		if (info.dwBuildNumber == 10240) return OS_WIN10_1507;
		if (info.dwBuildNumber == 10586) return OS_WIN10_1511;
		if (info.dwBuildNumber == 14393) return OS_WIN10_1607;
		if (info.dwBuildNumber == 15063) return OS_WIN10_1703;
		if (info.dwBuildNumber == 16299) return OS_WIN10_1709;
		if (info.dwBuildNumber == 17134) return OS_WIN10_1803;
		if (info.dwBuildNumber == 17763) return OS_WIN10_1809;
		if (info.dwBuildNumber == 18362) return OS_WIN10_1903;
		if (info.dwBuildNumber == 18363) return OS_WIN10_1909;
		if (info.dwBuildNumber == 19041) return OS_WIN10_2004;
		if (info.dwBuildNumber == 19042) return OS_WIN10_20H2;
		if (info.dwBuildNumber == 19043) return OS_WIN10_21H1;
		if (info.dwBuildNumber == 19044) return OS_WIN10_21H2;
		if (info.dwBuildNumber == 19045) return OS_WIN10_22H2;
		if (info.dwBuildNumber == 22000) return OS_WIN11_21H2;
	}
	return OS_UNKNOWN;
}

// ���ص���֤��ע��ص���API�����MmVerifyCallbackFunction���������ǩ��
ULONG RtlByPassCallBackVerify(PVOID ldr)
{
	ULONG originFlags = ((PKLDR_DATA_TABLE_ENTRY64)ldr)->Flags;
	((PKLDR_DATA_TABLE_ENTRY64)ldr)->Flags |= 0x20;
	return originFlags;
}

// �ָ��ص���֤
VOID RtlResetCallBackVerify(PVOID ldr, ULONG oldFlags)
{
	((PKLDR_DATA_TABLE_ENTRY64)ldr)->Flags = oldFlags;
}

// �õ��ں�PE�ڵĿ�ʼλ�úͽ���λ��
NTSTATUS RtlFindImageSection(IN PVOID imageBase, IN CHAR* sectionName, OUT PVOID* sectionStart, OUT PVOID* sectionEnd)
{
	PIMAGE_NT_HEADERS ntHeaders = NULL;
	// ��Ϊ��ֻ�����ں��ļ���PE�ڲ���,Ϊ64λ����ϵͳ
	PIMAGE_SECTION_HEADER64 ntSection = NULL;
	PIMAGE_SECTION_HEADER64 findSection = NULL;
	ULONG index = 0;
	ULONG nameLen = 0;
	if (ntHeaders = RtlImageNtHeader(imageBase))
	{
		if (ntSection = (PIMAGE_SECTION_HEADER64)IMAGE_FIRST_SECTION(ntHeaders))
		{
			nameLen = min(strlen(sectionName), 8);
			for (index = 0; index < ntHeaders->FileHeader.NumberOfSections; index++)
			{
				if (_strnicmp((char*)ntSection[index].Name, sectionName, nameLen) == 0)
				{
					findSection = &ntSection[index];
					break;
				}
			}
			if (findSection != NULL)
			{
				*sectionStart = (PVOID)((ULONG64)imageBase + findSection->VirtualAddress);
				*sectionEnd = (PVOID)((ULONG64)*sectionStart + max(findSection->SizeOfRawData, findSection->Misc.VirtualSize));
				return STATUS_SUCCESS;
			}
		}
	}
	return STATUS_INVALID_IMAGE_FORMAT;
}

// ����������
PVOID RtlScanFeatureCode(PVOID begin, PVOID end, CHAR* featureCode)
{

	return NULL;
}

// �߳��ӳټ���  Sleep
NTSTATUS KeSleep(ULONG64 TimeOut)
{
	LARGE_INTEGER delayTime = { 0 };
	delayTime.QuadPart = -10 * 1000;
	delayTime.QuadPart *= TimeOut;
	return KeDelayExecutionThread(KernelMode, FALSE, &delayTime);
}

// ͨ��pid�ж��Ƿ���32λ����
BOOLEAN PsIsWow64Process(HANDLE processId)
{
	NTSTATUS status = STATUS_SUCCESS;
	PEPROCESS eProcess = NULL;
	status = PsLookupProcessByProcessId(processId, &eProcess);
	if (NT_SUCCESS(status))
	{
		if (PsGetProcessWow64Process(eProcess) == NULL)
		{
			return FALSE;
		}
		ObReferenceObject(eProcess);
	}
	return TRUE;
}

// ��д����
ULONG64 wpoff()
{
	_disable();
	ULONG64 mcr0 = __readcr0();
	__writecr0(mcr0 & (~0x10000));
	return mcr0;
}

// ��д����
VOID wpon(ULONG64 mcr0)
{
	__writecr0(mcr0);
	_enable();
}

// mdlӳ���ַ
PVOID MdlMapMemory(OUT PMDL* mdl, IN PVOID tagAddress, IN SIZE_T mapSize, IN MODE preMode)
{
	PMDL pMdl = IoAllocateMdl(tagAddress, mapSize, FALSE, FALSE, NULL);
	if (pMdl == NULL)
	{
		return NULL;
	}
	PVOID mapAddr = NULL;
	BOOLEAN isLock = FALSE;
	__try
	{
		MmProbeAndLockPages(pMdl, preMode, IoReadAccess);
		isLock = TRUE;
		mapAddr = MmMapLockedPagesSpecifyCache(pMdl, preMode, MmCached, NULL, FALSE, NormalPagePriority);
	}
	__except(1)
	{
		if (isLock)
		{
			MmUnlockPages(pMdl);
		}
		IoFreeMdl(pMdl);
		return NULL;
	}
	*mdl = pMdl;
	return mapAddr;
}

// mdlȡ��ӳ��
VOID MdlUnMapMemory(IN PMDL mdl, IN PVOID mapBase)
{
	if (mdl == NULL) return;
	__try
	{
		MmUnmapLockedPages(mapBase, mdl);
		MmUnlockPages(mdl);
		IoFreeMdl(mdl);
	}
	__except (1)
	{
		return;
	}
}

// MmCopyVirtualMemory�ӿڷ�װһ��,��̬��ȡ��ַ(��iat hook��api)
NTSTATUS CT_MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode, PSIZE_T ReturnSize)
{
	typedef NTSTATUS(NTAPI* MmCopyVirtualMemoryPfn)(PEPROCESS, PVOID, PEPROCESS, PVOID, SIZE_T, KPROCESSOR_MODE, PSIZE_T);
	static MmCopyVirtualMemoryPfn CopyPfn = NULL;
	if (!CopyPfn)
	{
		WCHAR funcNameStr[] = { 'M','m','C','o','p','y','V','i','r','t','u','a','l','M', 'e','m','o','r','y', 0, 0 };
		UNICODE_STRING uniFuncNameStr = { 0 };
		RtlInitUnicodeString(&uniFuncNameStr, funcNameStr);
		CopyPfn = MmGetSystemRoutineAddress(&uniFuncNameStr);
	}
	if (CopyPfn)
	{
		return CopyPfn(SourceProcess, SourceAddress, TargetProcess, TargetAddress, BufferSize, PreviousMode, ReturnSize);
	}
	return STATUS_NOT_IMPLEMENTED;
}

// �޸��ڴ�����
NTSTATUS CT_ZwProtectVirtualMemory(IN PVOID address, IN SIZE_T spaceSize, IN ULONG newProtect, OUT ULONG* oldProtect)
{
	NTSTATUS stus = STATUS_UNSUCCESSFUL;
	typedef NTSTATUS(NTAPI* ZwProtectVirtualMemoryPfn)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
	static ZwProtectVirtualMemoryPfn ZwProtectVirtualMemory = NULL;
	if (ZwProtectVirtualMemory == NULL)
	{
		WCHAR zwQuerySec[] = { 'Z','w','Q','u','e','r','y','S','e','c','t','i','o','n', 0, 0 };
		WCHAR zwProtectVir[] = { 'Z','w','P','r','o','t','e','c','t','V','i','r','t','u','a','l','M','e','m','o','r','y', 0, 0 };
		UNICODE_STRING uZwQuerySec = { 0 };
		UNICODE_STRING uProtectVir = { 0 };
		RtlInitUnicodeString(&uZwQuerySec, zwQuerySec);
		RtlInitUnicodeString(&uProtectVir, zwProtectVir);

		ZwProtectVirtualMemory = MmGetSystemRoutineAddress(&uProtectVir);
		// win7 win8 win8.1δ����Protect����,ͨ��ƫ������
		UCHAR* ZwQuerySection = (UCHAR*)MmGetSystemRoutineAddress(&uZwQuerySec);
		if (ZwQuerySection)
		{
			for (size_t i = 5; i < 100; i++)
			{
				// i=5����ֱ�Ӷ�λ��ZwQuerySection����ͷ
				// 48 8B C4 : Mov Rax,Rsp
				if (*(ZwQuerySection - i - 0) == 0x48 && *(ZwQuerySection - i + 1) == 0x8B && *(ZwQuerySection - i + 2) == 0xC4)
				{
					(*(PVOID*)&ZwProtectVirtualMemory) = (ZwQuerySection - i - 0);
					break;
				}
			}
		}
	}
	if (ZwProtectVirtualMemory != NULL)
	{
		// ���õ�ǰ�̵߳���ǰģʽ���û�ģʽ(�ܹ���try����)
		ULONG offsetPreMode = 0;
		PUCHAR retPos = (PUCHAR)ExGetPreviousMode;
		while (*retPos != 0xC3)  // ret
		{
			retPos++;
		}
		offsetPreMode = *(ULONG*)(retPos - 4);
		KPROCESSOR_MODE origiMode = ExGetPreviousMode();
		*(KPROCESSOR_MODE*)((UCHAR*)PsGetCurrentThread() + offsetPreMode) = UserMode;

		// �޸�ҳ������
		ULONG tmpProtect = 0;
		SIZE_T size = spaceSize;
		PVOID pageStart = (PVOID)(((ULONG64)address >> 12) << 12);
		__try
		{
			// ����ֱ�Ӵ�&address,��ΪAPI�ڲ���ı�address��ֵΪҳ�濪ʼλ��,�������޸�addressֵ
			stus = ZwProtectVirtualMemory(NtCurrentProcess(), &pageStart, &size, newProtect, &tmpProtect);
			if (NT_SUCCESS(stus) && tmpProtect != 0)
			{
				// tmpProtect��һ�ּ���д����
				*oldProtect = tmpProtect;
			}
		}
		__except (1) 
		{ 
			stus = STATUS_ACCESS_VIOLATION; 
		}

		// ��ԭ��ǰ�̵߳���ǰģʽ
		*(KPROCESSOR_MODE*)((UCHAR*)PsGetCurrentThread() + offsetPreMode) = origiMode;
	}
	return stus;
}