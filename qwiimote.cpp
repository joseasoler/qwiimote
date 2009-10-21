/**
  * @file
  * Source file for the QWiimote class.
  *
  * Bluetooth connection code based in the work of Michael Laforest at wiiuse (www.wiiuse.net)
  */

#include "qwiimote.h"
#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>

const quint16 QWiimote::WIIMOTE_VENDOR_ID  = 0x057E; ///< Wiimote vendor ID
const quint16 QWiimote::WIIMOTE_PRODUCT_ID = 0x0306; ///< Wiimote product ID

QWiimote::QWiimote()
{
}

bool QWiimote::findWiimote()
{
    // Get the device ID.
    GUID device_ID;
    HidD_GetHidGuid(&device_ID);

    SP_DEVICE_INTERFACE_DATA device_data;
    device_data.cbSize = sizeof(device_data);

    // Get all connected HID devices
    HDEVINFO device_info;
    device_info = SetupDiGetClassDevs(&device_ID, NULL, NULL, (DIGCF_DEVICEINTERFACE | DIGCF_PRESENT));

    HANDLE dev;
    int i, index;
    DWORD len;
    PSP_DEVICE_INTERFACE_DETAIL_DATA detail_data = NULL;
    HIDD_ATTRIBUTES attr;

    index = 0;
    bool found = false;

    forever {
        // Free detail_data if it was used
        if (detail_data != NULL) {
            free(detail_data);
            detail_data = NULL;
        }

        // Query the next HID device info
        index++;
        if (!SetupDiEnumDeviceInterfaces(device_info, NULL, &device_ID, index, &device_data)) break;

        // Get the size of the data block required
        i = SetupDiGetDeviceInterfaceDetail(device_info, &device_data, NULL, 0, &len, NULL);
        detail_data = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(len);
        detail_data->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        // Query the data for this device
        if (!SetupDiGetDeviceInterfaceDetail(device_info, &device_data, detail_data, len, NULL, NULL)) continue;

        // Open the device
        dev = CreateFile(detail_data->DevicePath,
                         (GENERIC_READ | GENERIC_WRITE),
                         (FILE_SHARE_READ | FILE_SHARE_WRITE),
                         NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

        if (dev == INVALID_HANDLE_VALUE) continue;

        // Get device attributes
        attr.Size = sizeof(attr);
        i = HidD_GetAttributes(dev, &attr);

        if ((attr.VendorID == WIIMOTE_VENDOR_ID) && (attr.ProductID == WIIMOTE_PRODUCT_ID)) {
            // Is the wiimote really connected?
            found = true;
            break;
        } else {
            // Not a wiimote
            CloseHandle(dev);
        }
    }

    if (detail_data) free(detail_data);

    SetupDiDestroyDeviceInfoList(device_info);

    return found;
}

