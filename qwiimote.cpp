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
    connect(&io_wiimote, SIGNAL(reportReady()), this, SLOT(getReport()));
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

/**
  * Gets a report from the wiimote.
  * For now, the function ignores all pending reports besides the last one.
  * @todo Right now the function ignores all pending reports besides the last one. If the report type is different than the expected type, something should be done.
  */
void QWiimote::getReport()
{
    QWiimoteReport report;

    // Ignore all reports besides the last one.
    while (this->io_wiimote.numWaitingReports() > 0) {
        report = this->io_wiimote.getReport();
        qDebug() << "Receiving the report " << report.data.toHex() << " from the wiimote.";
    }
    qDebug() << "This report will be processed.";

    this->button_data = QFlag(report.data[2] * 0x100 + report.data[1]);

    if (this->data_types == QWiimote::AccelerometerData) {
        //this->x_acceleration = (report.data[3] * 0x100 + report.data[4]);
        //this->y_acceleration = (report.data[5] * 0x100 + report.data[6]);
        //this->z_acceleration = (report.data[6] * 0x100 + report.data[7]);
    }

    emit updatedState();
    /*
    QWiimoteReport report;


    if (this->data_types == 0x00) {
        this->button_data = QFlag(report.data[2] * 0x100 + report.data[1]);
        emit updatedState();
    }
    */
}
