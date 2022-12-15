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

void
Curve::insertKeyPoint(date dt, DiscountFactorType df) {
	m_keyPoints[dt] = df;
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
	SwapsCashFlows data;
	date spotDt = date(m_baseDate);
	m_pMMCal->addBusDays(spotDt, m_daysToSpot);
	data.push_back(SwapsCashFlow(spotDt, -100, getDiscountFactor(spotDt)));
	int multiplier;
	if (m_pSwapsInput->m_swapsFreq == "Monthly") { multiplier = 12; }
	else if (m_pSwapsInput->m_swapsFreq == "Quarterly") { multiplier = 4; }
	else if (m_pSwapsInput->m_swapsFreq == "Semi-Annually") { multiplier = 2; }
	else if (m_pSwapsInput->m_swapsFreq == "Annually") { multiplier = 1; }
	for (int i = 0; i < m_pSwapsInput->m_swapsPoints.size(); i++) {
		int legs = multiplier * stoi(m_pSwapsInput->m_swapsPoints[i].first.substr(0, 2));
		date preDt = spotDt;
		for (int j = 0; j < legs; j++) {
			m_pMMCal->addMonths(spotDt, 12 / multiplier);
			if (m_pSwapsInput->m_swapsBasis == "ACT/365") {
				data.push_back(SwapsCashFlow(spotDt, 100 * m_pSwapsInput->m_swapsPoints[i].second * (spotDt - preDt / 365), getDiscountFactor(spotDt)));}
			else if (m_pSwapsInput->m_swapsBasis == "ACT/360") {
				data.push_back(SwapsCashFlow(spotDt, 100 * m_pSwapsInput->m_swapsPoints[i].second * (spotDt - preDt / 360), getDiscountFactor(spotDt)));}
			else if (m_pSwapsInput->m_swapsBasis == "ACT/ACT") {
				data.push_back(SwapsCashFlow(spotDt, 100 * m_pSwapsInput->m_swapsPoints[i].second * (spotDt - preDt / date::daysInYear(spotDt.year())), getDiscountFactor(spotDt)));
			}
			preDt = spotDt;
		}
		date lastDt = m_keyPoints.rbegin()->first;
		double lastdf = m_keyPoints.rbegin()->second;
		double uBound = 1.0, lBound = 0.0, npv = 0.0, df;
		int iter = 0;
		do {
			npv = 0.0;
			df = (uBound + lBound) / 2;
			for (int k = 0; k < data.size(); ++k) {
				npv += data[k].CF * (data[k].DF == 0.0 ? lastdf * pow(df / lastdf, (data[k].dt - lastDt) / (data.back().dt - lastDt)) : data[k].DF);
			}
			(npv > 0.0) ? uBound = df : lBound = df;
			iter++;
		} while (abs(npv) > 0.0001 && iter < 1000);
		for (int k = 0; k < data.size(); ++k) {
			if (data[k].DF == 0.0) { insertKeyPoint(data[k].dt, lastdf * pow(df / lastdf, (data[k].dt - lastDt) / (data.back().dt - lastDt))); }
		}
		data.clear();
	}
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
