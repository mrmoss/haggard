/**
  Convert a Gregorian calendar Year/Month/Day into
  a Chronological Julian Day (CJD).
 
  From http://www.lanl.gov/Caesar/node491.html#Julian_Day_Code
      (public domain, as US government code)
  
  Orion Sky Lawlor, olawlor@acm.org, 2007/02/25 (Public Domain)
*/

/**
  Return the Chronological Julian Day (CJD) for this
  Gregorian calendar Year/Month/Day.
  
  For example, cjd_from_ymd(2000,01,01) should be 2451545
    
Warning:
   Gregorian dates were adopted:
     by the first countries in Europe (4 October 1582 CE).
     by England and the American Colonies (14 September 1752 CE).
*/
int cjd_from_ymd(int Year,int Month,int Day) {
	int Julian_Year, Julian_Month, Julian_Day;
	int Shifted_Julian_Year, Julian_Day_Constant;
	
	/*
	! Convert to Julian (CE) Year, which includes a zero year:
	!
	!                           BC  AD
	!     Year:     -4  -3  -2  -1  1  2  3  4
	! Julian Year:  -3  -2  -1   0  1  2  3  4
	*/
	if (Year > 0) {
		Julian_Year = Year;
	} else {
		Julian_Year = Year + 1;
	}
	
	/*
	! Convert to Julian Month for ease in calculating month days and
	! dealing with February.
	!
	!     Month:      1   2  3  4  5  6  7  8   9  10  11  12
	! Julian Month:  14  15  4  5  6  7  8  9  10  11  12  13
	!
	! For consistency, modify Julian_Year to start with March.
	*/
	if (Month < 3) {
		Julian_Month = Month + 13;
		Julian_Year  = Julian_Year - 1;
	} else {
		Julian_Month = Month + 1;
	}

	/*
	! Calculate Julian_Year shifted by 8000 years to avoid problems 
	! with leap years in negative years.
	*/
	Shifted_Julian_Year = Julian_Year + 8000;

	/*
	! Adjustment constant -- this is the number of days from the start
	! of the Julian Day numbering system, 1 January 4713 BC, to the new 
	! zero date which is used for calculations. Due to the month shifting,
	! the new zero date is 30 October 2 BC.
	*/
	Julian_Day_Constant = - ( 
		  + 365*(-4713)                 /* 365 days/year. */
		  + (int)(30.6001*14)             /* Trick to get days/month. */
		  + (int)((-4713+8000)/4) - 2000  /* Leap days. */
		  + 1                           /* Day of the month. */
		  );
	
	/* Gregorian calendar: */
	Julian_Day = Julian_Day_Constant          /* Adjustment constant. */
		  + 365*Julian_Year                     /*  365 days/year. */
		  + (int)(30.6001*Julian_Month)           /*  Trick to get days/month. */
		  + (int)(Shifted_Julian_Year/4)   - 2000 /* Leap days. */
		  - (int)(Shifted_Julian_Year/100) + 80   /*  Gregorian leap  */
		  + (int)(Shifted_Julian_Year/400) - 20   /*    day adjustment. */
		  + 2      /*  Difference between Gregorian and Julian calendars at 0 CE. */
		  + Day;                                    /*  Day of the month. */
	
	return Julian_Day;
}


#ifdef STANDALONE  /* testing stub */
#include <stdio.h>
int main()
{
	const static struct test {
		int year,month,day;
		int cjd;
	} tests[] = {
		{ 1999,12,31,  2451544},
		{ 2000,01,01,  2451545},
		{ 2000,12,31,  2451545+365}
	};
	int t;
	for (t=0;t<sizeof(tests)/sizeof(tests[0]);t++) {
		int cur=cjd_from_ymd(tests[t].year, tests[t].month, tests[t].day);
		int cor=tests[t].cjd;
		if (cur!=cor) printf("Error: (%d,%d,%d) -> current %d, correct %d\n",
			tests[t].year, tests[t].month, tests[t].day, cur,cor);
	}
	int dow=cjd_from_ymd(2000,01,01)%7;
	if (dow!=5) printf("Error in day-of-week for Jan 1 2000: should be sat, 5, but is %d\n",dow);
	return 0;
}

#endif
