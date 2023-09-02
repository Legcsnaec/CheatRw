#include <ntifs.h>
#include "Module.h"
#include "Comm.h"

EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pReg);
NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pReg)
{
	pDrv->DriverUnload = [](PDRIVER_OBJECT pDrv)->VOID {
		UnRegCallBack();
		DbgPrint("unload ... \n\r");
	};
	
	//GetModuleR3((HANDLE)5380, "ntdll.dll", NULL);
	RegisterCallBack();

	return STATUS_SUCCESS;
}
