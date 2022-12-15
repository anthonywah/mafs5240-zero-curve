#include "CurveData.h"
#include <string>
#include <fstream>
#include <cstring>
#include "../Calendar/date.h"
#include <vector>

using std::string;
using std::ifstream;
using std::ofstream;
using std::qsort;

vector<string> monthInYear
{
	"BAD",	/* BAD MONTH = 0 */
	"Jan",	"Feb", "Mar",
	"Apr",	"May", "Jun",
	"Jul", "Aug", "Sep",
	"Oct",	"Nov", "Dec"
};

date
parse_date(string date_string) {
	int pos = date_string.find("-");
	string token1, token2, token3;
	token1 = date_string.substr(0, pos);
	date_string.erase(0, pos + 1);
	pos = date_string.find("-");
	token2 = date_string.substr(0, pos);
	date_string.erase(0, pos + 1);
	token3 = date_string.substr(0, date_string.length());

	int day, month = 0, year;
	day = stoi(token1);
	year = stoi(token3) + 2000;
	for (int i = 0; i < 13; i++) {
		if (monthInYear[i] == token2) {
			month = i;
			break;
		}
	}
	return date(year, month, day);
}


// Read curve data from file and populate "CurveData"
bool
CurveData::load(const char* filename) {
	std::ifstream dataFile(filename);
	string line;
	size_t pos;
	string token1, token2, token3;


	cash = new CashInput;
	futures = new FuturesInput;
	swaps = new SwapsInput;

	while (std::getline(dataFile, line)) {
		pos = line.find(",");
		token1 = line.substr(0, pos);
		if (token1 == "Currency") {
			token2 = line.substr(pos + 1, line.length());
			currency = _strdup(token2.c_str());
		}
		else if (token1 == "Base Date") {
			token2 = line.substr(pos + 1, line.length());
			baseDate= parse_date(token2);
		}
		else if (token1 == "Days to Spot") {
			token2 = line.substr(pos + 1, line.length());
			daysToSpot = stol(token2);
		}
		else if (token1 == "Holiday") {
			token2 = line.substr(pos + 1, line.length());
			holidayCenter = _strdup(token2.c_str());
		}
		else if (token1 == "Cash Basis") {
			token2 = line.substr(pos + 1, line.length());
			cash->m_cashBasis = token2;
		}
		else if (token1 == "Futures Basis") {
			token2 = line.substr(pos + 1, line.length());
			futures->m_futuresBasis = token2;
		}
		else if (token1 == "Swaps Basis") {
			token2 = line.substr(pos + 1, line.length());
			swaps->m_swapsBasis = token2;
		}
		else if (token1 == "Swaps Freq") {
			token2 = line.substr(pos + 1, line.length());
			swaps->m_swapsFreq = token2;
		}
		else if (token1 == "Cash") {
			line.erase(0, pos + 1);
			pos = line.find(",");
			token2 = line.substr(0, pos);
			line.erase(0, pos + 1);
			token3 = line.substr(0, line.length());
			CashPoint cp = make_pair(token2, stod(token3));
			cash->m_cashPoints.push_back(cp);
		}
		else if (token1 == "Futures") {
			line.erase(0, pos + 1);
			pos = line.find(",");
			token2 = line.substr(0, pos);
			line.erase(0, pos + 1);
			token3 = line.substr(0, line.length());
			FuturesPoint fp = make_pair(parse_date(token2), stod(token3));
			futures->m_futuresPoints.push_back(fp);
		}
		else if (token1 == "Swaps") {
			line.erase(0, pos + 1);
			pos = line.find(",");
			token2 = line.substr(0, pos);
			line.erase(0, pos + 1);
			token3 = line.substr(0, line.length());
			SwapsPoint sp = make_pair(token2, stod(token3));
			swaps->m_swapsPoints.push_back(sp);
		}
	}
	dataFile.close();
	return true;
}

