#ifndef QPRECISETIME_H
#define QPRECISETIME_H

#include <QtGlobal>
#include <windows.h>

class QPreciseTime
{
public:
	QPreciseTime();
	qreal elapsed();
	static QPreciseTime currentTime();
	QPreciseTime &operator=(const QPreciseTime &other);

	bool operator==(const QPreciseTime &other) const { return starting_time == other.starting_time; }
	bool operator!=(const QPreciseTime &other) const { return starting_time != other.starting_time; }
	bool operator< (const QPreciseTime &other) const { return starting_time <  other.starting_time; }
	bool operator<=(const QPreciseTime &other) const { return starting_time <= other.starting_time; }
	bool operator> (const QPreciseTime &other) const { return starting_time >  other.starting_time; }
	bool operator>=(const QPreciseTime &other) const { return starting_time >= other.starting_time; }
private:
	__int64 starting_time; ///< Starting time for this instance.
};

#endif // QPRECISETIME_H
