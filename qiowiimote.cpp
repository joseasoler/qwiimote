/**
  * @file qiowiimote.cpp
  * Source file for the QIOWiimote class.
  */

#include "qiowiimote.h"
#include "debugcheck.h"

const quint16 QIOWiimote::WIIMOTE_VENDOR_ID  = 0x057E;
const quint16 QIOWiimote::WIIMOTE_PRODUCT_ID = 0x0306;

/* HidD_SetOutputReport is not defined in MinGW w32api. */
#if defined(__MINGW32__) && defined(_WIN32)
extern "C"{
	WINHIDSDI BOOL WINAPI HidD_SetOutputReport (HANDLE, PVOID, ULONG);
}
#endif

/* Public functions */

/**
  * Creates a new QIOWiimote object.
  * @param parent The parent of this instance. Usually it will be a #QWiimote.
  */
QIOWiimote::QIOWiimote(QObject * parent) : QObject(parent)
{
}

/**
  * Ensures that the wiimote connection is correctly closed before destroying the QIOWiimote object.
  */
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
	/* Get the GUID of the HID class. */
	LPGUID guid = new GUID;
	HidD_GetHidGuid(guid);

	/* Get device info. */
	HDEVINFO device_info = SetupDiGetClassDevs(guid, NULL, NULL, DIGCF_DEVICEINTERFACE);

	/* Create a new interface data struct and initialize its size. */
	PSP_DEVICE_INTERFACE_DATA device_interface_data = new SP_DEVICE_INTERFACE_DATA;
	device_interface_data->cbSize = sizeof(*device_interface_data);

	/* Iterate through interfaces until a wiimote is found. */
	qint16 index = 1;
	PSP_DEVICE_INTERFACE_DETAIL_DATA device_interface_detail;
	DWORD required_size;
	HIDD_ATTRIBUTES attributes;
	this->opened = false;
	while (!this->opened && SetupDiEnumDeviceInterfaces(device_info, NULL, guid, index, device_interface_data))
	{
		/* Get the required size. */
		SetupDiGetDeviceInterfaceDetail(device_info, device_interface_data, NULL, 0, &required_size, NULL);

		/* Assign the required number of bytes. */
		device_interface_detail = PSP_DEVICE_INTERFACE_DETAIL_DATA(new quint8[required_size]);
		device_interface_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		/* Get the device interface detailed information. */
		if (SetupDiGetDeviceInterfaceDetail
				(device_info, device_interface_data, device_interface_detail, required_size, NULL, NULL)) {
			/* Create a handle to the device. */
			wiimote_handle = CreateFile(device_interface_detail->DevicePath,
									   (GENERIC_READ | GENERIC_WRITE),
									   (FILE_SHARE_READ | FILE_SHARE_WRITE),
									   NULL,
									   OPEN_ALWAYS,
									   FILE_FLAG_OVERLAPPED,
									   NULL);
			if (HidD_GetAttributes(wiimote_handle, &attributes)) {
				/* Check if the device is actually a wiimote. */
				if ((attributes.VendorID == WIIMOTE_VENDOR_ID) && (attributes.ProductID == WIIMOTE_PRODUCT_ID)) {
					/* To test if the wiimote is really connected, an empty LED report is sent. */
					char led_report[] = {0x11, 0x00};
					if ((this->opened = this->writeReport(led_report, 2))) {
						// Prepare the overlapped structure.
						this->overlapped = new OverlappedQIOWiimote;
						this->overlapped->iowiimote = this;

						/* Schedule the first read. */
						this->readBegin();
					}
				}
			}
			/* The device is not a wiimote. */
			if (!this->opened) {
				CloseHandle(wiimote_handle);
			}
		}

		free(device_interface_detail);
		index++;
	}

	/* Clean up used variables. */
	free(device_interface_data);
	free(guid);
	SetupDiDestroyDeviceInfoList(device_info);
	return this->opened;
}

/**
  * Closes the connection to the Wiimote.
  * @todo Change reporting type before closing the connection. Does not seem necessary, though.
  */
void QIOWiimote::close()
{
	if (opened) {
		/* Send an empty LED report to the wiimote. */
		char led_report[] = {0x11, 0x00};
		this->writeReport(led_report, 2);

		/* Cancel pending data read from the wiimote. */
		CancelIo(this->wiimote_handle);

		/* Close device handle. */
		CloseHandle(this->wiimote_handle);
		free(overlapped);

		opened = false;
	}
}

/**
  * Sends a report to the Wiimote. Writing is done synchronously.
  * @param data Report that will be sent to the wiimote.
  * @param max_size Size of the report. Using a size greater than #MAX_REPORT_SIZE is not allowed.
  */
bool QIOWiimote::writeReport(const char * data, qint64 max_size)
{
	Q_ASSERT_X(max_size <= MAX_REPORT_SIZE, "QIOWiimote::writeReport", "A report can't have a size greater than 22.");
	qDebug() << "Writing the report " << QByteArray(data, max_size).toHex() << " to the wiimote.";

	char data_copy[MAX_REPORT_SIZE];

	for(register int i = 0; i < max_size; i++) data_copy[i] = data[i];
	/* Pad the rest of the report with zeroes just to be sure. */
	for(register int i = max_size; i < MAX_REPORT_SIZE; i++) data_copy[i] = 0;
	return (HidD_SetOutputReport(this->wiimote_handle, data_copy, MAX_REPORT_SIZE) == TRUE);
}

/**
  * Sends a report to the Wiimote. Writing is done synchronously.
  * Overloaded function.
  * @param data Report that will be sent to the wiimote.
  */
bool QIOWiimote::writeReport(QByteArray data)
{
	return this->writeReport(data.constData(), data.size());
}

/* Private functions */

/**
  * Starts asynchronous read of data.
  */
void QIOWiimote::readBegin()
{
	ReadFileEx(this->wiimote_handle,
			   this->read_buffer,
			   MAX_REPORT_SIZE,
			   (LPOVERLAPPED)this->overlapped,
			   QIOWiimote::readCallback);
}

/**
  * This callback is called whenever a read operation is finished.
  * @param error_code The I/O completion status. This parameter can be one of the system error codes.
  * @param bytes_transferred The number of bytes transferred. If an error occurs, this parameter is zero.
  * @param overlapped A pointer to the OVERLAPPED structure specified by the asynchronous I/O function.
  */
void CALLBACK QIOWiimote::readCallback(DWORD error_code,
									   DWORD bytes_transferred,
									   LPOVERLAPPED overlapped)
{
	/* Get the QIOWiimote object that should receive this report. */
	QIOWiimote * this_io = ((OverlappedQIOWiimote *)overlapped)->iowiimote;
	this_io->readEnd(error_code, bytes_transferred);
}

/**
  * Takes the raw report and makes it ready for processing.
  * The report format is time|report.
  * @param error_code The I/O completion status. This parameter can be one of the system error codes.
  * @param bytes_transferred The number of bytes transferred. If an error occurs, this parameter is zero.
  */
void QIOWiimote::readEnd(DWORD error_code, DWORD bytes_transferred)
{
	if (error_code == 0) {
		QWiimoteReport new_report;
		/* Set the time to the current time. */
		new_report.time = QTime::currentTime();
		/* Set the data. */
		new_report.data = QByteArray::fromRawData(this->read_buffer, bytes_transferred);
		/* Schedule the next read. */
		this->readBegin();
		/* Emit this report. */
		emit this->reportReady(new_report);
	} else {
		emit this->reportError();
	}
}
