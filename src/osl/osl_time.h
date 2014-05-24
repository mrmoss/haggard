/**
  Standalone, easy-linking timer routines.
  
  Orion Sky Lawlor, olawlor@acm.org, 2005/04/11 (Public Domain)
*/
#ifndef __OSL_TIME_H
#define __OSL_TIME_H

#ifndef _WIN32 /* UNIX system-- use gettimeofday */
#  include <sys/time.h>

/* Return the current wall clock time, in seconds */
inline double oslTime(void) {
	struct timeval tv;
	gettimeofday(&tv,0);
	return (tv.tv_sec * 1.0) + (tv.tv_usec * 0.000001);
}
#else /* windows-- use "clock" */
#  include <time.h>

/* Return the current wall clock time, in seconds */
inline double oslTime(void) {
	return clock()*(1.0/CLOCKS_PER_SEC);
}
#endif

#endif
