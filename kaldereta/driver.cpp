#include <ntifs.h>
#include <windef.h>
#include "process.h"
#include "memory.h"

MOUSE_OBJECT mouse_obj = { 0 };
KEYBOARD_OBJECT keyboard_obj = { 0 };

NTSTATUS Control(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);

	irp->IoStatus.Information = sizeof(SANGKAP_SA_KALDERETA);

	static PEPROCESS s_target_process;;

	auto stack = IoGetCurrentIrpStackLocation(irp);
	auto data = (SANGKAP_SA_KALDERETA*)irp->AssociatedIrp.SystemBuffer;

	if (!mouse_obj.service_callback || !mouse_obj.mouse_device) {
		memory::initMouse(&mouse_obj);
	}

	if (!keyboard_obj.service_callback || !keyboard_obj.keyboard_device) {
		memory::initKeyboard(&keyboard_obj);
	}

	if (stack) {
		if (data && sizeof(*data) >= sizeof(SANGKAP_SA_KALDERETA)) {
			const auto ctl_code = stack->Parameters.DeviceIoControl.IoControlCode;

			if (ctl_code == BaseRequest)
			{
				process::getBaseAddress(data);
			}

			else if (ctl_code == SizeRequest)
			{
				process::getMainModuleSize(data);
			}

			else if (ctl_code == PebRequest)
			{
				process::getPeb(data);
			}

			else if (ctl_code == QIPRequest)
			{
				process::queryInformation(data);
			}

			else if (ctl_code == ReadRequest)
			{
				memory::readBuffer(data);
			}

			else if (ctl_code == WriteRequest)
			{
				memory::writeBuffer(data);
			}

			else if (ctl_code == AVMRequest)
			{
				memory::allocateVirtualMemory(data);
			}

			else if (ctl_code == FVMRequest)
			{
				memory::freeVirtualMemory(data);
			}

			else if (ctl_code == PVMRequest)
			{
				memory::protectVirtualMemory(data);
			}

			else if (ctl_code == QVMRequest)
			{
				memory::queryVirtualMemory(data);
			}

			else if (ctl_code == ModuleRequest)
			{
				process::getModuleInfo(data);
			}

			else if (ctl_code == IndexRequest)
			{
				process::getModuleInfoByIndex(data);
			}

			else if (ctl_code == MouseRequest)
			{
				memory::mouseEvent(mouse_obj, data);
			}

			else if (ctl_code == KeyboardRequest)
			{
				memory::keyboardEvent(keyboard_obj, data);
			}

			else if (ctl_code == SetCursorRequest)
			{
				memory::setCursorPos(data);
			}
		}
	}

	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Create(PDEVICE_OBJECT pDeviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Close(PDEVICE_OBJECT pDeviceObject, PIRP irp)
{
	UNREFERENCED_PARAMETER(pDeviceObject);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Unsupport(PDEVICE_OBJECT device_obj, PIRP irp) {
	UNREFERENCED_PARAMETER(device_obj);
	irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return irp->IoStatus.Status;
}

NTSTATUS Main(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
	UNREFERENCED_PARAMETER(registery_path);

	UNICODE_STRING dev_name, sym_link;
	PDEVICE_OBJECT dev_obj;

	RtlInitUnicodeString(&dev_name, L"\\Device\\Kaldereta");
	auto status = IoCreateDevice(driver_obj, 0, &dev_name, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &dev_obj);
	if (status != STATUS_SUCCESS) return status;

	RtlInitUnicodeString(&sym_link, L"\\DosDevices\\Kaldereta");
	status = IoCreateSymbolicLink(&sym_link, &dev_name);
	if (status != STATUS_SUCCESS) return status;

	SetFlag(dev_obj->Flags, DO_BUFFERED_IO);

	for (int t = 0; t <= IRP_MJ_MAXIMUM_FUNCTION; t++)
	{
		driver_obj->MajorFunction[t] = Unsupport;
	}

	driver_obj->MajorFunction[IRP_MJ_CREATE] = Create;
	driver_obj->MajorFunction[IRP_MJ_CLOSE] = Close;
	driver_obj->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Control;
	driver_obj->DriverUnload = NULL;

	ClearFlag(dev_obj->Flags, DO_DEVICE_INITIALIZING);

	return status;
}

extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT driver_obj, PUNICODE_STRING registery_path) {
	UNREFERENCED_PARAMETER(driver_obj);
	UNREFERENCED_PARAMETER(registery_path);

	DbgPrintEx(0, 0, "Kaldereta: IOCTL Version\n");

	UNICODE_STRING drv;
	RtlInitUnicodeString(&drv, L"\\Driver\\Kaldereta");
	IoCreateDriver(&drv, &Main);

	return STATUS_SUCCESS;
}