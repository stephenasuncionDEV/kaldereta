#include "sdk.h"

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	DbgPrintEx(0, 0, "Kaldereta: Version 0.0.2\n");
	DbgPrintEx(0, 0, "Kaldereta: Driver Loaded\n");

	return Driver::Initialize();
}