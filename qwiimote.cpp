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
    connect(&io_wiimote, SIGNAL(reportReady(QWiimoteReport)), this, SLOT(getReport(QWiimoteReport)));
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
        this->requestCalibrationData();
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

/**
  * @todo Continuous reporting is not set off.
  */
void QWiimote::setDataTypes(QWiimote::DataTypes new_data_types)
{
    this->data_types = new_data_types;
    char report_type;
    char report_mode;
    if (new_data_types == QWiimote::AccelerometerData) {
        report_type = 0x04; // Continuous reporting required.
        report_mode = 0x31;
    }
    else {
        report_type = 0x00; // Continuous reporting not required.
        report_mode = 0x30;
    }
    char data_report[] = {0x12, report_type, report_mode};
    this->io_wiimote.writeReport(data_report, 3);
}

QWiimote::WiimoteButtons QWiimote::buttonData() const
{
    return this->button_data;
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

bool QWiimote::requestCalibrationData()
{
    char data_report[] =
                        {
                            0x17, //Report type
                            0x00, //Read from the EEPROM
                            0x00,
                            0x00,
                            0x16, //Memory position
                            0x00,
                            0x08, //Data size
                        };

    return this->io_wiimote.writeReport(data_report, 7);
}

/**
  * Gets a report from the wiimote.
  * For now, the function ignores all pending reports besides the last one.
  * @todo Missing bit in x_acceleration.
  */
void QWiimote::getReport(QWiimoteReport report)
{
    qDebug() << "Receiving the report " << report.data.toHex() << " from the wiimote.";
    if (report.data[0] != (char)0x21) {
        this->button_data = QFlag(report.data[2] * 0x100 + report.data[1]);
        if (report.data[0] == (char)0x31) {


            if (this->data_types == QWiimote::AccelerometerData) {
                this->x_acceleration =  (report.data[3] & 0xFF) * 4;
                this->x_acceleration += (report.data[1] & 0x60) >> 5;
                this->y_acceleration =  (report.data[4] & 0xFF) * 4;
                this->y_acceleration += (report.data[2] & 0x20) >> 4;
                this->z_acceleration =  (report.data[5] & 0xFF) * 4;
                this->y_acceleration += (report.data[2] & 0x40) >> 5;
            } else {
                this->x_acceleration = 0;
                this->y_acceleration = 0;
                this->z_acceleration = 0;
            }
        }
        emit updatedState();
    }
}
