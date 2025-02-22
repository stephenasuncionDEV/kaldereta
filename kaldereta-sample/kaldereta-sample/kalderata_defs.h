#pragma once

typedef struct __KALDERETA_MEMORY
{
	const char* moduleName;

	long x;
	long y;

	void* output;
	void* bufferAddress;

	ULONG pid;
	ULONG protection;
	ULONG allocationType;
	ULONG freeType;
	ULONG oldProtection;
	UINT_PTR address;
	ULONG64 baseAddress;
	ULONGLONG imageSize;
	ULONGLONG size;
	USHORT buttonFlags;
	USHORT keyCode;

	BOOLEAN reqProcessId;
	BOOLEAN reqBaseAddress;
	BOOLEAN virtualProtect;
	BOOLEAN virtualAlloc;
	BOOLEAN virtualFree;
	BOOLEAN write;
	BOOLEAN writeBuffer;
	BOOLEAN read;
	BOOLEAN readBuffer;
	BOOLEAN mouseEvent;
	BOOLEAN keyboardEvent;
	BOOLEAN setCursorPos;
}KALDERETA_MEMORY;

typedef HMODULE(__stdcall* pLoadLibraryA)(LPCSTR);
typedef FARPROC(__stdcall* pGetProcAddress)(HMODULE, LPCSTR);
typedef INT(__stdcall* dllmain)(HMODULE, DWORD, LPVOID);

struct loaderdata
{
	LPVOID ImageBase;
	PIMAGE_NT_HEADERS NtHeaders;
	PIMAGE_BASE_RELOCATION BaseReloc;
	PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
	pLoadLibraryA fnLoadLibraryA;
	pGetProcAddress fnGetProcAddress;
};

#define MOUSE_LEFT_BUTTON_DOWN   0x0001  // Left Button changed to down.
#define MOUSE_LEFT_BUTTON_UP     0x0002  // Left Button changed to up.
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004  // Right Button changed to down.
#define MOUSE_RIGHT_BUTTON_UP    0x0008  // Right Button changed to up.
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010  // Middle Button changed to down.
#define MOUSE_MIDDLE_BUTTON_UP   0x0020  // Middle Button changed to up.

#define MOUSE_BUTTON_1_DOWN		 MOUSE_LEFT_BUTTON_DOWN
#define MOUSE_BUTTON_1_UP        MOUSE_LEFT_BUTTON_UP
#define MOUSE_BUTTON_2_DOWN      MOUSE_RIGHT_BUTTON_DOWN
#define MOUSE_BUTTON_2_UP        MOUSE_RIGHT_BUTTON_UP
#define MOUSE_BUTTON_3_DOWN      MOUSE_MIDDLE_BUTTON_DOWN
#define MOUSE_BUTTON_3_UP        MOUSE_MIDDLE_BUTTON_UP

#define MOUSE_BUTTON_4_DOWN      0x0040
#define MOUSE_BUTTON_4_UP        0x0080
#define MOUSE_BUTTON_5_DOWN      0x0100
#define MOUSE_BUTTON_5_UP        0x0200

#define MOUSE_WHEEL              0x0400
#define MOUSE_HWHEEL			 0x0800

#define MOUSE_MOVE_RELATIVE         0
#define MOUSE_MOVE_ABSOLUTE         1
#define MOUSE_VIRTUAL_DESKTOP    0x02  // the coordinates are mapped to the virtual desktop
#define MOUSE_ATTRIBUTES_CHANGED 0x04  // requery for mouse attributes

#define KEY_MAKE				 0
#define KEY_BREAK				 1
#define KEY_E0					 2
#define KEY_E1					 4
#define KEY_TERMSRV_SET_LED		 8
#define KEY_TERMSRV_SHADOW		 0x10
#define KEY_TERMSRV_VKPACKET	 0x20

#define KEY_DOWN                 KEY_MAKE
#define KEY_UP                   KEY_BREAK
#define KEY_BLANK                -1