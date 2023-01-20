#pragma once
#include "memory.h"
#include "system.h"
#include <cstdint>

MOUSE_OBJECT mouse_obj = { 0 };
KEYBOARD_OBJECT keyboard_obj = { 0 };

namespace SharedMemory
{
	template <typename T>
	struct Memory_t
	{
		T data;
		SIZE_T size;
		bool bSuccess;
	};

	template <typename T>
	Memory_t<T> ReadSharedMemory(PVOID Address, SIZE_T Size = sizeof(T))
	{
		SIZE_T Bytes = { 0 };

		Memory_t<T> mem;
		if (NT_SUCCESS(MmCopyVirtualMemory(gProcess, Address, IoGetCurrentProcess(), static_cast<PVOID>(&mem.data), Size, KernelMode, &Bytes)))
		{
			mem.size = Bytes;
			mem.bSuccess = true;
			return mem;
		}

		mem.size = 0;
		mem.bSuccess = false;
		return mem;
	}

	template <typename T>
	Memory_t<T> WriteSharedMemory(PVOID Address, T Buffer, SIZE_T Size = sizeof(T))
	{
		SIZE_T Bytes = { 0 };

		Memory_t<T> mem;
		if (NT_SUCCESS(MmCopyVirtualMemory(IoGetCurrentProcess(), (PVOID)&Buffer, gProcess, Address, Size, KernelMode, &Bytes)))
		{
			mem.size = Bytes;
			mem.bSuccess = true;
			return mem;
		}

		mem.size = 0;
		mem.bSuccess = false;
		return mem;
	}

	bool SetRequestCode(SIZE_T RequestID, Code code)
	{
		auto mem = WriteSharedMemory<SHORT>((PVOID)((PBYTE)gData.pRequestCode + (RequestID * sizeof(SHORT))), (SHORT)code);
		return mem.bSuccess;
	}

	Status GetStatus()
	{
		return *(Status*)gData.pStatus;
	}

	BOOL SetPendingRequest(SIZE_T RequestId, BOOL bPendingRequest)
	{
		auto mem = WriteSharedMemory<BOOL>((PVOID)((PBYTE)gData.bPendingRequest + (RequestId * sizeof(BOOL))), bPendingRequest);
		return mem.bSuccess;
	}

	BOOL havePendingRequest(SIZE_T RequestId, BOOL& bHaveRequest)
	{
		auto mem = ReadSharedMemory<BOOL>((PVOID)((PBYTE)gData.bPendingRequest + (RequestId * sizeof(BOOL))));

		if (mem.bSuccess)
			bHaveRequest = (Code)mem.data;

		return mem.bSuccess;
	}

	BOOL GetRequestCode(SIZE_T RequestId, Code& code)
	{
		auto mem = ReadSharedMemory<SHORT>((PVOID)((PBYTE)gData.pRequestCode + (RequestId * sizeof(SHORT))));

		if (mem.bSuccess)
			code = (Code)mem.data;

		return mem.bSuccess;
	}

	bool GetRequestData(SIZE_T RequestId, OperationData& oData)
	{
		auto mem = ReadSharedMemory<OperationData>((PVOID)((PBYTE)gData.pDataRequest + (RequestId * sizeof(OperationData))));

		if (mem.bSuccess)
			oData = mem.data;

		return mem.bSuccess;
	}

	bool WriteResponse(SIZE_T RequestId, OperationData& pData)
	{
		auto mem = WriteSharedMemory<OperationData>((PVOID)((PBYTE)gData.pDataResponse + (RequestId * sizeof(OperationData))), pData);
		return mem.bSuccess;
	}

	VOID Respond(SIZE_T RequestId)
	{
		BOOL bHaveRequest = false;
		if (!havePendingRequest(RequestId, bHaveRequest))
			return;

		if (!bHaveRequest)
			return;

		OperationData oData;
		GetRequestData(RequestId, oData);

		Code cRequest;
		GetRequestCode(RequestId, cRequest);

		switch (cRequest)
		{
		case BaseRequest:
			Process::GetBaseAddress(&oData);
			break;

		case SizeRequest:
			Process::GetMainModuleSize(&oData);
			break;

		case PebRequest:
			Process::GetPeb(&oData);
			break;

		case QIPRequest:
			Process::QueryInformation(&oData);
			break;

		case CopyRequest:
			Memory::CopyVirtualMemory(&oData);
			break;

		case AVMRequest:
			Memory::AllocateVirtualMemory(&oData);
			break;

		case FVMRequest:
			Memory::FreeVirtualMemory(&oData);
			break;

		case PVMRequest:
			Memory::ProtectVirtualMemory(&oData);
			break;

		case QVMRequest:
			Memory::QueryVirtualMemory(&oData);
			break;

		case ModuleRequest:
			Process::GetModuleInfo(&oData);
			break;

		case IndexRequest:
			Process::GetModuleInfoByIndex(&oData);
			break;

		case MouseRequest: 
			Memory::mouseEvent(mouse_obj, &oData);
			break;

		case KeyboardRequest:
			Memory::keyboardEvent(keyboard_obj, &oData);
			break;

		case SetCursorRequest:
			Memory::setCursorPos(&oData);
			break;

		default:
			SetPendingRequest(RequestId, false);
			SetRequestCode(RequestId, Failure);
			return;
		}

		oData.bComplete = true;
		if (!WriteResponse(RequestId, oData))
		{
			SetPendingRequest(RequestId, false);
			SetRequestCode(RequestId, Failure);
			return;
		}

		SetPendingRequest(RequestId, false);
		SetRequestCode(RequestId, Complete);
	}

	VOID Loop()
	{
		gProcess = Process::GetProcess(gData.ProcessId);

		if (gProcess == nullptr)
			return;

		if (!mouse_obj.service_callback || !mouse_obj.mouse_device)
		{
			Memory::InitializeMouse(&mouse_obj);
			Utils::GenerateTrampoline(mouse_obj.service_callback, (PVOID*)&mouse_obj.service_callback);
			if (!mouse_obj.service_callback || !mouse_obj.mouse_device)
				return;

		}

		if (!keyboard_obj.service_callback || !keyboard_obj.keyboard_device)
		{
			Memory::InitializeKeyboard(&keyboard_obj);
			Utils::GenerateTrampoline(keyboard_obj.service_callback, (PVOID*)&keyboard_obj.service_callback);
			if (!keyboard_obj.service_callback || !keyboard_obj.keyboard_device)
				return;
		}

		for (;;)
		{
			if (*(DWORD*)((BYTE*)gProcess + ActiveThreadsOffset) == 1)
			{
				// We're the only active thread - the client must be trying to terminate
				ObfDereferenceObject(gProcess);
				return;
			}

			if (GetStatus() == Exit)
			{
				ObfDereferenceObject(gProcess);
				return;
			}

			for (SIZE_T i = 0; i < numMemoryPools; ++i)
			{
				Respond(i);
			}
		}
	}
}