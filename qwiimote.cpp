/**
  * @file
  * Source file for the QWiimote class.
  */

#include "qwiimote.h"
#include <setupapi.h>
#include <ddk/hidsdi.h>

const quint16 QWiimote::WIIMOTE_VENDOR_ID  = 0x057E;
const quint16 QWiimote::WIIMOTE_PRODUCT_ID = 0x0306;

QWiimote::QWiimote()
{
}

bool QWiimote::findWiimote()
{
    //Get the GUID of the HID class
    LPGUID guid = new GUID;
    HidD_GetHidGuid(guid);

    //Get device info
    HDEVINFO device_info = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_DEVICEINTERFACE);

    // Create a new interface data struct and initialize its size
    PSP_DEVICE_INTERFACE_DATA device_interface_data = new SP_DEVICE_INTERFACE_DATA;
    device_interface_data->cbSize = sizeof(*device_interface_data);

    //Enumerate through interfaces
    qint16 index = 1;
    PSP_DEVICE_INTERFACE_DETAIL_DATA device_interface_detail;
    DWORD required_size;
    HIDD_ATTRIBUTES attributes;
    bool wiimote_found = false;
    while (!wiimote_found && SetupDiEnumDeviceInterfaces(device_info, NULL, guid, index, device_interface_data))
    {
        //Get the required size
        SetupDiGetDeviceInterfaceDetail(device_info, device_interface_data, NULL, 0, &required_size, NULL);

        //Assign the required number of bytes
        device_interface_detail = PSP_DEVICE_INTERFACE_DETAIL_DATA(new quint8[required_size]);
        device_interface_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        //Get the device interface detailed info
        if (SetupDiGetDeviceInterfaceDetail(device_info, device_interface_data, device_interface_detail,
                                        required_size, NULL, NULL)) {
            wiimote_handle = CreateFile(device_interface_detail->DevicePath,
                                       (GENERIC_READ | GENERIC_WRITE),
                                       (FILE_SHARE_READ | FILE_SHARE_WRITE),
                                       NULL,
                                       OPEN_ALWAYS,
                                       FILE_FLAG_OVERLAPPED,
                                       NULL);
            if (HidD_GetAttributes(wiimote_handle, &attributes)) {
                if ((attributes.VendorID == WIIMOTE_VENDOR_ID) && (attributes.ProductID == WIIMOTE_PRODUCT_ID)) {
                    // The device is a wiimote. It is really connected?
                    wiimote_found = true;
                }
            } else {
                CloseHandle(wiimote_handle);
            }
        }

        free(device_interface_detail);
        index++;
    }

    SetupDiDestroyDeviceInfoList(device_info);
    return wiimote_found;
}

