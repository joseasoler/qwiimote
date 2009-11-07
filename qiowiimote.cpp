#include "qiowiimote.h"
#include "debugcheck.h"
#include <ctime>

const quint16 QIOWiimote::WIIMOTE_VENDOR_ID  = 0x057E;
const quint16 QIOWiimote::WIIMOTE_PRODUCT_ID = 0x0306;

/* HidD_SetOutputReport is not defined in MinGW w32api. */
extern "C"{
    WINHIDSDI BOOL WINAPI HidD_SetOutputReport (HANDLE, PVOID, ULONG);
}

/* Public */

QIOWiimote::QIOWiimote(QObject * parent) : QObject(parent)
{
    opened = false;
    report_queue.clear();
}

QIOWiimote::~QIOWiimote()
{
    this->close();
}

/**
  * Opens the connection to a wiimote.
  * @return true if the connection was successfully opened. false otherwise.
  */
bool QIOWiimote::open()
{
    // Get the GUID of the HID class.
    LPGUID guid = new GUID;
    HidD_GetHidGuid(guid);

    // Get device info.
    HDEVINFO device_info = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_DEVICEINTERFACE);

    // Create a new interface data struct and initialize its size.
    PSP_DEVICE_INTERFACE_DATA device_interface_data = new SP_DEVICE_INTERFACE_DATA;
    device_interface_data->cbSize = sizeof(*device_interface_data);

    // Iterate through interfaces.
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
                    if (wiimote_found = this->writeReport(led_report, 2)) {
                        // Prepare the overlapped structure.
                        this->overlapped = new OverlappedQIOWiimote;
                        this->overlapped->iowiimote = this;

                        // Schedule the first read.
                        this->readBegin();

                        opened = true;
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
    if (opened) {
        //Cancel pending data read from the wiimote.
        CancelIo(this->wiimote_handle);

        // Close device handle.
        CloseHandle(this->wiimote_handle);
        free(overlapped);

        // Clear the report queue list.
        report_queue.clear();
        opened = false;
    }
}

/**
  * Writing is done synchronously.
  */
bool QIOWiimote::writeReport(const char * data, qint64 max_size)
{
    return (HidD_SetOutputReport(this->wiimote_handle, strdup(data), max_size) == TRUE);
}

/* Private */

/**
  * Starts asynchronous read of data.
  */
void QIOWiimote::readBegin()
{
    ReadFileEx(this->wiimote_handle,
               this->read_buffer,
               22,
               (LPOVERLAPPED)this->overlapped,
               QIOWiimote::readCallback);
}

/**
  * This callback is called whenever a read operation is finished.
  */
void CALLBACK QIOWiimote::readCallback(DWORD error_code,
                                       DWORD bytes_transferred,
                                       LPOVERLAPPED overlapped)
{
    QIOWiimote * this_io = ((OverlappedQIOWiimote *)overlapped)->iowiimote;
    this_io->readEnd(error_code, bytes_transferred);
}

/**
  * Takes the raw report and makes it ready for processing.
  * The report format is time|report
  */
void QIOWiimote::readEnd(DWORD error_code, DWORD bytes_transferred)
{
    if (error_code == 0) {
        QByteArray new_report = QByteArray::number(time(), 10);
        new_report.append(0xFF);
        new_report.append(this->read_buffer, bytes_transferred);
        report_queue.enqueue(new_report);
    } else {
        emit this->reportError();
    }
}
