#ifndef __OSL_JULIAN_DAY_H
/**
  Return the Chronological Julian Day (CJD) for this
  Gregorian calendar Year/Month/Day.
  
  For example, cjd_from_ymd(2000,01,01) should be 2451545
    
Warning:
   Gregorian dates were adopted:
     by the first countries in Europe (4 October 1582 CE).
     by England and the American Colonies (14 September 1752 CE).
*/
int cjd_from_ymd(int Year,int Month,int Day);
#endif
