/**
  * @file qwiimote.h
  * Header file for the QWiimote class.
  * QWiimote is an API for the wiimote which follows the Qt Object model.
  * The QWiimote class mantains the state of a single wiimote.
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
    quint16 rawAccelerationX() const;
    quint16 rawAccelerationY() const;
    quint16 rawAccelerationZ() const;

signals:
    void updatedState();
private:
    bool requestCalibrationData();
    void resetAccelerationData();

    QIOWiimote io_wiimote;                ///< Instance of QIOWiimote used to send / receive wiimote data.
    char send_buffer[22];                 ///< Buffer used to send reports to the wiimote.

    QWiimote::DataTypes data_types;       ///< Current data type status.
    QWiimote::WiimoteButtons button_data; ///< Button status.
    quint16 x_acceleration;               ///< Raw acceleration in the x axis.
    quint16 y_acceleration;               ///< Raw acceleration in the y axis.
    quint16 z_acceleration;               ///< Raw acceleration in the z axis.

    quint16 x_zero_acceleration;          ///< Zero position for the x axis.
    quint16 y_zero_acceleration;          ///< Zero position for the y axis.
    quint16 z_zero_acceleration;          ///< Zero position for the z axis.

    quint16 x_gravity;                    ///< Gravity calibration for the x axis.
    quint16 y_gravity;                    ///< Gravity calibration for the y axis.
    quint16 z_gravity;                    ///< Gravity calibration for the z axis.

private slots:
    void getReport(QWiimoteReport report);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::DataTypes)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::WiimoteButtons)

#endif // QWIIMOTE_H
