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

    enum WiimoteButton {
        ButtonLeft  = 0x0001,
        ButtonRight = 0x0002,
        ButtonDown  = 0x0004,
        ButtonUp    = 0x0008,
        ButtonPlus  = 0x0010,
        ButtonTwo   = 0x0100,
        ButtonOne   = 0x0200,
        ButtonB     = 0x0400,
        ButtonA     = 0x0800,
        ButtonMinus = 0x1000,
        ButtonHome  = 0x8000,
    };

    Q_DECLARE_FLAGS(WiimoteButtons, WiimoteButton)

    QWiimote(QObject * parent = NULL);
    ~QWiimote();
    bool start(QWiimote::DataTypes new_data_types = 0);
    void stop();
    QWiimote::DataTypes dataTypes() const;
    void setDataTypes(QWiimote::DataTypes new_data_types);

    QWiimote::WiimoteButtons buttonData() const;

signals:
    void updatedState();
private:
    QIOWiimote io_wiimote;
    QWiimote::DataTypes data_types;
    QWiimote::WiimoteButtons button_data;

private slots:
    void getReport();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::DataTypes)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::WiimoteButtons)

#endif // QWIIMOTE_H
