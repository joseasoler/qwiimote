#include "qiowiimote.h"

const quint16 QIOWiimote::WIIMOTE_VENDOR_ID  = 0x057E;
const quint16 QIOWiimote::WIIMOTE_PRODUCT_ID = 0x0306;

/* HidD_SetOutputReport is not defined in MinGW w32api. */
extern "C"{
    WINHIDSDI BOOL WINAPI HidD_SetOutputReport (HANDLE, PVOID, ULONG);
}

QIOWiimote::QIOWiimote()
{
    this->setOpenMode(QIODevice::NotOpen);
}

QIOWiimote::QIOWiimote(QObject * parent) : QIODevice(parent)
{
    this->setOpenMode(QIODevice::NotOpen);
}

QIOWiimote::~QIOWiimote()
{
    this->close();
}

/**
  * Opens the connection to a wiimote.
  * @param mode This parameter is unused: the open mode is always read-write.
  * @return True if the connection was successfully opened.
  */
bool QIOWiimote::open(OpenMode mode)
{
    Q_UNUSED(mode);
    // Get the GUID of the HID class.
    LPGUID guid = new GUID;
    HidD_GetHidGuid(guid);

    // Get device info.
    HDEVINFO device_info = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_DEVICEINTERFACE);

    // Create a new interface data struct and initialize its size.
    PSP_DEVICE_INTERFACE_DATA device_interface_data = new SP_DEVICE_INTERFACE_DATA;
    device_interface_data->cbSize = sizeof(*device_interface_data);

    // Enumerate through interfaces.
    qint16 index = 1;
    PSP_DEVICE_INTERFACE_DETAIL_DATA device_interface_detail;
    DWORD required_size;
    HIDD_ATTRIBUTES attributes;
    bool wiimote_found = false;
    while (!wiimote_found && SetupDiEnumDeviceInterfaces(device_info, NULL, guid, index, device_interface_data))
    {
        // Get the required size.
        SetupDiGetDeviceInterfaceDetail(device_info, device_interface_data, NULL, 0, &required_size, NULL);

        // Assign the required number of bytes.
        device_interface_detail = PSP_DEVICE_INTERFACE_DETAIL_DATA(new quint8[required_size]);
        device_interface_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        // Get the device interface detailed information.
        if (SetupDiGetDeviceInterfaceDetail(device_info, device_interface_data, device_interface_detail,
                                        required_size, NULL, NULL)) {
            // Create a handle to the device.
            wiimote_handle = CreateFile(device_interface_detail->DevicePath,
                                       (GENERIC_READ | GENERIC_WRITE),
                                       (FILE_SHARE_READ | FILE_SHARE_WRITE),
                                       NULL,
                                       OPEN_ALWAYS,
                                       FILE_FLAG_OVERLAPPED,
                                       NULL);
            if (HidD_GetAttributes(wiimote_handle, &attributes)) {
                // Check if the device is actually a wiimote.
                if ((attributes.VendorID == WIIMOTE_VENDOR_ID) && (attributes.ProductID == WIIMOTE_PRODUCT_ID)) {
                    // To test if the wiimote is really connected, an empty LED report is sent.
                    char led_report[] = {0x11, 0x00};
                    if (wiimote_found = this->writeData(led_report, 2)) {
                        // To get data from the wiimote, write access is required.
                        this->setOpenMode(QIODevice::ReadWrite);
                    }
                }
            }
            // The device is not a wiimote.
            if (!wiimote_found) {
                CloseHandle(wiimote_handle);
            }
        }

        free(device_interface_detail);
        index++;
    }

    free(device_interface_data);
    free(guid);
    SetupDiDestroyDeviceInfoList(device_info);
    return wiimote_found;
}

void QIOWiimote::close()
{
    if (this->openMode() != QIODevice::NotOpen) CloseHandle(wiimote_handle);
    this->setOpenMode(QIODevice::NotOpen);
}

/* Protected */

qint64 QIOWiimote::readData(char * data, qint64 max_size)
{
    return 0;
}

qint64 QIOWiimote::readLineData(char * data, qint64 max_size)
{
    return 0;
}

qint64 QIOWiimote::writeData(const char * data, qint64 max_size)
{
    if (HidD_SetOutputReport(this->wiimote_handle, strdup(data), max_size) == TRUE) return max_size;
    return -1;
}
