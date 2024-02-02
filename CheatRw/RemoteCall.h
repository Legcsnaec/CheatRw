#pragma once
#include <ntifs.h>
#include "Unrevealed.h"
#include "Comm.h"

NTSTATUS RemoteCall(HANDLE pid, PVOID shellCode, ULONG shellcodeSize);
