/**
  * @file qwiimote.cpp
  * Source file for the QWiimote class.
  */

#include "qwiimote.h"
#include "debugcheck.h"

/**
  * Creates a new QWiimote instance.
  */
QWiimote::QWiimote(QObject * parent) : QObject(parent), io_wiimote(this)
{
    data_types = 0;
}

QWiimote::~QWiimote()
{
}

/**
  * The QWiimote starts working.
  * @todo This is just a preliminary version of start.
  * @return true if the QWiimote started correctly.
  */
bool QWiimote::start(QWiimote::DataTypes new_data_types)
{
    if (this->io_wiimote.open()) {
        this->setDataTypes(new_data_types);
        // All reports are ignored until we receive the calibration data.
        connect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getCalibrationReport(QWiimoteReport)));
        this->requestCalibrationData();
        this->motionplus_plugged = false;
        this->motionplus_active = false;
        return true;
    }

    return false;
}

/**
  * The QWiimote stops working and disconnects.
  * @todo This is just a preliminary version of stop.
  */
void QWiimote::stop()
{
    this->io_wiimote.close();
}

QWiimote::DataTypes QWiimote::dataTypes() const
{
    return this->data_types;
}

void QWiimote::setDataTypes(QWiimote::DataTypes new_data_types)
{
    this->data_types = new_data_types;
    send_buffer[0] = 0x12;

    if (this->data_types & QWiimote::MotionPlusData) {
        this->data_types &= QWiimote::AccelerometerData; //MotionPlus always activates AccelerometerData.
        send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); // Continuous reporting required.
        send_buffer[2] = 0x35;
    } else if (this->data_types & QWiimote::AccelerometerData) {
        send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); // Continuous reporting required.
        send_buffer[2] = 0x31;
    }
    else {
        send_buffer[1] = 0x00 | (this->led_data & QWiimote::Rumble); // Continuous reporting not required.
        send_buffer[2] = 0x30;
        resetAccelerationData();
    }
    this->io_wiimote.writeReport(send_buffer, 3);
}

void QWiimote::setLeds(QWiimote::WiimoteLeds leds)
{
    this->led_data = leds;
    send_buffer[0] = 0x11;
    send_buffer[1] = leds;
    this->io_wiimote.writeReport(send_buffer, 2);
}

QWiimote::WiimoteButtons QWiimote::buttonData() const
{
    return this->button_data;
}

QWiimote::WiimoteLeds QWiimote::leds() const
{
    return this->led_data;
}

quint16 QWiimote::rawAccelerationX() const
{
    return this->x_acceleration;
}

quint16 QWiimote::rawAccelerationY() const
{
    return this->y_acceleration;
}

quint16 QWiimote::rawAccelerationZ() const
{
    return this->z_acceleration;
}

qreal QWiimote::accelerationX() const
{
    return ((qreal)(this->x_acceleration - this->x_zero_acceleration) / (qreal)this->x_gravity);
}

qreal QWiimote::accelerationY() const
{
    return ((qreal)(this->y_acceleration - this->y_zero_acceleration) / (qreal)this->y_gravity);
}

qreal QWiimote::accelerationZ() const
{
    return ((qreal)(this->z_acceleration - this->z_zero_acceleration) / (qreal)this->z_gravity);
}

bool QWiimote::requestCalibrationData()
{
    send_buffer[0] = 0x17;                                       //Report type
    send_buffer[1] = 0x00 | (this->led_data & QWiimote::Rumble); //Read from the EEPROM
    send_buffer[2] = 0x00;                                       //Memory position
    send_buffer[3] = 0x00;
    send_buffer[4] = 0x16;
    send_buffer[5] = 0x00;                                       //Data size
    send_buffer[6] = 0x08;

    return this->io_wiimote.writeReport(send_buffer, 7);
}

void QWiimote::getCalibrationReport(QWiimoteReport report)
{
    qDebug() << "Receiving the report " << report.data.toHex() << " from the wiimote.";
    if (report.data[0] == (char)0x21) {
        this->x_zero_acceleration = ((report.data[6] & 0xFF) << 2) + ((report.data[9] & 0x30) >> 4);
        this->y_zero_acceleration = ((report.data[7] & 0xFF) << 2) + ((report.data[9] & 0x0C) >> 2);
        this->z_zero_acceleration = ((report.data[8] & 0xFF) << 2) + (report.data[9] & 0x03);
        qDebug() << "X zero acceleration: " << QString::number(this->x_zero_acceleration, 2);
        qDebug() << "Y zero acceleration: " << QString::number(this->y_zero_acceleration, 2);
        qDebug() << "Z zero acceleration: " << QString::number(this->z_zero_acceleration, 2);

        this->x_gravity = ((report.data[10] & 0xFF) << 2) + ((report.data[13] & 0x30) >> 4);
        this->y_gravity = ((report.data[11] & 0xFF) << 2) + ((report.data[13] & 0x0C) >> 2);
        this->z_gravity = ((report.data[12] & 0xFF) << 2) + (report.data[13] & 0x03);
        qDebug() << "X gravity: " << QString::number(this->x_gravity, 2);
        qDebug() << "Y gravity: " << QString::number(this->y_gravity, 2);
        qDebug() << "Z gravity: " << QString::number(this->z_gravity, 2);

        disconnect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getCalibrationReport(QWiimoteReport)));
        connect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getReport(QWiimoteReport)));

        motionplus_polling = new QTimer(this);
        connect(motionplus_polling, SIGNAL(timeout()), this, SLOT(pollMotionPlus()));
        motionplus_polling->start(1000);
    } else {
        qDebug() << "Report ignored.";
        this->requestCalibrationData();
    }
}

/**
  * Gets a report from the wiimote.
  * @todo Check possible report errors. Use a threshold while reporting acceleration changes.
  */
void QWiimote::getReport(QWiimoteReport report)
{
    qDebug() << "Receiving the report " << report.data.toHex() << " from the wiimote.";

    int report_type = report.data[0] & 0xFF;

    switch (report_type) {
        case 0x31: //Acceleration report
            if (this->data_types == QWiimote::AccelerometerData) {
                quint16 x_new, y_new, z_new;
                x_new =  (report.data[3] & 0xFF) * 4;
                x_new += (report.data[1] & 0x60) >> 5;
                y_new =  (report.data[4] & 0xFF) * 4;
                y_new += (report.data[2] & 0x20) >> 4;
                z_new =  (report.data[5] & 0xFF) * 4;
                z_new += (report.data[2] & 0x40) >> 5;

                if ((x_new != this->x_acceleration) ||
                    (y_new != this->y_acceleration) ||
                    (z_new != this->z_acceleration)) {
                    this->x_acceleration = x_new;
                    this->y_acceleration = y_new;
                    this->z_acceleration = z_new;
                    emit this->updatedAcceleration();
                }
            }
        break;
        // Fallthrough
        case 0x21:
            if (((report.data[3] & 0xF0)  != 0xF0) && //There are no errors
                ((report.data[6] & 0xFF)  == 0x00) && //There is a MotionPlus plugged in
                ((report.data[7] & 0xFF)  == 0x00) &&
                ((report.data[8] & 0xFF)  == 0xA6) &&
                ((report.data[9] & 0xFF)  == 0x20) &&
                ((report.data[10] & 0xFF) == 0x00) &&
                ((report.data[11] & 0xFF) == 0x05)) {
                qDebug() << "MotionPlus plugged in.";
                if (!this->motionplus_plugged) emit motionPlusState(true);
                this->motionplus_plugged = true;
            } else {
                qDebug() << "MotionPlus not plugged in.";
                if (!this->motionplus_plugged) emit motionPlusState(false);
                this->motionplus_plugged = false;
            }
        break;
    }

    //Button data is present in every report for now.
    QWiimote::WiimoteButtons button_new = QFlag(report.data[2] * 0x100 + report.data[1]);
    if (this->button_data != button_new) {
        button_data = button_new;
        emit this->updatedButtons();
    }
}

void QWiimote::pollMotionPlus()
{
    send_buffer[0] = 0x17;                                       //Report type
    send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); //Read from the registers
    send_buffer[2] = 0xA6;                                       //Memory position
    send_buffer[3] = 0x00;
    send_buffer[4] = 0xFA;
    send_buffer[5] = 0x00;                                       //Data size
    send_buffer[6] = 0x06;

    this->io_wiimote.writeReport(send_buffer, 7);
}

void QWiimote::resetAccelerationData()
{
    this->x_acceleration = 0;
    this->y_acceleration = 0;
    this->z_acceleration = 0;
    emit this->updatedAcceleration();
}
