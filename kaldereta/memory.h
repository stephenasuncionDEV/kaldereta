#pragma once
#include "process.h"

namespace memory {
	template <typename T = PVOID>
	T allocate(SIZE_T size) {
		return reinterpret_cast<T>(ExAllocatePool(NonPagedPool, size));
	}

	VOID free(PVOID buffer) {
		ExFreePool(buffer);
	}

	BOOLEAN copy(PVOID destination, PVOID src, SIZE_T size) {
		SIZE_T bytesRead{ 0 };
		return NT_SUCCESS(MmCopyVirtualMemory(IoGetCurrentProcess(),
			src,
			IoGetCurrentProcess(),
			destination,
			size,
			KernelMode,
			&bytesRead)) && bytesRead == size;
	}

	bool readBuffer(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ process::getProcess((DWORD)data->procId) };
		NTSTATUS status{ STATUS_SUCCESS };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		status = MmCopyVirtualMemory(eProcess,
			data->address,
			IoGetCurrentProcess(),
			data->buffer,
			data->size,
			KernelMode,
			&data->returnLength);

		if (!NT_SUCCESS(status)) {
			DbgPrintEx(0, 0, "Kaldereta: [ReadBuffer] Failed, Code: %08X\n", status);
			return false;
		}
		return true;
	}

	bool writeBuffer(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ process::getProcess((DWORD)data->procId) };
		NTSTATUS status{ STATUS_SUCCESS };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KAPC_STATE state;
		KeStackAttachProcess((PKPROCESS)eProcess, &state);

		MEMORY_BASIC_INFORMATION info;

		status = ZwQueryVirtualMemory(ZwCurrentProcess(), data->address, MemoryBasicInformation, &info, sizeof(info), NULL);

		if (!NT_SUCCESS(status)) {
			DbgPrintEx(0, 0, "Kaldereta: [WriteBuffer] Failed, Code: %08X\n", status);
			KeUnstackDetachProcess(&state);
			return false;
		}

		if (((uintptr_t)info.BaseAddress + info.RegionSize) < ((uintptr_t)data->address + data->size))
		{
			KeUnstackDetachProcess(&state);
			return false;
		}

		if (!(info.State & MEM_COMMIT) || (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
		{
			KeUnstackDetachProcess(&state);
			return false;
		}

		if ((info.Protect & PAGE_EXECUTE_READWRITE) || (info.Protect & PAGE_EXECUTE_WRITECOPY) || (info.Protect & PAGE_READWRITE) || (info.Protect & PAGE_WRITECOPY))
		{
			RtlCopyMemory((void*)data->address, data->buffer, data->size);
		}

		KeUnstackDetachProcess(&state);
		return true;
	}

	NTSTATUS allocateVirtualMemory(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ process::getProcess((DWORD)data->procId) };
		KAPC_STATE Apc{ NULL };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwAllocateVirtualMemory(ZwCurrentProcess(),
							 &data->baseAddress,
							 NULL,
							 &data->size,
							 data->allocType,
							 data->protect) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS freeVirtualMemory(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ process::getProcess((DWORD)data->procId) };
		KAPC_STATE Apc{ NULL };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwFreeVirtualMemory(ZwCurrentProcess(),
							 &data->baseAddress,
							 &data->size,
							 data->freeType) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS protectVirtualMemory(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ process::getProcess((DWORD)data->procId) };
		KAPC_STATE Apc{ NULL };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwProtectVirtualMemory(ZwCurrentProcess(),
							&data->baseAddress,
							&data->size,
							data->protect,
							&data->oldProtect) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS queryVirtualMemory(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ process::getProcess((DWORD)data->procId) };
		NTSTATUS Status{ STATUS_SUCCESS };
		KAPC_STATE Apc{ 0 };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		Status = ZwQueryVirtualMemory(ZwCurrentProcess(),
			data->baseAddress,
			MemoryBasicInformation,
			&data->mbi,
			sizeof(data->mbi),
			&data->returnLength);

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	PVOID getModuleBase(const char* moduleName)
	{
		ULONG bytes = 0;
		NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

		if (!bytes) {
			return NULL;
		}

		PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x54697465);

		status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

		if (!NT_SUCCESS(status)) {
			return NULL;
		}

		PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
		PVOID moduleBase = 0, moduleSize = 0;

		for (ULONG i = 0; i < modules->NumberOfModules; i++)
		{
			if (strcmp((char*)module[i].FullPathName, moduleName) == 0)
			{
				moduleBase = module[i].ImageBase;
				moduleSize = (PVOID)module[i].ImageBase;
				break;
			}
		}

		if (modules) {
			ExFreePoolWithTag(modules, NULL);
		}

		if (moduleBase <= NULL) {
			return NULL;
		}

		return moduleBase;
	}

	PVOID getModuleExport(const char* moduleName, LPCSTR routineName)
	{
		PVOID lpModule = getModuleBase(moduleName);

		if (!lpModule)
			return NULL;

		return RtlFindExportedRoutineByName(lpModule, routineName);
	}

	NTSTATUS initMouse(PMOUSE_OBJECT mouse_obj)
	{
		UNICODE_STRING class_string;
		RtlInitUnicodeString(&class_string, L"\\Driver\\MouClass");

		PDRIVER_OBJECT class_driver_object = NULL;
		NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
		if (!NT_SUCCESS(status)) {
			DbgPrintEx(0, 0, "Kaldereta: [Mouse] Failed Initializing Mouse 0x1, Code: %08X\n", status);
			return status;
		}

		UNICODE_STRING hid_string;
		RtlInitUnicodeString(&hid_string, L"\\Driver\\MouHID");

		PDRIVER_OBJECT hid_driver_object = NULL;
		status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
		if (!NT_SUCCESS(status))
		{
			DbgPrintEx(0, 0, "Kaldereta: [Mouse] Failed Initializing Mouse 0x2, Code: %08X\n", status);
			if (class_driver_object) { ObDereferenceObject(class_driver_object); }
			return status;
		}

		PVOID class_driver_base = NULL;

		PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
		while (hid_device_object && !mouse_obj->service_callback)
		{
			PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;
			while (class_device_object && !mouse_obj->service_callback)
			{
				if (!class_device_object->NextDevice && !mouse_obj->mouse_device)
				{
					mouse_obj->mouse_device = class_device_object;
				}

				PULONG_PTR device_extension = (PULONG_PTR)hid_device_object->DeviceExtension;
				ULONG_PTR device_ext_size = ((ULONG_PTR)hid_device_object->DeviceObjectExtension - (ULONG_PTR)hid_device_object->DeviceExtension) / 4;
				class_driver_base = class_driver_object->DriverStart;
				for (ULONG_PTR i = 0; i < device_ext_size; i++)
				{
					if (device_extension[i] == (ULONG_PTR)class_device_object && device_extension[i + 1] > (ULONG_PTR)class_driver_object)
					{
						mouse_obj->service_callback = (MouseClassServiceCallback)(device_extension[i + 1]);
						break;
					}
				}
				class_device_object = class_device_object->NextDevice;
			}
			hid_device_object = hid_device_object->AttachedDevice;
		}

		if (!mouse_obj->mouse_device)
		{
			PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
			while (target_device_object)
			{
				if (!target_device_object->NextDevice)
				{
					mouse_obj->mouse_device = target_device_object;
					break;
				}
				target_device_object = target_device_object->NextDevice;
			}
		}

		ObDereferenceObject(class_driver_object);
		ObDereferenceObject(hid_driver_object);

		return STATUS_SUCCESS;
	}

	NTSTATUS initKeyboard(PKEYBOARD_OBJECT keyboard_obj)
	{
		UNICODE_STRING class_string;
		RtlInitUnicodeString(&class_string, L"\\Driver\\KbdClass");

		PDRIVER_OBJECT class_driver_object = NULL;
		NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
		if (!NT_SUCCESS(status)) {
			DbgPrintEx(0, 0, "Kaldereta: [Keyboard] Failed Initializing Keyboard 0x1, Code: %08X\n", status);
			return status;
		}

		UNICODE_STRING hid_string;
		RtlInitUnicodeString(&hid_string, L"\\Driver\\KbdHID");

		PDRIVER_OBJECT hid_driver_object = NULL;
		status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
		if (!NT_SUCCESS(status))
		{
			DbgPrintEx(0, 0, "Kaldereta: [Keyboard] Failed Initializing Keyboard 0x2, Code: %08X\n", status);
			if (class_driver_object) { ObDereferenceObject(class_driver_object); }
			return status;
		}

		PVOID class_driver_base = NULL;

		PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
		while (hid_device_object && !keyboard_obj->service_callback)
		{
			PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;
			while (class_device_object && !keyboard_obj->service_callback)
			{
				if (!class_device_object->NextDevice && !keyboard_obj->keyboard_device)
				{
					keyboard_obj->keyboard_device = class_device_object;
				}

				PULONG_PTR device_extension = (PULONG_PTR)hid_device_object->DeviceExtension;
				ULONG_PTR device_ext_size = ((ULONG_PTR)hid_device_object->DeviceObjectExtension - (ULONG_PTR)hid_device_object->DeviceExtension) / 4;
				class_driver_base = class_driver_object->DriverStart;
				for (ULONG_PTR i = 0; i < device_ext_size; i++)
				{
					if (device_extension[i] == (ULONG_PTR)class_device_object && device_extension[i + 1] > (ULONG_PTR)class_driver_object)
					{
						keyboard_obj->service_callback = (KeyboardClassServiceCallback)(device_extension[i + 1]);
						break;
					}
				}
				class_device_object = class_device_object->NextDevice;
			}
			hid_device_object = hid_device_object->AttachedDevice;
		}

		if (!keyboard_obj->keyboard_device)
		{
			PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
			while (target_device_object)
			{
				if (!target_device_object->NextDevice)
				{
					keyboard_obj->keyboard_device = target_device_object;
					break;
				}
				target_device_object = target_device_object->NextDevice;
			}
		}

		ObDereferenceObject(class_driver_object);
		ObDereferenceObject(hid_driver_object);

		return STATUS_SUCCESS;
	}

	bool keyboardEvent(KEYBOARD_OBJECT keyboard_obj, SANGKAP_SA_KALDERETA* data) {
		ULONG input_data;
		KIRQL irql;
		KEYBOARD_INPUT_DATA kid = { 0 };

		kid.MakeCode = data->keyCode;
		kid.Flags = data->keyFlags;

		KeRaiseIrql(DISPATCH_LEVEL, &irql);
		keyboard_obj.service_callback(keyboard_obj.keyboard_device, &kid, (PKEYBOARD_INPUT_DATA)&kid + 1, &input_data);
		KeLowerIrql(irql);

		return true;
	}

	bool mouseEvent(MOUSE_OBJECT mouse_obj, SANGKAP_SA_KALDERETA* data)
	{
		ULONG input_data;
		KIRQL irql;
		MOUSE_INPUT_DATA mid = { 0 };

		mid.LastX = data->mouseX;
		mid.LastY = data->mouseY;
		mid.ButtonFlags = data->buttonFlags;

		KeRaiseIrql(DISPATCH_LEVEL, &irql);
		mouse_obj.service_callback(mouse_obj.mouse_device, &mid, (PMOUSE_INPUT_DATA)&mid + 1, &input_data);
		KeLowerIrql(irql);

		return true;
	}

	NTSTATUS setCursorPos(SANGKAP_SA_KALDERETA* data)
	{
		NTSTATUS status = STATUS_SUCCESS;
		PEPROCESS winlogonProcess = nullptr;

		ANSI_STRING processName;
		UNICODE_STRING processUnicodeName = { 0 };
		RtlInitAnsiString(&processName, "winlogon.exe");
		RtlAnsiStringToUnicodeString(&processUnicodeName, &processName, TRUE);
		ULONG pid = process::getProcessId(processUnicodeName);
		RtlFreeUnicodeString(&processUnicodeName);

		if (pid != 0) {
			if (NT_SUCCESS(PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(pid), &winlogonProcess))) {
				KAPC_STATE apc = { 0 };
				KeStackAttachProcess(winlogonProcess, &apc);
				uint64_t gptCursorAsyncAddr = reinterpret_cast<uint64_t>(getModuleExport("\\SystemRoot\\System32\\win32kbase.sys", "gptCursorAsync"));
				if (gptCursorAsyncAddr) {
					POINT cursorPos = *(POINT*)(gptCursorAsyncAddr);
					cursorPos.x = data->mouseX;
					cursorPos.y = data->mouseY;
					*(POINT*)(gptCursorAsyncAddr) = cursorPos;
				}
				else {
					status = STATUS_INVALID_PARAMETER;
				}
				KeUnstackDetachProcess(&apc);
			}
		}
		else {
			status = STATUS_INVALID_PARAMETER;
		}

		return status;
	}
}