#include "Calendar.h"
#include <cstring>
#include <cstdlib>

#define maxLineSize	80

bool
Calendar::roll(date& dt)
{
	while (!isBusDay(dt)) {
		dt++;
	}
	return SUCCESS;
}

bool
Calendar::isBusDay(const date& dt)
{	
	return (dt.isWeekDay() && (holidays.find(dt) == holidays.end()));
}

bool
Calendar::addBusDays(date& dt, int count)
{
	for (int i = 0; i < count; i++) {
		roll(dt);
	}
	return SUCCESS;
}

bool
Calendar::addMonths(date& dt, int count)
{
	// Check if original date is last business day of the month
	date checkDate = date(dt);
	do {
		checkDate++;
	} while (!isBusDay(checkDate));
	bool isEom = (checkDate.month() != dt.month());

	// Add months
	dt.addMonths(count);
	int origMonth = dt.month();

	if (isEom) { // If need to add to next end of month, roll to one further month after adding and then roll back
		while (dt.month() == origMonth) {
			dt++;
		}
	}
	else {  // else just roll til next business day
		while (!isBusDay(dt)) {
			dt++;
		}
	}

	// If not business day or rolled to next month, roll back to last busienss day
	while (!isBusDay(dt) || (origMonth != dt.month())) {
		dt--;
	}
	return SUCCESS;
}

bool
Calendar::addYears(date& dt, int count)
{	
	addMonths(dt, count * 12);
	return SUCCESS;
}

bool
Calendar::addHoliday(date& dt)
{	
	holidays.insert(dt);
	return SUCCESS;
}

bool
Calendar::removeHoliday(date& dt)
{
	holidays.erase(dt);
	return SUCCESS;
}

MMCalendar::MMCalendar(string filename, string mkt) :market(mkt) {
	ifstream holFile(filename);
	string oneLine;
	while (holFile >> oneLine) {
		if (oneLine.find(mkt) == string::npos) continue;
		string dtStr = oneLine.substr(0, oneLine.find(","));
		date dt = date(std::stoi(dtStr.substr(0, 4)), std::stoi(dtStr.substr(4, 2)), std::stoi(dtStr.substr(6, 2)));
		holidays.insert(dt);
	}
	holFile.close();
}

bool
MMCalendar::roll(date& dt)
{
	return SUCCESS;
}

bool
MMCalendar::isIMMDay(date& dt) {
	return false;
}
date&
MMCalendar::nextIMMDay(date& dt) {
	return dt;
}