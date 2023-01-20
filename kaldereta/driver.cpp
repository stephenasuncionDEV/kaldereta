#include "sdk.h"

NTSTATUS DriverEntry(DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrintEx(0, 0, "Kaldereta: Version 0.0.4\n");

	if (Driver::Initialize() != STATUS_SUCCESS) {
		DbgPrintEx(0, 0, "Kaldereta: Driver Failed\n");
	}

	return STATUS_UNSUCCESSFUL;
}