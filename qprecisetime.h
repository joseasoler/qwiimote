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

#ifndef QPRECISETIME_H
#define QPRECISETIME_H

#include <QtGlobal>
#include <windows.h>

/**
  * This class is a rewrite of QTime which has increased precision.
  */
class QPreciseTime
{
public:
	QPreciseTime();
	qreal elapsed();
	static QPreciseTime currentTime();
	QPreciseTime &operator=(const QPreciseTime &other);

	/**
	  * Equality operator.
	  * @param other Instance to be compared.
	  *
	  * @return Comparison result.
	  */
	bool operator==(const QPreciseTime &other) const { return starting_time == other.starting_time; }

	/**
	  * Unequality operator.
	  * @param other Instance to be compared.
	  *
	  * @return Comparison result.
	  */
	bool operator!=(const QPreciseTime &other) const { return starting_time != other.starting_time; }

	/**
	  * Lesser than operator.
	  * @param other Instance to be compared.
	  *
	  * @return Comparison result.
	  */
	bool operator< (const QPreciseTime &other) const { return starting_time <  other.starting_time; }

	/**
	  * Lesser or equal than operator.
	  * @param other Instance to be compared.
	  *
	  * @return Comparison result.
	  */
	bool operator<=(const QPreciseTime &other) const { return starting_time <= other.starting_time; }

	/**
	  * Greater than operator.
	  * @param other Instance to be compared.
	  *
	  * @return Comparison result.
	  */
	bool operator> (const QPreciseTime &other) const { return starting_time >  other.starting_time; }

	/**
	  * Greater or equal than operator.
	  * @param other Instance to be compared.
	  *
	  * @return Comparison result.
	  */
	bool operator>=(const QPreciseTime &other) const { return starting_time >= other.starting_time; }
private:
	__int64 starting_time; ///< Starting time for this instance.
};

#endif // QPRECISETIME_H
