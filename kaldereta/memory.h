#pragma once
#include "process.h"

namespace Memory {

	template <typename T = PVOID>
	T Allocate(SIZE_T Size) {
		return reinterpret_cast<T>(ExAllocatePool(NonPagedPool, Size));
	}

	VOID Free(PVOID Buffer) {
		ExFreePool(Buffer);
	}

	BOOLEAN Copy(PVOID Destination, PVOID Source, SIZE_T Size) {
		SIZE_T BytesRead{ 0 };
		return NT_SUCCESS(MmCopyVirtualMemory(IoGetCurrentProcess(),
			Source,
			IoGetCurrentProcess(),
			Destination,
			Size,
			KernelMode,
			&BytesRead)) && BytesRead == Size;
	}

	NTSTATUS CopyVirtualMemory(OperationData* Data) {
		NTSTATUS Status{ STATUS_SUCCESS };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		if (Data->Memory.Copy.ReadOperation) {
			Status = MmCopyVirtualMemory(eProcess,
				Data->Memory.Copy.Address,
				IoGetCurrentProcess(),
				Data->Memory.Copy.Buffer,
				Data->Memory.Size,
				UserMode,
				&Data->Memory.ReturnLength);
		}
		else {
			Status = MmCopyVirtualMemory(IoGetCurrentProcess(),
				Data->Memory.Copy.Buffer,
				eProcess,
				Data->Memory.Copy.Address,
				Data->Memory.Size,
				UserMode,
				&Data->Memory.ReturnLength);
		}

		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS AllocateVirtualMemory(OperationData* Data) {
		KAPC_STATE Apc{ NULL };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwAllocateVirtualMemory(ZwCurrentProcess(),
							 &Data->Memory.Base,
							 NULL,
							 &Data->Memory.Size,
							 Data->Memory.AllocType,
							 Data->Memory.Protect) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS FreeVirtualMemory(OperationData* Data) {
		KAPC_STATE Apc{ NULL };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwFreeVirtualMemory(ZwCurrentProcess(),
							 &Data->Memory.Base,
							 &Data->Memory.Size,
							 Data->Memory.FreeType) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS ProtectVirtualMemory(OperationData* Data) {
		KAPC_STATE Apc{ NULL };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		NTSTATUS Status{ ZwProtectVirtualMemory(ZwCurrentProcess(),
							&Data->Memory.Base,
							&Data->Memory.Size,
							Data->Memory.Protect,
							&Data->Memory.OldProtect) };

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS QueryVirtualMemory(OperationData* Data) {
		NTSTATUS Status{ STATUS_SUCCESS };
		KAPC_STATE Apc{ 0 };
		PEPROCESS eProcess{ Process::GetProcess(Data->Process.Id) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		Status = ZwQueryVirtualMemory(ZwCurrentProcess(),
			Data->Memory.Base,
			MemoryBasicInformation,
			&Data->Memory.MBI,
			sizeof(Data->Memory.MBI),
			&Data->Memory.ReturnLength);

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}
	PVOID GetModuleBase(const char* moduleName)
	{
		ULONG bytes = 0;
		NTSTATUS status = ZwQuerySystemInformation(SystemModuleInformation, NULL, bytes, &bytes);

		if (!bytes)
			return NULL;

		PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, 0x54697465);

		status = ZwQuerySystemInformation(SystemModuleInformation, modules, bytes, &bytes);

		if (!NT_SUCCESS(status))
			return NULL;

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

		if (modules)
			ExFreePoolWithTag(modules, NULL);

		if (moduleBase <= NULL)
			return NULL;

		return moduleBase;
	}

	PVOID GetModuleExport(const char* moduleName, LPCSTR routineName)
	{
		PVOID lpModule = GetModuleBase(moduleName);

		if (!lpModule)
			return NULL;

		return RtlFindExportedRoutineByName(lpModule, routineName);
	}

	NTSTATUS InitializeMouse(PMOUSE_OBJECT mouse_obj)
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
	NTSTATUS InitializeKeyboard(PKEYBOARD_OBJECT keyboard_obj)
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
	bool keyboardEvent(KEYBOARD_OBJECT keyboard_obj, OperationData* Data) {
		ULONG input_data;
		KIRQL irql;
		KEYBOARD_INPUT_DATA kid = { 0 };

		kid.MakeCode = Data->Keyboard.keyCode;
		kid.Flags = Data->Keyboard.button_flags;

		KeRaiseIrql(DISPATCH_LEVEL, &irql);
		keyboard_obj.service_callback(keyboard_obj.keyboard_device, &kid, (PKEYBOARD_INPUT_DATA)&kid + 1, &input_data);
		KeLowerIrql(irql);

		return true;
	}
	bool mouseEvent(MOUSE_OBJECT mouse_obj, OperationData* Data)
	{
		ULONG input_data;
		KIRQL irql;
		MOUSE_INPUT_DATA mid = { 0 };

		mid.LastX = Data->Mouse.x;
		mid.LastY = Data->Mouse.y;
		mid.ButtonFlags = Data->Mouse.button_flags;

		KeRaiseIrql(DISPATCH_LEVEL, &irql);
		mouse_obj.service_callback(mouse_obj.mouse_device, &mid, (PMOUSE_INPUT_DATA)&mid + 1, &input_data);
		KeLowerIrql(irql);

		return true;
	}
	NTSTATUS setCursorPos(OperationData* Data)
	{
		NTSTATUS status = STATUS_SUCCESS;
		PEPROCESS winlogonProcess = nullptr;

		ANSI_STRING processName;
		UNICODE_STRING processUnicodeName = { 0 };
		RtlInitAnsiString(&processName, "winlogon.exe");
		RtlAnsiStringToUnicodeString(&processUnicodeName, &processName, TRUE);
		ULONG pid = Process::GetProcessId(processUnicodeName);
		RtlFreeUnicodeString(&processUnicodeName);

		if (pid != 0) {
			if (NT_SUCCESS(PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(pid), &winlogonProcess))) {
				KAPC_STATE apc = { 0 };
				KeStackAttachProcess(winlogonProcess, &apc);
				uint64_t gptCursorAsyncAddr = reinterpret_cast<uint64_t>(GetModuleExport("\\SystemRoot\\System32\\win32kbase.sys", "gptCursorAsync"));
				if (gptCursorAsyncAddr) {
					POINT cursorPos = *(POINT*)(gptCursorAsyncAddr);
					cursorPos.x = Data->Cursor.cursor_x;
					cursorPos.y = Data->Cursor.cursor_y;
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