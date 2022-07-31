#include <ntifs.h>
#include "events.h"


extern "C" {

    /*
        Macros IOCTL :D
    */
    #define DrvInit = 0x80034000
    #define Mdl = 0x81004000
    #define HeartBeat = 0x81014000
    #define HeartBeat2 = 0x80024000 
    #define RWMemory = 0x81074000 
    #define EnumProcessList = 0x83014000
    #define ListProcessModule = 0x81054000
    #define Unk1 = 0x82004000
    #define EnumDrivers = 0x82024000 

    /*
        Macros Name :D
    */
    #define MHYPROT2_WIN32_DEVICE_NAME_A		"\\\\.\\mhyprot2"
    #define MHYPROT2_WIN32_DEVICE_NAME_W		L"\\\\.\\mhyprot2"
    #define MHYPROT2_DEVICE_NAME_A			"\\Device\\mhyprot2"
    #define MHYPROT2_DEVICE_NAME_W			L"\\Device\\mhyprot2"
    #define MHYPROT2_DOS_DEVICE_NAME_A		"\\DosDevices\\mhyprot2" 
    #define MHYPROT2_DOS_DEVICE_NAME_W		L"\\DosDevices\\mhyprot2" 


    #define FILE_DEVICE_HIDE	0x8000 

    #define IOCTL_BASE			 0x800 

    #define CTL_CODE_HIDE(i)	\
	CTL_CODE(FILE_DEVICE_HIDE, IOCTL_BASE+i, METHOD_BUFFERED, FILE_ANY_ACCESS) 

    #define KillProcess CTL_CODE_HIDE(1)
    #define ScannProcess CTL_CODE_HIDE(2) 
    #define ReadProcessMem CTL_CODE_HIDE(3)
    #define SetProcessHook CTL_CODE_HIDE(4) 

    typedef struct _DEVICE_EXTENSION
    {
        ULONG  Extension;
    } DEVICE_EXTENSION, * PDEVICE_EXTENSION;

    VOID KdCreateDeviceSymbolLink(PDRIVER_OBJECT driverObj);

    NTSTATUS KdMhyprot2DeviceCtrlRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp);

    typedef struct _SYSTEM_THREADS {
        LARGE_INTEGER  KernelTime;
        LARGE_INTEGER  UserTime;
        LARGE_INTEGER  CreateTime;
        ULONG          WaitTime;
        PVOID          StartAddress;
        CLIENT_ID      ClientId;
        KPRIORITY      Priority;
        KPRIORITY      BasePriority;
        ULONG          ContextSwitchCount;
        LONG           State;
        LONG           WaitReason;
    } SYSTEM_THREADS, * PSYSTEM_THREADS;


    typedef struct _SYSTEM_PROCESSES {
        ULONG            NextEntryDelta;
        ULONG            ThreadCount;
        ULONG            Reserved1[6];
        LARGE_INTEGER    CreateTime;
        LARGE_INTEGER    UserTime;
        LARGE_INTEGER    KernelTime;
        UNICODE_STRING   ProcessName;
        KPRIORITY        BasePriority;
        SIZE_T           ProcessId;
        SIZE_T           InheritedFromProcessId;
        ULONG            HandleCount;
        ULONG            Reserved2[2];
        VM_COUNTERS      VmCounters;
        IO_COUNTERS      IoCounters;
        SYSTEM_THREADS   Threads[1];
    } SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;


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
        UCHAR  FullPathName[256];
    } RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

    typedef struct _RTL_PROCESS_MODULES
    {
        ULONG NumberOfModules;
        RTL_PROCESS_MODULE_INFORMATION Modules[1];
    } RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;


    #define SystemModuleInformation	0x0B  

    NTSTATUS NTAPI ZwQuerySystemInformation(ULONG SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

    NTSTATUS KdScanProcesso(const char* NomeProcesso);

    typedef struct _KERNEL_JOGO_LER_MEMORIA {
        ULONG PID;
        PVOID ENDERECO;
        PVOID RESPOSTA;
        ULONG TAMANHO;
    } KERNEL_JOGO_LER_MEMORIA, *PKERNEL_JOGO_LER_MEMORIA;


    VOID KdSetBaseAddrRW(PVOID addr);

    VOID KdSetPidAddrRW(HANDLE PID);

    const wchar_t* KdGetProcessBlackList();

    typedef struct _KERNEL_PROCESSO_ANALISE {
        const wchar_t* nome_processo;
        int status;
    } KERNEL_PROCESSO_ANALISE, * PKERNEL_PROCESSO_ANALISE;

}