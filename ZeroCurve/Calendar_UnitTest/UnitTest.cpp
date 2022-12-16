#include "pch.h"
#include "CppUnitTest.h"
#include "..\Calendar\Calendar.h"
#include "..\Calendar\date.h"
#include "..\ZeroCurve\CurveData.h"
#include "..\ZeroCurve\CurveInput.h"
#include "..\ZeroCurve\Curve.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest
{
	MMCalendar* pJpy;
	CurveData cd;
	Curve* pCurve;

	TEST_CLASS(CalendarUnitTest)
	{
	public:

		TEST_CLASS_INITIALIZE(LoadHolidays)
		{
			// test class initialization code
			//pJpy = new MMCalendar("C:\\Users\\liuch\\Desktop\\MAFS5240\\mafs5240-zero-curve\\ZeroCurve\\JPY_Holiday.txt", "JPY");
			pJpy = new MMCalendar("C:\\Users\\wahch\\Desktop\\MAFM\\MAFS5240\\GroupProject\\mafs5240-zero-curve\\ZeroCurve\\JPY_Holiday.txt", "JPY");
		}
		TEST_METHOD(Holiday20131104)
		{
			date d20131104(2013, 11, 4);
			Assert::IsFalse(pJpy->isBusDay(d20131104));
		}

		TEST_METHOD(MonthEnd) 
		{
			date d20130131(2013, 1, 31);
			date d(d20130131);
			pJpy->addMonths(d, 1);
			Assert::IsTrue(d == date(2013, 2, 28));
		}

		TEST_METHOD(ModifiedFollowing)
		{
			date d(2013, 2, 28);
			pJpy->addMonths(d, 1);
			Assert::IsTrue(d == date(2013, 3, 29));
		}
	};

	TEST_CLASS(ZeroCurveUnitTest) {
	public:
		TEST_CLASS_INITIALIZE(ConstructCurve)
		{
			// test class initialization code
			string mkt = "JPY";
			//string holidayPath = "C:\\Users\\liuch\\Desktop\\MAFS5240\\mafs5240-zero-curve\\ZeroCurve\\JPY_Holiday.txt";
			//const char* curveDataPath = "C:\\Users\\liuch\\Desktop\\MAFS5240\\mafs5240-zero-curve\\ZeroCurve\\\curveData.txt";
			string holidayPath = "C:\\Users\\wahch\\Desktop\\MAFM\\MAFS5240\\GroupProject\\mafs5240-zero-curve\\ZeroCurve\\JPY_Holiday.txt";
			const char* curveDataPath = "C:\\Users\\wahch\\Desktop\\MAFM\\MAFS5240\\GroupProject\\mafs5240-zero-curve\\ZeroCurve\\curveData.txt";
			pJpy = new MMCalendar(holidayPath, mkt);
			cd.load(curveDataPath);
			pCurve = new Curve(cd.baseDate, cd.daysToSpot, pJpy, cd.cash, cd.futures, cd.swaps);
		}
		TEST_METHOD(Cash3M)
		{
			date d(2007, 6, 12);
			DiscountFactorType df = pCurve->getDiscountFactor(d);
			Assert::AreEqual(0.998135175, df, 0.00000001);
		}

		TEST_METHOD(Futures20070322)
		{
			date d(2007, 3, 22);
			DiscountFactorType df = pCurve->getDiscountFactor(d);
			Assert::AreEqual(0.999710609, df, 0.00000001);
		}

		TEST_METHOD(Futures20081217)
		{
			date d(2008, 12, 17);
			DiscountFactorType df = pCurve->getDiscountFactor(d);
			Assert::AreEqual(0.984416955, df, 0.00000001);
		}
		TEST_METHOD(FirstSwap)
		{
			date d(2010, 3, 12);
			DiscountFactorType df = pCurve->getDiscountFactor(d);
			Assert::AreEqual(0.968739775, df, 0.00000001);
		}

		TEST_METHOD(Swap20170313)
		{
			date d(2017, 3, 13);
			DiscountFactorType df = pCurve->getDiscountFactor(d);
			Assert::AreEqual(0.836922849, df, 0.00000001);
		}

		TEST_METHOD(Swap20300312)
		{
			date d(2030, 3, 12);
			DiscountFactorType df = pCurve->getDiscountFactor(d);
			Assert::AreEqual(0.576921389, df, 0.00000001);
		}
	};
}
