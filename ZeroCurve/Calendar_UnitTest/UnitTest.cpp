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
			pJpy = new MMCalendar("C:\\Users\\liuch\\Desktop\\MAFS5240\\mafs5240-zero-curve\\ZeroCurve\\JPY_Holiday.txt", "JPY");
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

		TEST_METHOD(addMonth)
		{
			date d(2022, 12, 21);
			pJpy->addMonths(d, 1);
			Assert::IsTrue(d == date(2023, 1, 23));
			date d2(2022, 12, 1);
			pJpy->addMonths(d2, 1);
			Assert::IsTrue(d2 == date(2023, 1, 4));
		}

		TEST_METHOD(ROLL)
		{
			date d(2022, 12, 16);
			pJpy->roll(d);
			Assert::IsTrue(d == date(2022, 12, 19));
			date d2(2022, 12, 30);
			pJpy->roll(d2);
			Assert::IsTrue(d2 == date(2022, 12, 29));
		}

		TEST_METHOD(IMMDay1)
		{
			date d(2022, 12, 21);
			Assert::IsTrue(pJpy->isIMMDay(d));
			date d2 = pJpy->nextIMMDay(d);
			Assert::IsTrue(d2 == date(2023, 3, 15));
			date d3(2022, 1, 3);
			date d4 = pJpy->nextIMMDay(d3);
			Assert::IsTrue(d4 == date(2022, 3, 16));
		}



	};

	TEST_CLASS(ZeroCurveUnitTest) {
	public:
		TEST_CLASS_INITIALIZE(ConstructCurve)
		{
			// test class initialization code
			string mkt = "JPY";
			//string holidayPath = "C:\\Users\\liuch\\Desktop\\MAFS5240\\mafs5240-zero-curve\\ZeroCurve\\JPY_Holiday.txt";
			//string curveDataPath = "C:\\Users\\liuch\\Desktop\\MAFS5240\\mafs5240-zero-curve\\ZeroCurve\\\curveData.txt";
			string holidayPath = "C:\\Users\\wahch\\Desktop\\MAFM\\GroupProject\\mafs5240-zero-curve\\ZeroCurve\\JPY_Holiday.txt";
			const char* curveDataPath = "C:\\Users\\wahch\\Desktop\\MAFM\\GroupProject\\mafs5240-zero-curve\\ZeroCurve\\curveData.txt";
			
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
