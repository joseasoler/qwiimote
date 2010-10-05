/**
  * @file qwiimotereport.h
  * Header file for the QWiimoteReport struct.
  * This data structure stores a report along with its arrival time.
  */

#ifndef QWIIMOTEREPORT_H
#define QWIIMOTEREPORT_H

#include <QByteArray>
#include "qprecisetime.h"

/**
  * Stores a received Wiimote report until it is processed.
  * QTime can't be used because its precision under Windows systems is too low (10-16 milliseconds).
  * @see #QIOWiimote and #QPreciseTime.
  */
struct QWiimoteReport {
	QPreciseTime time; ///< Time of arrival of the report.
	QByteArray data;   ///< Data of the report.
};

#endif // QWIIMOTEREPORT_H
