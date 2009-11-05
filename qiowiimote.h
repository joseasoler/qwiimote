#ifndef QIOWIIMOTE_H
#define QIOWIIMOTE_H

#include <QIODevice>
#include <windows.h>
#include <setupapi.h>
#include <ddk/hidsdi.h>
#include "debugcheck.h"

class QIOWiimote : public QIODevice
{
public:
    QIOWiimote();
    QIOWiimote(QObject * parent);
    ~QIOWiimote();
    bool open(OpenMode mode = QIODevice::ReadWrite);
    void close();

    /**
      * @todo Implement these functions
      */
    /*
    bool waitForBytesWritten(int msecs);
    bool waitForReadyRead(int msecs);
    */

    /**
      * @todo I doubt this function will have any use, but it should be implemented.
      */
    bool canReadLine() const { return false; }

    /**
      * The communication with the wiimote is sequential.
      */
    bool isSequential() const { return true; }

    /**
      * There is no concept of position in sequential devices.
      */
    qint64 pos() const { return 0; }

    /**
      * Reset does not have to do anything.
      */
    bool reset() { return true; }

    /**
      * Sequential devices do nothing on a seek.
      */
    bool seek(qint64 pos) { return false; }

protected:
    qint64 readData(char * data, qint64 max_size);
    qint64 readLineData(char * data, qint64 max_size);
    qint64 writeData(const char * data, qint64 max_size);

private:
    static const quint16 WIIMOTE_VENDOR_ID;  ///< Wiimote vendor ID
    static const quint16 WIIMOTE_PRODUCT_ID; ///< Wiimote product ID

    HANDLE wiimote_handle;                   ///< Handle to send / receive data from the wiimote
    bool sendData(char * data, qint64 max_size);
};

#endif // QIOWIIMOTE_H
