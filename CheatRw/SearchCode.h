#pragma once
#include <ntifs.h>

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	HANDLE Section;                 // Not filled in
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct _FindCode
{
	UCHAR code[0x200];
	ULONG len;
	int offset;
	ULONG lastAddressOffset;
}FindCode, * PFindCode;

// ----------------------------------------------------------------------------------

ULONG_PTR SearchNtCode(char* code, int offset);
ULONG_PTR SearchCode(char* moduleName, char* segmentName, char* code, int offset);
