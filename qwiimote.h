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
public:
    QWiimote();
};

#endif // QWIIMOTE_H
