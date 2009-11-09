/**
  * @file qwiimote.h
  * Header file for the QWiimote class.
  * QWiimote is an API for the wiimote which follows the Qt Object model.
  * http://qt.nokia.com/doc/4.5/object.html
  */

#ifndef QWIIMOTE_H
#define QWIIMOTE_H

#include <QObject>
#include <QFlags>
#include "qiowiimote.h"

class QWiimote : public QObject
{
    Q_OBJECT
public:
    enum DataType {
        AccelerometerData = 1,
    };
    Q_DECLARE_FLAGS(DataTypes, DataType)
    QWiimote(QObject * parent = NULL);
    ~QWiimote();
    bool start(QWiimote::DataTypes new_data_types = 0);
    void stop();
    QWiimote::DataTypes dataTypes() const;
    void setDataTypes(QWiimote::DataTypes new_data_types);
private:
    QIOWiimote io_wiimote;
    QWiimote::DataTypes data_types;

private slots:
    void getReport();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::DataTypes)

#endif // QWIIMOTE_H
