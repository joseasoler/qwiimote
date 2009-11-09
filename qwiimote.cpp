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
    connect(io_wiimote, SIGNAL(reportReady()), this, SLOT(getReport()));
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

void QWiimote::getReport()
{

}
