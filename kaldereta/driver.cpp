#include "sdk.h"

NTSTATUS DriverEntry(DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath) {
	DbgPrintEx(0, 0, "Kaldereta: Version 0.0.3\n");
	DbgPrintEx(0, 0, "Kaldereta: Driver Loaded\n");

	return Driver::Initialize();
}