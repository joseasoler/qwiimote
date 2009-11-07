/**
  * @file qiowiimote.h
  * Header file for the QIOWiimote class.
  * QIOWiimote handles direct communication with the wiimote.
  */

#ifndef QIOWIIMOTE_H
#define QIOWIIMOTE_H

#include <QObject>
#include <QQueue>
#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include "qwiimotereport.h"

class QIOWiimote;

/**
  * Struct used with asynchronous reading from the wiimote.
  */
struct OverlappedQIOWiimote {
    OVERLAPPED overlapped;
    QIOWiimote * iowiimote;
};

class QIOWiimote : public QObject
{
    Q_OBJECT
public:
    QIOWiimote(QObject * parent = NULL);
    ~QIOWiimote();
    bool open();
    bool isOpened() { return this->opened; }
    void close();
    bool writeReport(const char * data, qint64 max_size);
    bool writeReport(QByteArray data);
    int  numWaitingReports() const { return this->report_queue.size(); }
    QWiimoteReport getReport();

private:
    static const quint16 WIIMOTE_VENDOR_ID;  ///< Wiimote vendor ID.
    static const quint16 WIIMOTE_PRODUCT_ID; ///< Wiimote product ID.
    HANDLE wiimote_handle;                   ///< Handle to send / receive data from the wiimote.
    char read_buffer[22];                    ///< Buffer used for asynchronous read.
    bool opened;                             ///< True only if the connection is opened.
    QQueue<QWiimoteReport> report_queue;     ///< Reports ready to be processed.

    void readBegin();
    static void CALLBACK readCallback(DWORD error_code, DWORD bytes_transferred, LPOVERLAPPED overlapped);
    void readEnd(DWORD error_code, DWORD bytes_transferred);
    OverlappedQIOWiimote * overlapped;

signals:
    /**
      * This signal is emmited whenever a new report is ready for being processed.
      */
    void reportReady();
    /**
      * This signal is emmited when there is an error at a received report.
      */
    void reportError();
};

#endif // QIOWIIMOTE_H
