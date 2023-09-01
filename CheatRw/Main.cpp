#include <ntifs.h>
#include "Module.h"

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pReg);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pReg)
{
	pDrv->DriverUnload = [](PDRIVER_OBJECT pDrv)->VOID {
		DbgPrint("unload ... \n\r");
	};
	
	GetModuleR3((HANDLE)5380, "ntdll.dll", NULL);

	return STATUS_SUCCESS;
}
