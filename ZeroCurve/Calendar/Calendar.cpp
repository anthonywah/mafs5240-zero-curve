#include "Calendar.h"
#include <cstring>
#include <cstdlib>

#define maxLineSize	80

bool
Calendar::roll(date& dt)
{
	// keep adding days if dt is not bus day
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
		// add 1 day, roll to next bus day if dt is not a bus day
		dt++;	
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
	// if dt is bus day, do nothing
	if (isBusDay(dt)) {
		return SUCCESS;
	}
	date dt_copy = date(dt.year(), dt.month(), dt.day());
	addBusDays(dt_copy, 1);
	bool isLastTradeDay = (dt_copy.month() != dt.month());
	if (isLastTradeDay) {
		// dt is the last trading day, roll backward
		dt--;
		while (!isBusDay(dt)) {
			dt--;
		}
	}
	else {
		addBusDays(dt, 1);
	}
	return SUCCESS;
}

bool
MMCalendar::isIMMDay(date& dt) {
	// if not march, june, sept, dec simply return false
	if (dt.month() % 3 != 0) { 
		return false; 
	} 
	// if it is, check if it is same as the imm of the month
	date som = date(dt.year(), dt.month(), 1);
	return dt == nextIMMDay(som);
}
date&
MMCalendar::nextIMMDay(date& dt) {
	if (dt.month() % 3 == 0) {
		// if it is march, june, sept, dec,
		// find the IMM day of the month


		// third wednesday is always the first wednesday on or after day 15 of the month
		date day15(dt.year(), dt.month(), 15);
		date imm_day = day15 + (7 + 3 - day15.dayOfWeek()) % 7; // this gets the 1st wednesday after day 15 of the month
		roll(imm_day); // roll to the nearest bus day if it is not
		// if the imm day has not passed, return it
		if (dt < imm_day) { 
			return imm_day; 
		}
	}
	// if it is not march, june, sept, dec, or the imm day of the month has passed
	// it is same as the next IMM day of the day 1 of next month
	date som(dt.year(), dt.month(), 1);
	addMonths(som, 1);
	return nextIMMDay(som);
}