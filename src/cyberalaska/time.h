/**
Wall clock time interface, adapted from osl/time_function.h

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-04 (Public Domain)
*/
#ifndef __CYBERALASKA__TIME_H
#define __CYBERALASKA__TIME_H



/**
  Return the current wall clock time, in seconds since Jan 1 1970.
*/
#if defined(_WIN32)
#  include <sys/timeb.h>

namespace cyberalaska {
inline double time(void) { /* This seems to give terrible resolution (60ms!) */
        struct _timeb t;
        _ftime(&t);
        return t.millitm*1.0e-3+t.time*1.0;
}
}; /* end namespace */

#else /* UNIX or other system */
#  include <sys/time.h> //For gettimeofday time implementation

namespace cyberalaska {
inline double time(void) {
	struct timeval tv;
	gettimeofday(&tv,0);
	return tv.tv_usec*1.0e-6+tv.tv_sec*1.0;
}
}; /* end namespace */

#endif




#endif /* end include guard */

