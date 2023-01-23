#include "hook.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrintEx(0, 0, "Kaldereta: Function Hook Version\n");

	if (hook::callKernelFunc(&hook::hookHandler))
	{
		DbgPrintEx(0, 0, "Kaldereta: Driver Loaded\n");
	}

	return STATUS_SUCCESS;
}