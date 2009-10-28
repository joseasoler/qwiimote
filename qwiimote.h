/**
  * @file
  * Header file for the QWiimote class
  * QWiimote is an API for the wiimote following the Qt Object model.
  * http://qt.nokia.com/doc/4.5/object.html
  */

#ifndef QWIIMOTE_H
#define QWIIMOTE_H

#include <windows.h>
#include <QObject>
#include "debugcheck.h"

class QWiimote : public QObject
{
protected:
    HANDLE wiimote_handle;                   ///< Handle to send / receive data from the wiimote

    static const quint16 WIIMOTE_VENDOR_ID;  ///< Wiimote vendor ID
    static const quint16 WIIMOTE_PRODUCT_ID; ///< Wiimote product ID
public:
    QWiimote();
    bool findWiimote();
};

#endif // QWIIMOTE_H
