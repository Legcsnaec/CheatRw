#include <ntifs.h>
#include "GetModule.h"
#include "Comm.h"

VOID DriverUnload(PDRIVER_OBJECT pDrv)
{
	UNREFERENCED_PARAMETER(pDrv);
	CommUninitialize();
	DbgPrint("unload ... \n\r");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDrv, PUNICODE_STRING pReg)
{
	UNREFERENCED_PARAMETER(pReg);

	NTSTATUS stat = STATUS_SUCCESS;

	stat = CommInitialize();

	pDrv->DriverUnload = DriverUnload;

	return stat;
}
