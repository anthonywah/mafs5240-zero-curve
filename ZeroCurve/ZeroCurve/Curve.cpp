#include "Curve.h"
#include <cstring>

Curve::Curve(long baseDt, int spot, MMCalendar* pMMCal_,
	CashInput* cin, FuturesInput* fin, SwapsInput* sin)
	: m_baseDate(baseDt), m_daysToSpot(spot), m_pMMCal(pMMCal_),
	m_pCashInput(cin), m_pFuturesInput(fin), m_pSwapsInput(sin) {
	initProcess();
	processCash();
	processFutures();
	processSwaps();
}

KeyPoint
Curve::retrieveKeyPoint(KeyPoints::const_iterator ki) {
	if (ki == m_keyPoints.end())
		return make_pair(0L, 0.0);
	else
		return *ki;
}

void
Curve::initProcess() {
}

double
Curve::interpolate(date dt) {
	return 0.0;
}

double Curve::getDiscountFactor(date dt)
{
	return 0.0;
}

void
Curve::processCash() {
}

void
Curve::processFutures() {
}


void
Curve::processSwaps() {
}
