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

        // All reports are ignored until the calibration data is received.
        connect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getCalibrationReport(QWiimoteReport)));
        this->requestCalibrationData();
        data_types = 0;
        this->motionplus_state = QWiimote::MotionPlusInactive;
        this->status_requested = false;
        this->battery_level = 0;
        this->battery_empty = false;
        this->motionplus_polling = NULL;

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
    send_buffer[0] = 0x12;

    if (new_data_types & QWiimote::MotionPlusData && this->motionplus_state == QWiimote::MotionPlusInactive) {
        new_data_types |= QWiimote::AccelerometerData; // MotionPlus always activates AccelerometerData.
        this->motionplus_state = QWiimote::MotionPlusActivated;

        if (this->motionplus_polling == NULL) {// Start MotionPlus polling.
            motionplus_polling = new QTimer(this);
            connect(motionplus_polling, SIGNAL(timeout()), this, SLOT(pollMotionPlus()));
            motionplus_polling->start(1000);
            this->pollMotionPlus();
        }
    } else if (!(new_data_types & QWiimote::MotionPlusData) && this->motionplus_state == QWiimote::MotionPlusWorking) {
        this->motionplus_state = QWiimote::MotionPlusInactive;
        if (this->motionplus_polling != NULL) {// Stop MotionPlus polling.
            disconnect(motionplus_polling, SIGNAL(timeout()), this, SLOT(pollMotionPlus()));
            delete motionplus_polling;
            motionplus_polling = NULL;
        }
    }
    this->data_types = new_data_types;
    if (this->data_types & QWiimote::MotionPlusData && this->motionplus_state == QWiimote::MotionPlusWorking) {
        send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); // Continuous reporting required.
        send_buffer[2] = 0x35;
    } else if (this->data_types & QWiimote::AccelerometerData && this->motionplus_state != QWiimote::MotionPlusWorking) {
        send_buffer[1] = 0x04 | (this->led_data & QWiimote::Rumble); // Continuous reporting required.
        send_buffer[2] = 0x31;
    }
    else {
        send_buffer[1] = 0x00 | (this->led_data & QWiimote::Rumble); // Continuous reporting not required.
        send_buffer[2] = 0x30;
        resetAccelerationData();
    }
    qDebug() << "Data types: " << this->data_types;
    this->io_wiimote.writeReport(send_buffer, 3);
}

void QWiimote::setLeds(QWiimote::WiimoteLeds leds)
{
    this->led_data = leds;
    send_buffer[0] = 0x11; // LED report.
    send_buffer[1] = leds; // LED status.
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
    return this->x_calibrated_acceleration;
}

qreal QWiimote::accelerationY() const
{
    return this->y_calibrated_acceleration;
}

qreal QWiimote::accelerationZ() const
{
    return this->y_calibrated_acceleration;
}

quint8 QWiimote::batteryLevel() const
{
    return this->battery_level;
}

bool QWiimote::batteryEmpty() const
{
    return this->battery_empty;
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
        // Get the required calibration values from the report.
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

        // Stop checking only calibration reports.
        disconnect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getCalibrationReport(QWiimoteReport)));
        // Start checking all other reports.
        connect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getReport(QWiimoteReport)));

        // Start status report polling.
        status_polling = new QTimer(this);
        connect(status_polling, SIGNAL(timeout()), this, SLOT(pollStatusReport()));
        status_polling->start(12000);
        this->pollStatusReport();

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
        case 0x35: // Acceleration + Extension report
            // Fallthrough
        case 0x31: // Acceleration report
            if (this->data_types & QWiimote::AccelerometerData) {
                quint16 x_new, y_new, z_new;
                x_new =  (report.data[3] & 0xFF) * 4;
                x_new += (report.data[1] & 0x60) >> 5;
                y_new =  (report.data[4] & 0xFF) * 4;
                y_new += (report.data[2] & 0x20) >> 4;
                z_new =  (report.data[5] & 0xFF) * 4;
                z_new += (report.data[2] & 0x40) >> 5;

                // Process acceleration info only if the new values are different than the old ones.
                if ((x_new != this->x_acceleration) ||
                    (y_new != this->y_acceleration) ||
                    (z_new != this->z_acceleration)) {
                    this->x_acceleration = x_new;
                    this->y_acceleration = y_new;
                    this->z_acceleration = z_new;
                    // Calibrated values
                    this->x_calibrated_acceleration = ((qreal)(this->x_acceleration - this->x_zero_acceleration) /
                                                 (qreal)this->x_gravity);
                    this->y_calibrated_acceleration = ((qreal)(this->y_acceleration - this->y_zero_acceleration) /
                                                 (qreal)this->y_gravity);
                    this->z_calibrated_acceleration = ((qreal)(this->z_acceleration - this->z_zero_acceleration) /
                                                 (qreal)this->z_gravity);
                    emit this->updatedAcceleration();
                }
            }
        break;

    case 0x21: // Read memory data, assumed to be a MotionPlus check.
            if (((report.data[3] & 0xF0)  != 0xF0) && //There are no errors.
                ((report.data[6] & 0xFF)  == 0x00) && //There is a MotionPlus plugged in.
                ((report.data[7] & 0xFF)  == 0x00) &&
                ((report.data[8] & 0xFF)  == 0xA6) &&
                ((report.data[9] & 0xFF)  == 0x20) &&
                ((report.data[10] & 0xFF) == 0x00) &&
                ((report.data[11] & 0xFF) == 0x05)) {
                qDebug() << "MotionPlus plugged in.";
                if (this->motionplus_state == QWiimote::MotionPlusActivated) {
                    this->motionplus_state = QWiimote::MotionPlusWorking;
                    emit motionPlusState(true);
                }
            } else if (this->motionplus_state == QWiimote::MotionPlusWorking) {
                this->motionplus_state = QWiimote::MotionPlusActivated;
                emit motionPlusState(false);
            }
        break;

        case 0x20: // Status report
            quint8 new_battery_level = (report.data[6] & 0xFF);
            bool new_battery_empty = ((report.data[3] & 0x01) == 0x01);
            if ((new_battery_level != this->battery_level) ||
                (new_battery_empty != this->battery_empty)) {
                this->battery_level = new_battery_level;
                qDebug() << "Battery level: " << this->battery_level;
                emit this->updatedBattery();
                if (this->battery_empty) {
                    emit this->emptyBattery();
                }
            }


            // If the status request was not requested, the data reporting mode must be changed.
            if (!this->status_requested) {
                this->setDataTypes(this->data_types);
            }
            this->status_requested = false;

        break;
    }

    //Button data is present in every received report for now.
    QWiimote::WiimoteButtons button_new = QFlag(report.data[2] * 0x100 + report.data[1]);
    if (this->button_data != button_new) {
        button_data = button_new;
        emit this->updatedButtons();
    }
}

void QWiimote::pollMotionPlus()
{
    static char motionplus_buffer[] = {
        0x17, //Report type
        0x04, //Read from the registers
        0xA6, //Memory position
        0x00,
        0xFA,
        0x00, //Data size
        0x06
    };

    motionplus_buffer[1] = 0x04  | (this->led_data & QWiimote::Rumble);

    this->io_wiimote.writeReport(motionplus_buffer, 7);
    this->setDataTypes(this->data_types);
}

void QWiimote::requestStatusReport()
{
    static char statusreport_buffer[] = {
        0x15,
        0x00
    };

    statusreport_buffer[1] = this->led_data & QWiimote::Rumble;

    this->io_wiimote.writeReport(statusreport_buffer, 2);
}

void QWiimote::pollStatusReport()
{
    this->requestStatusReport();
    this->status_requested = true;
}

void QWiimote::resetAccelerationData()
{
    this->x_acceleration = 0;
    this->y_acceleration = 0;
    this->z_acceleration = 0;
    this->x_calibrated_acceleration = 0;
    this->y_calibrated_acceleration = 0;
    this->z_calibrated_acceleration = 0;
    emit this->updatedAcceleration();
}


void QWiimote::enableMotionPlus()
{
    // Write 0x04 to register 0xA600FE.
    static char activate_buffer[] = {
        0x16, // Report type
        0x04, // Write to the registers
        0xA6, // Memory position
        0x00,
        0xFE,
        0x01, // Data size
        0x04, // Activate MotionPlus
        0x00, // Padding
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    };

    activate_buffer[1] = 0x04  | (this->led_data & QWiimote::Rumble);

    this->io_wiimote.writeReport(activate_buffer, 22);

    this->requestStatusReport();
}
