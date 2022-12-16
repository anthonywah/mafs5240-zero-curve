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

	// Year-basis for day count convention
	cashYB = getDayCountBase(m_pCashInput->m_cashBasis, m_baseDate);
	futuresYB = getDayCountBase(m_pFuturesInput->m_futuresBasis, dtSpot);
	swapsYB = getDayCountBase(m_pSwapsInput->m_swapsBasis, dtSpot);

	// Number of swap payment per year
	SwapsFreqType freq = m_pSwapsInput->m_swapsFreq;
	nPerYear = freq == "Monthly" ? 12 : freq == "Quarterly" ? 4 : freq == "Semi-Annually" ? 2 : freq == "Annually" ? 1 : 0;

	// DF of base date is always 1
	m_keyPoints[m_baseDate] = 1.0;
}

double
Curve::interpolate(date dt) {
	date max_dt = m_keyPoints.begin()->first;
	double max_dt_df = m_keyPoints.begin()->second;
	date min_dt = m_keyPoints.begin()->first;
	double min_dt_df = m_keyPoints.begin()->second;
	date d1, d2;
	double d1Df, d2Df;
	for (KeyPoints::iterator it = m_keyPoints.begin(); it != m_keyPoints.end(); ++it) {
		if (it->first == dt) {  // No need to interpolate, just return the discount factor
			return it->second;	// Should have been handled by getDiscountFactor but just in case
		}
		if (it->first > max_dt) {
			max_dt = it->first;
			max_dt_df = it->second;
		}
		if (it->first < min_dt) {
			min_dt = it->first;
			min_dt_df = it->second;
		}
	}
	// dt larger than all, find the 2 largest pts to extrapolate
	if (dt > max_dt) {
		d1 = max_dt;
		d1Df = max_dt_df;
		d2 = date(0);
		for (KeyPoints::iterator it = m_keyPoints.begin(); it != m_keyPoints.end(); ++it) {
			if ((it->first != max_dt) and (it->first > d2)) {
				d2 = it->first;
				d2Df = it->second;
			}

		}
		return d1Df * pow(d2Df / d1Df, ((double)dt - (double)d1) / ((double)d2 - (double)d1));
	}
	// dt smaller than all, find the 2 smallest pts to extrapolate
	if (dt < min_dt) {
		d1 = min_dt;
		d1Df = min_dt_df;
		d2 = date(99999);
		for (KeyPoints::iterator it = m_keyPoints.begin(); it != m_keyPoints.end(); ++it) {
			if ((it->first != max_dt) and (it->first < d2)) {
				d2 = it->first;
				d2Df = it->second;
			}

		}
		return d1Df * pow(d2Df / d1Df, ((double)dt - (double)d1) / ((double)d2 - (double)d1));
	}
	// dt inside the range
	d1 = min_dt;
	d1Df = min_dt_df;
	d2 = max_dt;
	d2Df = max_dt_df;
	for (KeyPoints::iterator it = m_keyPoints.begin(); it != m_keyPoints.end(); ++it) {
		if ((it->first > d1) and (it->first < dt)) {
			d1 = it->first;
			d1Df = it->second;
		}
		if ((it->first < d2) and (it->first > dt)) {
			d2 = it->first;
			d2Df = it->second;

		}
	}

	return d1Df * pow(d2Df / d1Df, ((double)dt - (double)d1) / ((double)d2 - (double)d1));

}

double Curve::getDiscountFactor(date dt)
{
	double res = 0.0;
	KeyPoints::iterator it = Curve::m_keyPoints.find(dt);
	if (it != m_keyPoints.end()) {
		res = it->second;
		return res;
	}
	else {
		res = Curve::interpolate(dt);
	}
	return res;
}

void
Curve::processCash() {
	CashPoints cps = m_pCashInput->m_cashPoints;
	date keypoint_date;
	double keypoint_df;
	// Insert all points before spot first
	for (CashPoints::iterator it = cps.begin(); it != cps.end(); ++it) {
		string cash_type = it->first;
		double forward_rate = it->second;
		if (cash_type == "ON") {
			keypoint_date = m_baseDate + 1;
			keypoint_df = getDiscountFactor(m_baseDate) / (1 + forward_rate * 1 / cashYB);
			insertKeyPoint(keypoint_date, keypoint_df);
		}else if (cash_type == "TN") {
			keypoint_date = m_baseDate + 2;
			keypoint_df = getDiscountFactor(m_baseDate + 1) / (1 + forward_rate * 1 / cashYB);
			insertKeyPoint(keypoint_date, keypoint_df);
		}
	}
	// extrapolate for discount factor of spot
	keypoint_date = dtSpot;
	double dfSpot = getDiscountFactor(dtSpot);
	insertKeyPoint(keypoint_date, dfSpot);

	// Insert all points after spot
	for (CashPoints::iterator it = cps.begin(); it != cps.end(); ++it) {
		string cash_type = it->first;
		double forward_rate = it->second;
		if (cash_type == "SN") {
			keypoint_date = dtSpot + 1;
			keypoint_df = dfSpot / (1 + forward_rate * 1 / cashYB);
			insertKeyPoint(keypoint_date, keypoint_df);
		}
		else if (cash_type == "SW") {
			keypoint_date = dtSpot + 7;
			keypoint_df = dfSpot / (1 + forward_rate * 7 / cashYB);
			insertKeyPoint(keypoint_date, keypoint_df);
		}
		else if (cash_type[1] == 'M') {
			int num_month = cash_type[0] - '0';
			keypoint_date = dtSpot;
			m_pMMCal->addMonths(keypoint_date, num_month);
			keypoint_df = getDiscountFactor(dtSpot) / (1 + forward_rate * (keypoint_date - dtSpot) / cashYB);
			insertKeyPoint(keypoint_date, keypoint_df);
		}
		else if (cash_type[1] == 'Y') {
			int num_year = cash_type[0] - '0';
			keypoint_date = dtSpot;
			m_pMMCal->addYears(keypoint_date, num_year);
			keypoint_df = getDiscountFactor(dtSpot) / (1 + forward_rate * (keypoint_date - dtSpot) / cashYB);
			insertKeyPoint(keypoint_date, keypoint_df);
		}
	}
}

void
Curve::processFutures() {

	double df3M = getDiscountFactor(dt3M);

	// First get fp1
	FuturesPoints fps = m_pFuturesInput->m_futuresPoints;
	FuturesPoint fp1 = fps[0];
	for (FuturesPoints::iterator it = fps.begin(); it != fps.end(); ++it) {
		if (it->first < fp1.first) {
			fp1 = *it;
		}
	}

	// Then get fp2
	FuturesPoint fp2 = fp1;
	for (FuturesPoints::iterator it = fps.begin(); it != fps.end(); ++it) {
		if (it->first != fp1.first) {
			if (fp2.first == fp1.first || (fp2.first != fp1.first && it->first < fp2.first)) {
				fp2 = *it;
			}
		}
	}

	// Then calculate df1 first
	DiscountFactorType df1 = df3M / pow(getFpsDf1ToDf2Factor(fp1.second, fp1.first, fp2.first, futuresYB), (((double)dt3M - (double)fp1.first) / ((double)fp2.first - (double)fp1.first)));
	insertKeyPoint(fp1.first, df1);

	// Then loop through FuturesPoints to insert key points -> Calculate df2 subsequently
	// Not sure if FuturesPoints is sorted or not, so we adopt the following loops
	bool calculating = true;
	DiscountFactorType df2;
	while (calculating) {
		calculating = false;  // terminating condition
		for (FuturesPoints::iterator it = fps.begin(); it != fps.end(); ++it) {
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
			df2 = df1 * getFpsDf1ToDf2Factor(fp1.second, fp1.first, fp2.first, futuresYB);
			insertKeyPoint(fp2.first, df2);

			// Condition for next calc ulation
			df1 = df2;
			fp1 = fp2;
		}
	}
	// extrapolate future point
	date last_date = fps[fps.size() - 1].first;
	double last_df = getDiscountFactor(last_date);
	double last_price = fps[fps.size() - 1].second;
	date next_imm_day = last_date;
	next_imm_day = m_pMMCal->nextIMMDay(next_imm_day);
	double df = last_df / (1.0 + (100.0 - last_price) / 100.0 * ((double)next_imm_day - (double)last_date) / (double)futuresYB);
	insertKeyPoint(next_imm_day, df);
}

void
Curve::processSwaps() {
	SwapsCashFlows data;
	// Calculate key points 
	for (int i = 0; i < m_pSwapsInput->m_swapsPoints.size(); i++) {
		data.push_back(SwapsCashFlow(dtSpot, -100, getDiscountFactor(dtSpot)));
		int num_year = stoi(m_pSwapsInput->m_swapsPoints[i].first.substr(0, 2));
		int legs = nPerYear * num_year;
		date cashflow_Dt = dtSpot;
		date prev_cashflow_Dt = dtSpot;
		for (int j = 0; j < legs; j++) {
			cashflow_Dt = dtSpot;
			m_pMMCal->addMonths(cashflow_Dt, 12 / nPerYear * (j + 1));
			double cash_flow_amt = 100.0 * m_pSwapsInput->m_swapsPoints[i].second  * ((double)cashflow_Dt - (double)prev_cashflow_Dt) / swapsYB;
			if (j == legs - 1) {
				cash_flow_amt += 100.0;
			}
			// DF = 0.0 is a placeholder, it will be recalculated in the bisection method
			data.push_back(SwapsCashFlow(cashflow_Dt, cash_flow_amt, 0.0)); 
			prev_cashflow_Dt = cashflow_Dt;
		}
		// assume keypoints are sorted 
		// find last available keypoint, and corresponding df
		date lastDt = m_keyPoints.rbegin()->first;
		double lastdf = m_keyPoints.rbegin()->second;
		double uBound = 1.0, lBound = 0.0, npv = 0.0, k, df;
		int iter = 0;
		do {
			npv = 0.0;
			k = (uBound + lBound) / 2.0;
			for (int j = 0; j < data.size(); j++) {
				if (data[j].dt > lastDt){
					df = lastdf * pow(k, ((double)data[j].dt - (double)lastDt) / swapsYB);
				}
				else {
					df = getDiscountFactor(data[j].dt);
				}
				data[j].DF = df * data[j].CF;
				npv += data[j].DF;
			}
			(npv > 0.0) ? uBound = k : lBound = k;
			iter++;
		} while (abs(npv) > 0.00000001 && iter < 100000);

		for (int j = 0; j < data.size(); j++) {
			insertKeyPoint(data[j].dt, data[j].DF / data[j].CF);
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
	return 1 / (1 + ((1 - (p1 / 100)) * (((double)d2 - (double)d1) / (double)dayCountBase)));
}
