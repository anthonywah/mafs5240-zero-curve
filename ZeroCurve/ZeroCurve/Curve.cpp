#include "Curve.h"
#include <cstring>
#include <cmath>

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
	double res = 0.0;
	date d1 = date(0), d2 = date(0);
	int d1Diff = -999999, d2Diff = 999999;
	DiscountFactorType d1Df = 0.0, d2Df = 0.0;
	for (KeyPoints::iterator it = m_keyPoints.begin(); it != m_keyPoints.end(); it++) {
		if (it->first == dt) {  // No need to interpolate, just return the discount factor
			res = it->second;	// Should have been handled by getDiscountFactor but just in case
			break;
		}
		int dayDiff = it->first - dt;
		if (dayDiff < d2Diff) {  // look for closest day ahead of dt
			d2Diff = dayDiff;
			d2 = it->first;
			d2Df = it->second;
		}
		if (dayDiff > d1Diff) {  // look for closet day behind dt
			d1Diff = dayDiff;
			d1 = it->first;
			d1Df = it->second;
		}
	}
	if (res == 0.0) {  // Calculate exponential interpolation
		res = d1Df * pow(d2Df / d1Df, (dt - d1) / (d2 - d1));
	}
	return res;
}

double Curve::getDiscountFactor(date dt)
{
	double res = 0.0;
	KeyPoints::iterator it = Curve::m_keyPoints.find(dt);
	if (it != m_keyPoints.end()) {
		res = it->second;
		return true;
	}
	else {
		res = Curve::interpolate(dt);
	}
	return res;
}

void
Curve::processCash() {
}

void
Curve::processFutures() {
	// First get df1fut from simultaneously solving equations
	// Then calculate remaining futures price
}

void
Curve::processSwaps() {
}

bool
Curve::insertKeyPoint(date dt, DiscountFactorType df) {
	if (Curve::m_keyPoints.find(dt) == m_keyPoints.end()) {
		Curve::m_keyPoints[dt] = df;
		return true;
	}

	// Return false if key existed already
	return false;  
}