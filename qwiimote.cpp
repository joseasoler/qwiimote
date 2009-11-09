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

void QWiimote::setDataTypes(QWiimote::DataTypes new_data_types)
{
    this->data_types = new_data_types;
    char report_type = 0x00;
    if (new_data_types == QWiimote::AccelerometerData)  report_type = 0x31;
    char report_mode[] = {0x12, 0x00, report_type};
    this->io_wiimote.writeReport(report_mode, 3);
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
    }

    if (this->data_types == QFlag(0)) {
        this->button_data = QFlag(report.data[2] * 0x100 + report.data[1]);
        emit updatedState();
    }

    /*
    QWiimoteReport report;


    if (this->data_types == 0x00) {
        this->button_data = QFlag(report.data[2] * 0x100 + report.data[1]);
        emit updatedState();
    }
    */
}
