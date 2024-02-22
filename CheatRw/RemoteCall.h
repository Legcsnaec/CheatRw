#pragma once
#include <ntifs.h>
#include "Unrevealed.h"
#include "Comm.h"

NTSTATUS RtlRemoteCall(HANDLE pid, PVOID shellCodePtr, ULONG64 shellCodeSize);