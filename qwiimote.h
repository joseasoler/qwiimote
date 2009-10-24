/**
  * @file
  * Header file for the QWiimote class
  * QWiimote is an API for the wiimote following the Qt Object model.
  * http://qt.nokia.com/doc/4.5/object.html
  */

#ifndef QWIIMOTE_H
#define QWIIMOTE_H

#include <QObject>

class QWiimote : public QObject
{
protected:
    static const quint16 WIIMOTE_VENDOR_ID;
    static const quint16 WIIMOTE_PRODUCT_ID;
public:
    QWiimote();
    bool findWiimote();
};

#endif // QWIIMOTE_H