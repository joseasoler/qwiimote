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
#include <QTimer>
#include <QQuaternion>
#include "qiowiimote.h"

class QWiimote : public QObject
{
    Q_OBJECT
public:
    enum DataType {
        AccelerometerData = 0x01,
        MotionPlusData    = 0x02, //MotionPlus always activates AccelerometerData.
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

    enum MotionPlusState {
        MotionPlusInactive,                         ///< Initial state.
        MotionPlusActivated,                        ///< The MotionPlus has been activated by the user, but not by hardware.
        MotionPlusWorking,                          ///< The MotionPlus is fully working.
    };

    Q_DECLARE_FLAGS(MotionPlusStates, MotionPlusState)

    enum WiimoteLed {
        Rumble = 0x01,
        Led1   = 0x10,
        Led2   = 0x20,
        Led3   = 0x40,
        Led4   = 0x80,
    };

    Q_DECLARE_FLAGS(WiimoteLeds, WiimoteLed)

    QWiimote(QObject * parent = NULL);
    ~QWiimote();
    bool start(QWiimote::DataTypes new_data_types = 0);
    void stop();

    void setDataTypes(QWiimote::DataTypes new_data_types);
    void setLeds(QWiimote::WiimoteLeds leds);

    QWiimote::DataTypes dataTypes() const;
    QWiimote::WiimoteButtons buttonData() const;
    QWiimote::WiimoteLeds leds() const;

    quint16 rawAccelerationX() const;
    quint16 rawAccelerationY() const;
    quint16 rawAccelerationZ() const;
    qreal accelerationX() const;
    qreal accelerationY() const;
    qreal accelerationZ() const;

    quint8 batteryLevel() const;
    bool batteryEmpty() const;

signals:
    void updatedButtons();
    void updatedAcceleration();
    void updatedBattery();
    void emptyBattery();
    void motionPlusState(bool);
    void updatedMotionPlus();
private:
    bool requestCalibrationData();
    void resetAccelerationData();
    void requestStatusReport();
    void enableMotionPlus();
    void disableMotionPlus();

    QIOWiimote io_wiimote;                ///< Instance of QIOWiimote used to send / receive wiimote data.
    char send_buffer[22];                 ///< Buffer used to send reports to the wiimote.

    QWiimote::DataTypes data_types;       ///< Current data type status.
    QWiimote::WiimoteButtons button_data; ///< Button status.
    QWiimote::WiimoteLeds led_data;       ///< Led status.

    quint16 x_acceleration;               ///< Raw acceleration in the x axis.
    quint16 y_acceleration;               ///< Raw acceleration in the y axis.
    quint16 z_acceleration;               ///< Raw acceleration in the z axis.
    quint16 x_zero_acceleration;          ///< Zero position for the x axis.
    quint16 y_zero_acceleration;          ///< Zero position for the y axis.
    quint16 z_zero_acceleration;          ///< Zero position for the z axis.
    quint16 x_gravity;                    ///< Gravity calibration for the x axis.
    quint16 y_gravity;                    ///< Gravity calibration for the y axis.
    quint16 z_gravity;                    ///< Gravity calibration for the z axis.
    qreal   x_calibrated_acceleration;    ///< Acceleration in the x axis.
    qreal   y_calibrated_acceleration;    ///< Acceleration in the y axis.
    qreal   z_calibrated_acceleration;    ///< Acceleration in the z axis.

    QTimer * motionplus_polling;          ///< Timer that checks the MotionPlus state.
    QWiimote::MotionPlusStates motionplus_state;

    QTimer * status_polling;              ///< Timer that polls wiimote status reports.
    bool status_requested;                ///< True if a status report is expected.
    quint8 battery_level;                 ///< Battery level of the wiimote.
    bool battery_empty;                   ///< True if the battery is almost empty.


private slots:
    void getCalibrationReport(QWiimoteReport report);
    void getReport(QWiimoteReport report);
    void pollMotionPlus();
    void pollStatusReport();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::DataTypes)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::WiimoteButtons)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::WiimoteLeds)

Q_DECLARE_OPERATORS_FOR_FLAGS(QWiimote::MotionPlusStates)

#endif // QWIIMOTE_H
