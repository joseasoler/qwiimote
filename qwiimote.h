/**
  * @file qwiimote.h
  * Header file for the QWiimote class.
  * QWiimote is an API for the wiimote following the Qt Object model.
  * http://qt.nokia.com/doc/4.5/object.html
  */

#ifndef QWIIMOTE_H
#define QWIIMOTE_H

#include <QObject>
#include "qiowiimote.h"

class QWiimote : public QObject
{
    Q_OBJECT
public:
    QWiimote(QObject * parent = NULL);
    ~QWiimote();
private:
    QIOWiimote io_wiimote;
};

#endif // QWIIMOTE_H
