#pragma once
#include <ntifs.h>
#include <ntddk.h>
#include <IntSafe.h>
#include <ntimage.h>
#include <ntddmou.h>
#include <ntddkbd.h>
#include <windef.h>
#include <cstdint>

constexpr ULONG BaseRequest	= CTL_CODE(FILE_DEVICE_UNKNOWN, 0x601, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG SizeRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x602, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG PebRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x603, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG QIPRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x604, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG ReadRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x605, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG WriteRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x606, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG AVMRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x607, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG FVMRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x608, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG PVMRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x609, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG QVMRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x610, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG ModuleRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x611, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG IndexRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x612, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG MouseRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x613, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG KeyboardRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x614, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
constexpr ULONG SetCursorRequest = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x615, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);

struct SANGKAP_SA_KALDERETA {
	char* procName;
	HANDLE procId;
	PVOID baseAddress;
	SIZE_T imageSize;
	int index;

	PVOID address;
	PVOID buffer;
	SIZE_T size;
	SIZE_T returnLength;

	USHORT keyCode;
	USHORT keyFlags;

	long mouseX;
	long mouseY;
	USHORT buttonFlags;

	PPEB peb;
	PROCESS_BASIC_INFORMATION pbi;

	DWORD allocType;
	DWORD freeType;
	DWORD protect;
	DWORD oldProtect;
	MEMORY_BASIC_INFORMATION mbi;
};

extern "C" POBJECT_TYPE * IoDriverObjectType;

typedef VOID
(*MouseClassServiceCallback)(
	PDEVICE_OBJECT DeviceObject,
	PMOUSE_INPUT_DATA InputDataStart,
	PMOUSE_INPUT_DATA InputDataEnd,
	PULONG InputDataConsumed
	);

typedef VOID
(*KeyboardClassServiceCallback)(
	PDEVICE_OBJECT DeviceObject,
	PKEYBOARD_INPUT_DATA InputDataStart,
	PKEYBOARD_INPUT_DATA InputDataEnd,
	PULONG InputDataConsumed
	);

typedef struct _MOUSE_OBJECT
{
	PDEVICE_OBJECT mouse_device;
	MouseClassServiceCallback service_callback;
} MOUSE_OBJECT, * PMOUSE_OBJECT;

typedef struct _KEYBOARD_OBJECT
{
	PDEVICE_OBJECT keyboard_device;
	KeyboardClassServiceCallback service_callback;
} KEYBOARD_OBJECT, * PKEYBOARD_OBJECT;

typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,
	SystemProcessorInformation,
	SystemPerformanceInformation,
	SystemTimeOfDayInformation,
	SystemPathInformation,
	SystemProcessInformation,
	SystemCallCountInformation,
	SystemDeviceInformation,
	SystemProcessorPerformanceInformation,
	SystemFlagsInformation,
	SystemCallTimeInformation,
	SystemModuleInformation,
	SystemLocksInformation,
	SystemStackTraceInformation,
	SystemPagedPoolInformation,
	SystemNonPagedPoolInformation,
	SystemHandleInformation,
	SystemObjectInformation,
	SystemPageFileInformation,
	SystemVdmInstemulInformation,
	SystemVdmBopInformation,
	SystemFileCacheInformation,
	SystemPoolTagInformation,
	SystemInterruptInformation,
	SystemDpcBehaviorInformation,
	SystemFullMemoryInformation,
	SystemLoadGdiDriverInformation,
	SystemUnloadGdiDriverInformation,
	SystemTimeAdjustmentInformation,
	SystemSummaryMemoryInformation,
	SystemNextEventIdInformation,
	SystemEventIdsInformation,
	SystemCrashDumpInformation,
	SystemExceptionInformation,
	SystemCrashDumpStateInformation,
	SystemKernelDebuggerInformation,
	SystemContextSwitchInformation,
	SystemRegistryQuotaInformation,
	SystemExtendServiceTableInformation,
	SystemPrioritySeperation,
	SystemPlugPlayBusInformation,
	SystemDockInformation,
	SystemProcessorSpeedInformation,
	SystemCurrentTimeZoneInformation,
	SystemLookasideInformation
} SYSTEM_INFORMATION_CLASS, * PSYSTEM_INFORMATION_CLASS;
typedef struct _SYSTEM_PROCESS_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	LARGE_INTEGER SpareLi1;
	LARGE_INTEGER SpareLi2;
	LARGE_INTEGER SpareLi3;
	LARGE_INTEGER CreateTime;
	LARGE_INTEGER UserTime;
	LARGE_INTEGER KernelTime;
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	HANDLE InheritedFromUniqueProcessId;
	ULONG HandleCount;
	ULONG SessionId;
	ULONG_PTR PageDirectoryBase;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG PageFaultCount;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	SIZE_T QuotaPeakPagedPoolUsage;
	SIZE_T QuotaPagedPoolUsage;
	SIZE_T QuotaPeakNonPagedPoolUsage;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER ReadOperationCount;
	LARGE_INTEGER WriteOperationCount;
	LARGE_INTEGER OtherOperationCount;
	LARGE_INTEGER ReadTransferCount;
	LARGE_INTEGER WriteTransferCount;
	LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

typedef struct _PEB_LDR_DATA {
	ULONG Length;
	BOOLEAN Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, * PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	PVOID Mutant;
	PVOID ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
	PVOID ProcessParameters;
	PVOID SubSystemData;
	PVOID ProcessHeap;
	PVOID FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	PVOID CrossProcessFlags;
	PVOID KernelCallbackTable;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	PVOID ApiSetMap;
} PEB, * PPEB;

typedef struct _SYSTEM_MODULE
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[MAXIMUM_FILENAME_LENGTH];
} SYSTEM_MODULE, * PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG NumberOfModules;
	SYSTEM_MODULE Modules[1];
} SYSTEM_MODULE_INFORMATION, * PSYSTEM_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR  FullPathName[MAXIMUM_FILENAME_LENGTH];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct PiDDBCache
{
	LIST_ENTRY		List;
	UNICODE_STRING	DriverName;
	ULONG			TimeDateStamp;
	NTSTATUS		LoadStatus;
	char			_0x0028[16];
};

extern "C"
{
	NTKERNELAPI 
		NTSTATUS 
		IoCreateDriver(
			PUNICODE_STRING DriverName, 
			PDRIVER_INITIALIZE InitializationFunction
		);

	NTKERNELAPI
		PVOID
		PsGetProcessSectionBaseAddress(
			PEPROCESS Process
		);

	NTKERNELAPI
		PPEB
		NTAPI
		PsGetProcessPeb(
			PEPROCESS Process
		);

	NTKERNELAPI
		NTSTATUS
		MmCopyVirtualMemory(
			PEPROCESS SourceProcess,
			PVOID SourceAddress,
			PEPROCESS TarGet,
			PVOID TargetAddress,
			SIZE_T BufferSize,
			KPROCESSOR_MODE PreviousMode,
			PSIZE_T ReturnSize
		);

	NTSYSCALLAPI
		NTSTATUS
		NTAPI
		ZwQuerySystemInformation(
			ULONG InfoClass,
			PVOID Buffer,
			ULONG Length,
			PULONG ReturnLength
		);

	NTSYSCALLAPI
		NTSTATUS
		ZwQueryInformationProcess(
			HANDLE ProcessHandle,
			PROCESSINFOCLASS ProcessInformationClass,
			PVOID ProcessInformation,
			ULONG ProcessInformationLength,
			PULONG ReturnLength
		);

	NTSYSCALLAPI
		NTSTATUS
		NTAPI
		ZwProtectVirtualMemory(
			HANDLE ProcessHandle,
			PVOID* BaseAddress,
			PSIZE_T RegionSize,
			ULONG NewAccessProtection,
			PULONG OldAccessProtection
		);

	NTSYSCALLAPI
		NTSTATUS
		NTAPI
		ObReferenceObjectByName(
			_In_ PUNICODE_STRING ObjectName,
			_In_ ULONG Attributes,
			_In_opt_ PACCESS_STATE AccessState,
			_In_opt_ ACCESS_MASK DesiredAccess,
			_In_ POBJECT_TYPE ObjectType,
			_In_ KPROCESSOR_MODE AccessMode,
			_Inout_opt_ PVOID ParseContext,
			_Out_ PVOID* Object
		);

	PVOID NTAPI RtlFindExportedRoutineByName(
		_In_ PVOID ImageBase,
		_In_ PCCH RoutineName
	);
}