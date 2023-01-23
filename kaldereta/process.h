#pragma once
#include "defs.h"

namespace process {
	PEPROCESS getProcess(DWORD procId) {
		PEPROCESS eProcess{ nullptr };
		PsLookupProcessByProcessId(reinterpret_cast<HANDLE>(procId), &eProcess);
		return eProcess;
	}

	ULONG getProcessId(UNICODE_STRING procName) {
		ULONG proc_id = 0;
		NTSTATUS status = STATUS_SUCCESS;

		PVOID buffer = ExAllocatePoolWithTag(NonPagedPool, 1024 * 1024, 'enoN');
		if (!buffer) {
			DbgPrintEx(0, 0, "Kaldereta: [ProcessID] Failed 0x1\n");
			return 0;
		}

		PSYSTEM_PROCESS_INFORMATION pInfo = (PSYSTEM_PROCESS_INFORMATION)buffer;

		status = ZwQuerySystemInformation(SystemProcessInformation, pInfo, 1024 * 1024, NULL);
		if (!NT_SUCCESS(status)) {
			DbgPrintEx(0, 0, "Kaldereta: [ProcessID] Failed 0x2, Code: %08X\n", status);
			return 0;
		}

		for (;;) {
			if (RtlEqualUnicodeString(&pInfo->ImageName, &procName, TRUE)) {
				return (ULONG)pInfo->UniqueProcessId;
			}
			else if (pInfo->NextEntryOffset) {
				pInfo = (PSYSTEM_PROCESS_INFORMATION)((PUCHAR)pInfo + pInfo->NextEntryOffset);
			}
			else {
				break;
			}
		}

		ExFreePoolWithTag(buffer, 'enoN');

		return proc_id;
	}

	NTSTATUS getBaseAddress(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ getProcess((DWORD)data->procId) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		data->baseAddress = PsGetProcessSectionBaseAddress(eProcess);

		ObfDereferenceObject(eProcess);
		return data->baseAddress ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	NTSTATUS getMainModuleSize(SANGKAP_SA_KALDERETA* data) {
		ANSI_STRING AS;
		UNICODE_STRING moduleName;
		RtlInitAnsiString(&AS, data->procName);
		RtlAnsiStringToUnicodeString(&moduleName, &AS, TRUE);

		PEPROCESS eProcess{ getProcess((DWORD)data->procId) };

		PPEB pPeb = PsGetProcessPeb(eProcess);
		if (!pPeb)
			return 0;

		KAPC_STATE state;

		KeStackAttachProcess(eProcess, &state);

		PPEB_LDR_DATA pLdr = (PPEB_LDR_DATA)pPeb->Ldr;

		if (!pLdr)
		{
			KeUnstackDetachProcess(&state);
			return 0;
		}

		for (PLIST_ENTRY list = (PLIST_ENTRY)pLdr->InLoadOrderModuleList.Flink; list != &pLdr->InLoadOrderModuleList; list = (PLIST_ENTRY)list->Flink)
		{
			PLDR_DATA_TABLE_ENTRY pEntry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList);

			if (RtlCompareUnicodeString(&pEntry->BaseDllName, &moduleName, TRUE) == 0)
			{
				ULONG64 baseAddr = (ULONG64)pEntry->DllBase;
				data->imageSize = pEntry->SizeOfImage;

				KeUnstackDetachProcess(&state);
				return baseAddr;
			}
		}

		KeUnstackDetachProcess(&state);

		return data->imageSize ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	NTSTATUS getPeb(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ getProcess((DWORD)data->procId) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		data->peb = PsGetProcessPeb(eProcess);

		ObfDereferenceObject(eProcess);
		return data->peb ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	NTSTATUS queryInformation(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ getProcess((DWORD)data->procId) };
		KAPC_STATE apc{ 0 };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &apc);

		NTSTATUS Status{ ZwQueryInformationProcess(ZwCurrentProcess(),
							   ProcessBasicInformation,
							   &data->pbi,
							   sizeof(data->pbi),
							   nullptr) };

		KeUnstackDetachProcess(&apc);
		ObfDereferenceObject(eProcess);
		return Status;
	}

	NTSTATUS getModuleInfo(SANGKAP_SA_KALDERETA* data) {
		KAPC_STATE Apc{ 0 };
		PVOID Base{ nullptr };
		DWORD Size{ NULL };
		UNICODE_STRING usModule{ 0 };

		if (data->procName) {
			ANSI_STRING asModule{ 0 };

			RtlInitAnsiString(&asModule, data->procName);
			if (!NT_SUCCESS(RtlAnsiStringToUnicodeString(&usModule, &asModule, TRUE))) {
				return STATUS_UNSUCCESSFUL;
			}
		}

		PEPROCESS eProcess{ getProcess((DWORD)data->procId) };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		LIST_ENTRY* List = &(PsGetProcessPeb(eProcess)->Ldr->InLoadOrderModuleList);

		for (LIST_ENTRY* Entry = List->Flink; Entry != List;) {
			auto Module{ CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList) };

			if (Module) {
				++data->index;

				if (data->procName && !RtlCompareUnicodeString(&Module->BaseDllName, &usModule, TRUE)) {
					data->baseAddress = Module->DllBase;
					data->imageSize = Module->SizeOfImage;
				}
			}

			Entry = Module->InLoadOrderModuleList.Flink;
		}

		KeUnstackDetachProcess(&Apc);
		RtlFreeUnicodeString(&usModule);
		ObfDereferenceObject(eProcess);
		return data->imageSize ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
	}

	NTSTATUS getModuleInfoByIndex(SANGKAP_SA_KALDERETA* data) {
		PEPROCESS eProcess{ getProcess((DWORD)data->procId) };
		KAPC_STATE Apc{ 0 };
		int Count{ 0 };

		if (eProcess == nullptr) {
			return STATUS_UNSUCCESSFUL;
		}

		KeStackAttachProcess(eProcess, &Apc);

		LIST_ENTRY* List = &(PsGetProcessPeb(eProcess)->Ldr->InLoadOrderModuleList);

		for (LIST_ENTRY* Entry = List->Flink; Entry != List;) {
			auto Module{ CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderModuleList) };

			if (Module && Count == data->index) {
				data->baseAddress = Module->DllBase;
				data->imageSize = Module->SizeOfImage;
				break;
			}

			Count += 1;
			Entry = Module->InLoadOrderModuleList.Flink;
		}

		KeUnstackDetachProcess(&Apc);
		ObfDereferenceObject(eProcess);
		return STATUS_SUCCESS;
	}
}