#pragma once
#include <ntifs.h>
#include "Unrevealed.h"
#include "Comm.h"

NTSTATUS ReadMemory(HANDLE pid, PVOID tagAddress, PVOID readBuffer, SIZE_T readSize);