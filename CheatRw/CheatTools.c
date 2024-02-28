#include "CheatTools.h"
#include "Unrevealed.h"
#include "SearchCode.h"

// 64\32λ�汾,���ݻ�ַ�����Ƶõ�������ַ,���������
PVOID MmGetSystemRoutineAddressEx(ULONG64 modBase, CHAR* searchFnName)
{
	if (modBase == 0 || searchFnName == NULL)  return NULL;

	ULONG64 funcAddr = 0;
	do
	{
		PIMAGE_OPTIONAL_HEADER32 optionHeader32 = NULL;
		PIMAGE_OPTIONAL_HEADER64 optionHeader64 = NULL;
		PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)modBase;
		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)(modBase + dosHeader->e_lfanew);

		if (ntHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) optionHeader64 = &ntHeaders->OptionalHeader;
		else optionHeader32 = &ntHeaders->OptionalHeader;

		ULONG virtualAddress = 0;
		virtualAddress = (optionHeader64 != NULL ? optionHeader64->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress :
			optionHeader32->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
		if (virtualAddress == 0)
		{
			break;
		}

		PIMAGE_EXPORT_DIRECTORY exportDir = (PIMAGE_EXPORT_DIRECTORY)(modBase + virtualAddress);
		PULONG fnsAddress = (PULONG)(modBase + exportDir->AddressOfFunctions);
		PULONG namesAddress = (PULONG)(modBase + exportDir->AddressOfNames);
		PUSHORT ordinalsAddress = (PUSHORT)(modBase + exportDir->AddressOfNameOrdinals);

		ULONG funcOrdinal, i;
		for (ULONG i = 0; i < exportDir->NumberOfNames; i++)
		{
			char* funcName = (char*)(modBase + namesAddress[i]);
			if (modBase < MmUserProbeAddress)
			{
				__try
				{
					if (_strnicmp(searchFnName, funcName, strlen(searchFnName)) == 0)
					{
						if (funcName[strlen(searchFnName)] == 0)
						{
							funcAddr = modBase + fnsAddress[ordinalsAddress[i]];
							break;
						}
					}
				}
				__except (1)
				{
					continue;
				}
			}
			else
			{
				if (MmIsAddressValid(funcName) && _strnicmp(searchFnName, funcName, strlen(searchFnName)) == 0)
				{
					if (funcName[strlen(searchFnName)] == 0)
					{
						// �����+ordinals = ��ʵ���
						funcOrdinal = exportDir->Base + ordinalsAddress[i] - 1;
						funcAddr = modBase + fnsAddress[funcOrdinal];
						break;
					}
				}
			}
		}
	} while (0);
	return (PVOID)funcAddr;
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

// �ַ���������±�
int RtlStringLastIndexOf(PUNICODE_STRING fullPath, WCHAR ch)
{
	if (fullPath == NULL || fullPath->Buffer == NULL) return -1;

	PWCHAR pathBuffer = fullPath->Buffer;
	int len = wcslen(pathBuffer) - 1;
	for (int j = len; j > 0; j--)
	{
		if (pathBuffer[j] == ch)
		{
			return j;
		}
	}
	return -1;
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
// ������ԭ������ν
ULONG RtlByPassCallBackVerify(PVOID ldr)
{
	if (ldr == NULL || MmIsAddressValid(ldr))
	{
		return 0;
	}
	ULONG originFlags = ((PKLDR_DATA_TABLE_ENTRY64)ldr)->Flags;
	((PKLDR_DATA_TABLE_ENTRY64)ldr)->Flags |= 0x20;
	return originFlags;
}

// �ָ��ص���֤
VOID RtlResetCallBackVerify(PVOID ldr, ULONG oldFlags)
{
	if (ldr == NULL || MmIsAddressValid(ldr)) return;
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
		ObDereferenceObject(eProcess);
	}
	return TRUE;
}

// ��д����
ULONG64 wpoff()
{
	_disable();
	ULONG64 mcr0 = __readcr0();
	__writecr0(mcr0 & (~0x10000));
	_enable();
	return  mcr0;
}

// ��д����
VOID wpon(ULONG64 mcr0)
{
	_disable();
	__writecr0(mcr0);
	_enable();
}

// mdlӳ���ַ
PVOID MdlMapMemory(OUT PMDL* mdl, IN PVOID tagAddress, IN ULONG mapSize, IN MODE preMode)
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

// ͨ���������õ���������
NTSTATUS GetDriverObjectByName(IN PWCH driverName, OUT PDRIVER_OBJECT* driver)
{
	if (driverName == NULL || driver == NULL)
	{
		return STATUS_UNSUCCESSFUL;
	}

	UNICODE_STRING drvNameUnStr = { 0 };
	RtlInitUnicodeString(&drvNameUnStr, driverName);
	PDRIVER_OBJECT drv = NULL;
	NTSTATUS stat = ObReferenceObjectByName(&drvNameUnStr, FILE_ALL_ACCESS, 0, 0, *IoDriverObjectType, KernelMode, NULL, &drv);
	if (NT_SUCCESS(stat))
	{
		*driver = drv;
		ObDereferenceObject(drv);
	}
	return STATUS_SUCCESS;
}

// �õ����̵����߳�
NTSTATUS GetMainThreadByEprocess(IN PEPROCESS eprocess, OUT PETHREAD* ethread)
{
	if (eprocess == NULL || ethread == NULL)
	{
		return STATUS_UNSUCCESSFUL;
	}

	NTSTATUS stat = STATUS_UNSUCCESSFUL;
	KAPC_STATE apcStat = { 0 };
	HANDLE thread = NULL;
	PETHREAD tmpEthread = NULL;

	KeStackAttachProcess(eprocess, &apcStat);
	stat = CT_ZwGetNextThread(NtCurrentProcess(), NULL, THREAD_ALL_ACCESS, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, &thread);
	if (NT_SUCCESS(stat))
	{
		// ��ȡ�̶߳���
		stat = ObReferenceObjectByHandle(thread, THREAD_ALL_ACCESS, *PsThreadType, KernelMode, &tmpEthread, NULL);
		NtClose(thread);
	}
	KeUnstackDetachProcess(&apcStat);

	*ethread = tmpEthread;
	return stat;
}

//  ---------------------  �ӿ����  --------------------- 
// 
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
NTSTATUS CT_ZwProtectVirtualMemory(IN PVOID Address, IN SIZE_T SpaceSize, IN ULONG NewProtect, OUT ULONG* OldProtect)
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
		SIZE_T size = SpaceSize;
		PVOID pageStart = (PVOID)(((ULONG64)Address >> 12) << 12);
		__try
		{
			// ����ֱ�Ӵ�&address,��ΪAPI�ڲ���ı�address��ֵΪҳ�濪ʼλ��,�������޸�addressֵ
			stus = ZwProtectVirtualMemory(NtCurrentProcess(), &pageStart, &size, NewProtect, &tmpProtect);
			if (NT_SUCCESS(stus) && tmpProtect != 0)
			{
				// tmpProtect��һ�ּ���д����
				*OldProtect = tmpProtect;
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

// ��ǩ����֤�Ļص�ע��
NTSTATUS CT_ObRegisterCallbacks(IN POB_CALLBACK_REGISTRATION CallbackRegistration, OUT PVOID* RegistrationHandle)
{
	PCHAR MmVerifyPfn = NULL;
	RTL_OSVERSIONINFOW version = { 0 };
	NTSTATUS stat = RtlGetVersion(&version);
	if (!NT_SUCCESS(stat))
	{
		return stat;
	}
	stat = STATUS_UNSUCCESSFUL;

	// �ҵ���Ӧϵͳ��MmVerifyFun��ַ
	PUCHAR ObRegisterPfn = (PUCHAR)ObRegisterCallbacks;
	if (version.dwBuildNumber == 7600 || version.dwBuildNumber == 7601)
	{
		for (int i = 0; i < 0x500; i++)
		{
			if (ObRegisterPfn[i] == 0x74 && ObRegisterPfn[i + 2] == 0xe8 && ObRegisterPfn[i + 7] == 0x3b && ObRegisterPfn[i + 8] == 0xc3)
			{
				LARGE_INTEGER larger;
				larger.QuadPart = ObRegisterPfn + i + 7;
				larger.LowPart += *(PULONG)(ObRegisterPfn + i + 3);
				MmVerifyPfn = larger.QuadPart;
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < 0x500; i++)
		{
			// ����win10ϵͳ����ͨ��,�Ҳ�����ȥida��Ӧϵͳ��
			// mov xxxx   call xxxx   test eax,eax
			if (ObRegisterPfn[i] == 0xBA && ObRegisterPfn[i + 5] == 0xe8 && ObRegisterPfn[i + 10] == 0x85 && ObRegisterPfn[i + 11] == 0xc0)
			{
				LARGE_INTEGER larger;
				larger.QuadPart = ObRegisterPfn + i + 10;			// ��һ�е�ַ
				larger.LowPart += *(PULONG)(ObRegisterPfn + i + 6);	// offset
				MmVerifyPfn = larger.QuadPart;						// ������ַ
				break;
			}
		}
	}

	// ֱ���޸�Ӳ����򲹶�����֤��ע��ص�
	if (MmVerifyPfn)
	{
		// ֱ��ӳ��һ�������ַ��д
		// (PS���ں��д����ֻ��,ӳ��һ�ݵ�ַĬ�Ͽɶ�д)
		PHYSICAL_ADDRESS phyAddress = MmGetPhysicalAddress(MmVerifyPfn);
		PVOID memMap = MmMapIoSpace(phyAddress, 10, MmNonCached);
		if (memMap)
		{
			UCHAR oldCode[10] = { 0 };
			UCHAR patch[] = { 0xb0,0x1,0xc3 };
			memcpy(oldCode, memMap, 10);
			memcpy(memMap, patch, sizeof(patch));
			stat = ObRegisterCallbacks(CallbackRegistration, RegistrationHandle);
			memcpy(memMap, oldCode, 10);
		}
	}
	return stat;
}

// ��ȡ��ǰ�̵߳���һ�߳�,�����ǰ�߳�ΪNULL���ȡ���߳�
NTSTATUS CT_ZwGetNextThread(IN HANDLE ProcessHandle, IN HANDLE ThreadHandle, IN ACCESS_MASK DesiredAccess, IN ULONG HandleAttributes, IN ULONG Flags, OUT PHANDLE NewThreadHandle)
{
	NTSTATUS stat = STATUS_UNSUCCESSFUL;
	typedef NTSTATUS(NTAPI* ZwGetNextThreadPfn)(HANDLE, HANDLE, ACCESS_MASK, ULONG, ULONG, PHANDLE);
	static ZwGetNextThreadPfn ZwGetNextThreadFunc = NULL;
	if (!ZwGetNextThreadFunc)
	{
		WCHAR zwGetNextThread[] = { 'Z','w','G','e','t','N','e','x','t','T','h','r','e','a', 'd', 0, 0 };
		UNICODE_STRING unZeGetNextThread = { 0 };
		RtlInitUnicodeString(&unZeGetNextThread, zwGetNextThread);
		ZwGetNextThreadFunc = MmGetSystemRoutineAddress(&unZeGetNextThread);
		if(ZwGetNextThreadFunc == NULL)
		{
			// Win7δ����,����������λ
			UNICODE_STRING unName = { 0 };
			RtlInitUnicodeString(&unName, L"ZwGetNotificationResourceManager");
			PUCHAR ZwGetNotificationResourceManagerAddr = (PUCHAR)MmGetSystemRoutineAddress(&unName);
			ZwGetNotificationResourceManagerAddr -= 0x50;
			for (int i = 0; i < 0x30; i++)
			{
				if (ZwGetNotificationResourceManagerAddr[i] == 0x48
					&& ZwGetNotificationResourceManagerAddr[i + 1] == 0x8B
					&& ZwGetNotificationResourceManagerAddr[i + 2] == 0xC4)
				{
					ZwGetNextThreadFunc = ZwGetNotificationResourceManagerAddr + i;
					break;
				}
			}
		}
	}
	if (ZwGetNextThreadFunc)
	{
		stat = ZwGetNextThreadFunc(ProcessHandle, ThreadHandle, DesiredAccess, HandleAttributes, Flags, NewThreadHandle);
	}
	return stat;
}

// �̹߳��� δ����
NTSTATUS CT_PsSuspendThread(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL)
{
	typedef NTSTATUS(NTAPI* PsSuspendThreadPfn)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);
	static PsSuspendThreadPfn PsSuspendThreadFunc = NULL;

	if (!PsSuspendThreadFunc)
	{
		RTL_OSVERSIONINFOW version = { 0 };
		NTSTATUS stat = RtlGetVersion(&version);
		if (!NT_SUCCESS(stat))
		{
			return stat;
		}
		if (version.dwBuildNumber == 7600 || version.dwBuildNumber == 7601)
		{
			// Win7
			PsSuspendThreadFunc = (PsSuspendThreadPfn)SearchCode("ntoskrnl.exe", "PAGE", "4C8BEA488BF133FF897C**65********4C89******6641*******48******0F**488B01", -0x15llu);
		}
		else
		{
			// �����汾Ҳ���ܲ�һ��,�Ҳ�����idaȥ��Ӧϵͳ�汾���� 
			// Win10 19044����
			PsSuspendThreadFunc = (PsSuspendThreadPfn)SearchCode("ntoskrnl.exe", "PAGE", "535657415641574883ec304c8bf2488bf9836424200065488b3425880100004889742470", -0xallu);
		}
	}
	if (PsSuspendThreadFunc)
	{
		return PsSuspendThreadFunc(Thread, PreviousSuspendCount);
	}
	return STATUS_NOT_IMPLEMENTED;
}

// �ָ̻߳� δ����
NTSTATUS CT_PsResumeThread(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL)
{
	typedef NTSTATUS(NTAPI* PsResumeThreadPfn)(IN PETHREAD Thread, OUT PULONG PreviousSuspendCount OPTIONAL);
	static PsResumeThreadPfn PsResumeThreadFunc = NULL;

	if (!PsResumeThreadFunc)
	{
		RTL_OSVERSIONINFOW version = { 0 };
		NTSTATUS stat = RtlGetVersion(&version);
		if (!NT_SUCCESS(stat))
		{
			return stat;
		}
		if (version.dwBuildNumber == 7600 || version.dwBuildNumber == 7601)
		{
			// Win7
			PsResumeThreadFunc = (PsResumeThreadPfn)SearchCode("ntoskrnl.exe", "PAGE", "405348***488BDAE8****4885DB74*890333C048***5BC3", 0);
		}
		else
		{
			// �����汾Ҳ���ܲ�һ��,�Ҳ�����idaȥ��Ӧϵͳ�汾����
			// Win10 19044����
			PsResumeThreadFunc = (PsResumeThreadPfn)SearchCode("ntoskrnl.exe", "PAGE", "4c8b8720020000b800800000418b887c08000085c8", -0x2allu);
		}
	
	}
	if (PsResumeThreadFunc)
	{
		return PsResumeThreadFunc(Thread, PreviousSuspendCount);
	}
	return STATUS_NOT_IMPLEMENTED;
}

// �����û��ռ��ڴ�
NTSTATUS CT_ZwAllocateVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T AllocSize, ULONG AllcType, ULONG Protect)
{
	NTSTATUS Result = STATUS_SUCCESS;
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
		Result = RtlAllocateVirtualMemory(ProcessHandle, BaseAddress, 0, AllocSize, AllcType, Protect);
	}
	__except (1)
	{
		Result = STATUS_UNSUCCESSFUL;
	}
	return Result;
}

// �ͷ��û��ռ��ڴ�
NTSTATUS CT_ZwFreeVirtualMemory(HANDLE ProcessHandle, PVOID* BaseAddress)
{
	SIZE_T size = 0;
	NTSTATUS(NTAPI * RtlFreeVirtualMemory)(HANDLE, PVOID*, PSIZE_T, ULONG);
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
		RtlFreeVirtualMemory(ProcessHandle, BaseAddress, &size, MEM_RELEASE);
	}
	__except (1) { return STATUS_ACCESS_VIOLATION; }
	return STATUS_SUCCESS;
}