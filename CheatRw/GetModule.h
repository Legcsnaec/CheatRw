#pragma once
#include <ntifs.h>
#include "Unrevealed.h"

// ============================================== º¯ÊýÉùÃ÷ ============================================
ULONG64 GetModuleR3(IN HANDLE pid, IN char* moduleName, OUT PULONG64 sizeImage);

NTSTATUS QueryMemory(IN HANDLE pid, IN ULONG64 baseAddress, OUT PMYMEMORY_BASIC_INFORMATION pInfomation);