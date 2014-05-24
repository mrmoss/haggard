/**
A "timestamped" object has a last_updated time, which is used to 
determine if the object is out of date.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-06 (Public Domain)
*/
#ifndef __CYBERALASKA_TIMESTAMP_H
#define __CYBERALASKA_TIMESTAMP_H

#include "../cyberalaska/time.h"

namespace cyberalaska {

	class timestamped {
	public:
		/** The cyberalaska::time(), in seconds, when we were last updated. 
		   This is written automatically when you call update_timestamp().
		   It's read by the is_current() method below.
		*/
		double last_update;
		
		/** An estimate of the lag, in seconds, between updates to our value. */
		double lag;
		
		/** Mark this timestamp with the current time (or cur_time if set). */
		virtual void update_timestamp(double cur_time=0) {
			if (cur_time==0) cur_time=cyberalaska::time();
			lag=cur_time-last_update;
			last_update=cur_time;
		}

		/** Return true if our value has been updated in less than max_lag seconds. */
		bool is_current(double max_age=1.0) const {
			return cyberalaska::time()-last_update<max_age;
		}
		
		/** Return true if we have been updated after this timestamp. */
		bool has_newer(double than_time) const {
			return last_update>than_time;
		}
		
		timestamped() 
			:last_update(0.0), lag(100.0) {}
		virtual ~timestamped() {}

		template <class PUP> 
		void pup(PUP &p) {
			pup(p,last_update,"last_update");
			pup(p,lag,"lag");
		}
		
	};

};


#endif

