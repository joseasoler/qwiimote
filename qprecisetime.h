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
