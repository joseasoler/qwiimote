#include "qprecisetime.h"

/* Public functions. */

/**
  * Creates a new #QPreciseTime object.
  */
QPreciseTime::QPreciseTime()
{
	this->starting_time = -1;
}

/**
  * Allows to know the elapsed time since this #QPreciseTime instance was started.
  *
  * @return Number of milliseconds elapsed since start.
  */
qreal QPreciseTime::elapsed()
{
	__int64 ticks_per_second;
	QueryPerformanceFrequency((LARGE_INTEGER *) &ticks_per_second);
	qreal ticks_per_millisecond = ((qreal)ticks_per_second) / 1000.0;

	return (double)(QPreciseTime::currentTime().starting_time - this->starting_time) / ticks_per_millisecond;
}

/**
  * Gets the current time.
  *
  * @return #QPreciseTime starting at the current time.
  */
QPreciseTime QPreciseTime::currentTime()
{
	QPreciseTime current_time;
	QueryPerformanceCounter((LARGE_INTEGER *) &current_time.starting_time);
	return current_time;
}

/**
  * Copies another #QPreciseTime instance.
  * @param other Instance to be copied.
  *
  * @return Reference to the copy.
  */
QPreciseTime &QPreciseTime::operator=(const QPreciseTime &other)
{
	 this->starting_time = other.starting_time;
	 return *this;
}
