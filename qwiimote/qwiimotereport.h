/*
 * This file is part of QWiimote.
 *
 * QWiimote is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QWiimote is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QWiimote. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file qwiimotereport.h
 *
 * Header file for the QWiimoteReport struct.
 *
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
