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

SwapsCashFlow::SwapsCashFlow(date dtInput, double CFInput, double DFInput)
	: dt(dtInput), CF(CFInput), DF(DFInput) {}


KeyPoint
Curve::retrieveKeyPoint(KeyPoints::const_iterator ki) {
	if (ki == m_keyPoints.end())
		return make_pair(0L, 0.0);
	else
		return *ki;
}

void
Curve::initProcess() {
	dtSpot = m_baseDate;
	m_pMMCal->addBusDays(dtSpot, m_daysToSpot);
	dt3M = dtSpot;
	m_pMMCal->addMonths(dt3M, 3);
	
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

	DiscountFactorType df3M = getDiscountFactor(dt3M);
	int futDayCountBase = getDayCountBase(m_pFuturesInput->m_futuresBasis, dtSpot);

	// First get fp1
	FuturesPoints fps = m_pFuturesInput->m_futuresPoints;
	FuturesPoint fp1 = fps[0];
	for (FuturesPoints::iterator it = fps.begin(); it < fps.end(); it++) {
		if (it->first < fp1.first) {
			fp1 = *it;
		}
	}

	// Then get fp2
	FuturesPoint fp2 = fp1;
	for (FuturesPoints::iterator it = fps.begin(); it < fps.end(); it++) {
		if (it->first != fp1.first) {
			if (fp2.first == fp1.first || (fp2.first != fp1.first && it->first < fp2.first)) {
				fp2 = *it;
			}
		}
	}

	// Then calculate df1 first
	DiscountFactorType df1 = df3M / pow(getFpsDf1ToDf2Factor(fp1.second, fp1.first, fp2.first, futDayCountBase), ((dt3M - fp1.first) / (fp2.first - fp1.first)));
	insertKeyPoint(fp1.first, df1);

	// Then loop through FuturesPoints to insert key points -> Calculate df2 subsequently
	// Not sure if FuturesPoints is sorted or not, so we adopt the following loops
	bool calculating = true;
	DiscountFactorType df2;
	while (calculating) {
		calculating = false;  // terminating condition
		for (FuturesPoints::iterator it = fps.begin(); it < fps.end(); it++) {
			if (m_keyPoints.find(it->first) == m_keyPoints.end()) {  // The point has not been inserted yet
				calculating = true;
				if (it->first != fp1.first) {  // The point is not current point
					if (fp2.first == fp1.first || (fp2.first != fp1.first && it->first < fp2.first)) {  
						fp2 = *it;
					}
				}
			}
		}
		if (calculating) {
			df2 = df1 * getFpsDf1ToDf2Factor(fp1.second, fp1.first, fp2.first, futDayCountBase);
			insertKeyPoint(fp2.first, df2);

			// Condition for next calculation
			df1 = df2;
			fp1 = fp2;
		}
	}
}

void
Curve::processSwaps() {
	SwapsCashFlows data;
	date spotDt = date(m_baseDate);
	date preDt = spotDt;
	m_pMMCal->addBusDays(spotDt, m_daysToSpot);
	data.push_back(SwapsCashFlow(spotDt, -100, getDiscountFactor(spotDt)));

	// Fix multipler first
	SwapsFreqType freq = m_pSwapsInput->m_swapsFreq;
	int multiplier = freq == "Monthly" ? 12 : freq == "Quarterly" ? 4 : freq == "Semi-Annually" ? 2 : freq == "Annually" ? 1 : 0;

	// Calculate base for day count base
	int dayCountbase = getDayCountBase(m_pSwapsInput->m_swapsBasis, spotDt);
	
	// Calculate key points 
	for (int i = 0; i < m_pSwapsInput->m_swapsPoints.size(); i++) {
		int legs = multiplier * stoi(m_pSwapsInput->m_swapsPoints[i].first.substr(0, 2));
		for (int j = 0; j < legs; j++) {
			m_pMMCal->addMonths(spotDt, 12 / multiplier);
			data.push_back(SwapsCashFlow(spotDt, 100 * m_pSwapsInput->m_swapsPoints[i].second * (spotDt - preDt / dayCountbase), getDiscountFactor(spotDt)));
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
			if (data[k].DF == 0.0) { 
				insertKeyPoint(data[k].dt, lastdf * pow(df / lastdf, (data[k].dt - lastDt) / (data.back().dt - lastDt))); 
			}
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

int getDayCountBase(string basisType, date dt) {
	return basisType == "ACT/360" ? 360 : basisType == "ACT/365" ? 365 : basisType == "ACT/ACT" ? dt.daysInYear() : 0;
}


double getFpsDf1ToDf2Factor(FuturesPriceType p1, date d1, date d2, int dayCountBase) {
	return 1 / (1 + ((1 - (p1 / 100)) * ((d2 - d1) / dayCountBase)));
}
