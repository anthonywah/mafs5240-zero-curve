#include "Calendar.h"
#include <cstring>
#include <cstdlib>

#define maxLineSize	80

bool
Calendar::roll(date& dt)
{
	return FAILURE;
}

bool
Calendar::isBusDay(const date& dt)
{
	return false;
}

bool
Calendar::addBusDays(date& dt, int count)
{
	return FAILURE;
}

bool
Calendar::addMonths(date& dt, int count)
{
	return FAILURE;
}

bool
Calendar::addYears(date& dt, int count)
{
	return FAILURE;
}

bool
Calendar::addHoliday(date& dt)
{
	return FAILURE;
}

bool
Calendar::removeHoliday(date& dt)
{
	return FAILURE;
}

MMCalendar::MMCalendar(string filename, string mkt) :market(mkt) {
}

bool
MMCalendar::roll(date& dt)
{
	return FAILURE;
}

bool
MMCalendar::isIMMDay(date& dt) {
	return false;
}
date&
MMCalendar::nextIMMDay(date& dt) {
	return dt;
}