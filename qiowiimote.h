/**
  * @file qiowiimote.h
  * Header file for the QIOWiimote class.
  * QIOWiimote handles direct communication with the wiimote.
  */

#ifndef QIOWIIMOTE_H
#define QIOWIIMOTE_H

#include <QObject>
#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include "qwiimotereport.h"

#define MAX_REPORT_SIZE 22 ///< Maximum size of a report.

class QIOWiimote;

/**
  * Struct used with asynchronous reading from the wiimote.
  */
struct OverlappedQIOWiimote {
	OVERLAPPED overlapped;  ///< Used for asynchronous reading.
	QIOWiimote * iowiimote; ///< Pointer to the QIOWiimote instance that should receive this reading.
};

/**
  * Class that handles asynchronous reading and synchronous writing to a wiimote.
  * @see #OverlappedQIOWiimote.
  *
  * @todo Using more than one instance of this class is untested.
  */
class QIOWiimote : public QObject
{
	Q_OBJECT
public:
	QIOWiimote(QObject * parent = NULL);
	~QIOWiimote();
	bool open();
	bool isOpened() { return this->opened; }
	void close();
	bool writeReport(const char * data, const qint64 max_size);
	bool writeReport(const QByteArray data);

private:
	static const quint16 WIIMOTE_VENDOR_ID;  ///< Wiimote vendor ID.
	static const quint16 WIIMOTE_PRODUCT_ID; ///< Wiimote product ID.
	HANDLE wiimote_handle;                   ///< Handle to send / receive data from the wiimote.
	char read_buffer[MAX_REPORT_SIZE];       ///< Buffer used for asynchronous read.
	bool opened;                             ///< True only if the connection is opened.

	void readBegin();
	static void CALLBACK readCallback(DWORD error_code, DWORD bytes_transferred, LPOVERLAPPED overlapped);
	void readEnd(DWORD error_code, DWORD bytes_transferred);
	OverlappedQIOWiimote * overlapped;

signals:
	/**
	  * This signal is emmited whenever a new report is ready for being processed.
	  */
	void reportReady(QWiimoteReport report);
	/**
	  * This signal is emmited whenever an error is found at a received report.
	  */
	void reportError();
};

#endif // QIOWIIMOTE_H
