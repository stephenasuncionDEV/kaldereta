#pragma once
#include "global.h"
#include "ldasm.h"

namespace Utils {

	VOID Sleep(INT ms) {
		LARGE_INTEGER li{ 0 };
		li.QuadPart = -10000;

		for (INT i{ 0 }; i < ms; i++) {
			KeDelayExecutionThread(KernelMode, FALSE, &li);
		}
	}

	BOOLEAN ProbeUserAddress(PVOID Address, SIZE_T Size, DWORD Alignment) {
		if (Size == 0) {
			return TRUE;
		}

		DWORD64 Current = (DWORD64)Address;
		if (((DWORD64)Address & (Alignment - 1)) != 0) {
			return FALSE;
		}

		DWORD64 Last{ Current + Size - 1 };

		if ((Last < Current) || (Last >= MmUserProbeAddress)) {
			return FALSE;
		}

		return TRUE;
	}

	CHAR* LowerStr(CHAR* Str) {
		for (CHAR* S = Str; *S; ++S) {
			*S = (CHAR)tolower(*S);
		}
		return Str;
	}

	BOOLEAN CheckMask(CHAR* Base, CHAR* Pattern, CHAR* Mask) {
		for (; *Mask; ++Base, ++Pattern, ++Mask) {
			if (*Mask == 'x' && *Base != *Pattern) {
				return FALSE;
			}
		}

		return TRUE;
	}

	PVOID FindPattern(CHAR* Base, DWORD Length, CHAR* Pattern, CHAR* Mask) {
		Length -= (DWORD)strlen(Mask);

		for (DWORD i = 0; i <= Length; ++i) {
			PVOID Addr{ &Base[i] };

			if (CheckMask(static_cast<PCHAR>(Addr), Pattern, Mask)) {
				return Addr;
			}
		}

		return 0;
	}

	PVOID FindPatternImage(CHAR* Base, CHAR* Pattern, CHAR* Mask) {
		PVOID Match{ 0 };

		IMAGE_NT_HEADERS* Headers{ (PIMAGE_NT_HEADERS)(Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew) };
		IMAGE_SECTION_HEADER* Sections{ IMAGE_FIRST_SECTION(Headers) };

		for (DWORD i = 0; i < Headers->FileHeader.NumberOfSections; ++i) {
			IMAGE_SECTION_HEADER* Section{ &Sections[i] };

			if (*(INT*)Section->Name == 'EGAP' || memcmp(Section->Name, ".text", 5) == 0) {
				Match = FindPattern(Base + Section->VirtualAddress, Section->Misc.VirtualSize, Pattern, Mask);

				if (Match) {
					break;
				}
			}
		}

		return Match;
	}
	ULONG GetCPZ(PUCHAR Address, int mLen)
	{
		ldasm_data ld = { 0 };
		ULONG LenCount = 0, Len = 0;

		while (LenCount < mLen)
		{
			Len = ldasm(Address, &ld, TRUE);
			Address = Address + Len;
			LenCount = LenCount + Len;
		}

		return LenCount;
	}
	void GenerateTrampoline(IN PVOID FunctionAddress, OUT PVOID* TrampolineAddress) // Bypass For EAC,Vanguard and BE Made by Jenrix
	{
		UCHAR jmp_code_orifunc1[] = "\x90\xFF\x25\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF";

		UINT64 tmpv;
		PVOID HeaderByes, ori_func;
		ULONG PatchSize = GetCPZ((PUCHAR)FunctionAddress, 33);

		//step 1: Read current data
		HeaderByes = ExAllocatePool(NonPagedPool, 200);
		if (!HeaderByes)
		{
			DbgPrintEx(0, 0, "Error allocating memory for HeaderByes\n");
			return;
		}
		memcpy(HeaderByes, FunctionAddress, PatchSize);

		//step 2: Create ori function
		ori_func = ExAllocatePool(NonPagedPool, 100);
		if (!ori_func)
		{
			DbgPrintEx(0, 0, "Error allocating memory for ori_func\n");
			ExFreePool(HeaderByes);
			return;
		}
		memset(ori_func, 0x90, 100);

		char* tmpb = (char*)ExAllocatePool(NonPagedPool, 80);
		if (!tmpb)
		{
			DbgPrintEx(0, 0, "Error allocating memory for tmpb\n");
			ExFreePool(HeaderByes);
			ExFreePool(ori_func);
			return;
		}
		memcpy(tmpb, &jmp_code_orifunc1, 15);

		tmpv = (ULONG64)FunctionAddress + PatchSize;
		memcpy(tmpb + 7, &tmpv, 8);
		memcpy((PUCHAR)ori_func, HeaderByes, PatchSize);
		memcpy((PUCHAR)ori_func + PatchSize, tmpb, 15);

		*TrampolineAddress = ori_func;
		ExFreePool(tmpb);
		ExFreePool(HeaderByes);
	}
}