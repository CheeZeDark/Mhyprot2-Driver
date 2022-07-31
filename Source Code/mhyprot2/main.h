#pragma once
#include <ntifs.h>


extern "C" {
    VOID DriverUnload(PDRIVER_OBJECT pDriverObj);

    NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj, PUNICODE_STRING pRegistryString);
    //################################################################################################################################


    PDEVICE_OBJECT pDeviceObject;
    UNICODE_STRING dos, device;

}