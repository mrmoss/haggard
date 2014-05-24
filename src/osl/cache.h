/*
Orion Sky Lawlor, olawlor@acm.org, 2003/6/5
*/
#ifndef __OSL_CACHE_H
#define __OSL_CACHE_H

#include <vector>

/**
A fully associative cache, templated on Key and Data.
The Key template must be assignable and comparable using ==.
The Data template need only be assignable.
*/
template <class Key,class Data> class cache {
	class cacheLine {
	public:
		Key key;
		Data data;
		bool valid;
		cacheLine() {valid=false;}
	};
	std::vector<cacheLine> l;
	int len;
	int last;//Last hit in the cache
	int over;//Will overwrite this entry next
public:
	cache(int len_=4) :l(len_), len(len_) {last=0;over=len/2;}
	
	/// Put this new key in the table.
	///  Can write the data using h.get()=myData;
	void put(const Key &k) {
		//First look for an empty spot
		for (int i=0;i<len;i++) {
			int e=last+i; if (e>=len) e-=len;
			if (!l[e].valid) {//Just use this empty spot
				last=e;
				l[last].key=k;
				l[last].valid=true;
				return;
			}
		}
		//Otherwise, overwrite an old value
		over++; if (over>=len) over-=len;
		last=over;
		l[last].key=k;
	}
	
	/// Return true if this key is in the cache
	bool lookup(const Key &k) {
		for (int i=0;i<len;i++) {
			int e=last+i; if (e>=len) e-=len;
			if (l[e].valid && k==l[e].key) {
				last=e;
				return true;
			}
		}
		return false;
	}
	/// Indicate that the key for the last lookup is now bad.
	void invalidate(void) {l[last].valid=false;}
	
	/// Get the data from the last put or lookup call
	Data &get(void) {return l[last].data;}
	const Data &get(void) const {return l[last].data;}
};

#endif
