/**
  * @file qwiimotereport.h
  * Header file for the QWiimoteReport struct.
  * This data structure stores a report along with its arrival time.
  */

#ifndef QWIIMOTEREPORT_H
#define QWIIMOTEREPORT_H

#include <QByteArray>
#include <QTime>

/**
  * Stores a received Wiimote report until it is processed.
  * @see #QIOWiimote.
  */
struct QWiimoteReport {
	QTime time;      ///< Time of arrival of the report.
	QByteArray data; ///< Data of the report.
};

#endif // QWIIMOTEREPORT_H
