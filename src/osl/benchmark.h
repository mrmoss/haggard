/*
Tools for measuring timing and profiling statistics.

Orion Sky Lawlor, olawlor@acm.org, 2007/06/30 (Public Domain)
*/
#ifndef __OSL_BENCHMARK_H
#define __OSL_BENCHMARK_H

#include "osl/osl.h"

/**
Used to compute, store, and later plot the time 
taken for various operations.
*/
namespace benchmark {

	/// Return the wall time, in seconds.
	inline double time(void) {return osl::time();}

	/**
	  Describes a particular operation, such as a kind of drawing.
	*/
	class op_t {
	public:
		int idx;
	};
	
	/// Can only define up to this many operations (to avoid dynamic allocation)
	enum {op_null=0, op_max=1000};
	
	/// Number of operations currently defined (0..op_max)
	extern int op_len;
	
	/**
	  Sums up the time taken by each operation.
	*/
	class stats {
	public:
		/** Accumulators for each op_t: */
		double t[op_max];
		
		stats() {zero();}
		
		/// Clear all accumulators.
		void zero(void) {
			for (int op=0;op<op_len;op++) t[op]=0.0;
		}
		/// Add this value to this accumulator.
		inline void add(double val,op_t op) {t[op.idx]+=val;}

		/// Look up the value in this accumulator.
		inline double get(op_t op) const {return t[op.idx];}
		inline void set(double val,op_t op) {t[op.idx]=val;}
		
		/// Add everything in this object, scaled by scale, to us.
		inline void add(const stats &s,double scale=1.0) {
			for (int op=0;op<op_len;op++) t[op]+=s.t[op]*scale;
		}
		
		/// Print these stats, scaled by scale, where they exceed threshold.
		void print(FILE *f,const char *what,double scale=1.0,double thresh=0.001) const;
	};
	
	/**
	  Return the current stats object.
	  It is common to read from this object.
	  You should be careful when writing to this object, however!
	*/
	stats *get(void);
	
	/// Create a new count-type operation (don't use this, use the object-oriented interface below)
	op_t count_op(const char *shortName,const char *desc,const char *units);
	
	/**
	  Define a new timing operation.  Can be called at startup time
	  to initialize a static variable.  
	  When you call benchmark::swap, a timing operation 
	  replaces the previous timing operation, and updates its timer.
	*/
	op_t time_op(const char *shortName,const char *desc);
	
	
	/**
	  Start running this timer operation.
	  Implicitly stops running the previous timer operation.
	  Returns the old operation that was running.
	*/
	op_t swap(op_t time_op);

/************* Counting Operations ************/
	/**
	  Define a new counting operation.  Legal at static initialization time.
	  A counting operation keeps track of the number of times something happens.
	  You use it like this:
	  
		static benchmark::count_t op_tricount("tricount","Triangles rendered","tri");
		op_tricount++;
	*/
	class count_t : public op_t {
	public:
		count_t(const char *shortName,const char *desc,const char *units) 
			:op_t(count_op(shortName,desc,units)) {}
		/// Increment our counter.
		inline int operator++(void) { return (int)(get()->t[idx]++); }
		inline int operator++(int ignored) { return (int)(get()->t[idx]++); }
		inline int operator+=(int n) { return (int)(get()->t[idx]+=n); }
		/// Return the number of counts so far.
		inline operator int (void) const {return (int)(get()->t[idx]);}
		inline operator long (void) const {return (long)(get()->t[idx]);}
		inline operator double (void) const {return (double)(get()->t[idx]);}
	};

/************* Timer Operations ***************/
	/**
	  Define a new timing operation.  Legal at static initialization time.
	  A timing operation keeps track of how long a piece of code takes.
	  
	  You can use a timer either by explicitly swapping it on and off, or 
	  else use the timer_sentry class below.
	  
		static benchmark::timer_t op_tritime("tritime","Time rendering triangles");
		
		{
			benchmark::timer_sentry sentry(op_tritime);
			... // code to be timed goes here 
			// sentry's destructor stops the timer
		}
		
		// Or use the "swap" interface (not as good as the sentry)
		benchmark::op_t old_op=benchmark::swap(op_tritime);
		.. // code to be timed goes here 
		benchmark::swap(old_op);
	*/
	class timer_t : public op_t {
	public:
		timer_t(const char *shortName,const char *desc)
			:op_t(time_op(shortName,desc)) {}
		/// Return the number of seconds spent in this op so far.
		inline operator double (void) const {return (double)(get()->t[idx]);}
	};
	
	/**
	   Sentry class: starts and stops timer for one operation.
	   The sentry's destructor stops the timer, and restarts the
	   previous operation.
	   
	   So the sentry works even for nested operations:
		start a
			start b
			// b is running here
			stop b
			// a is running again here
			start c
			stop c
			// a is running here
		stop a
	*/
	class timer_sentry {
		op_t prev_op, op;
	public:
		timer_sentry(op_t op_) :op(op_) 
		{
			prev_op=swap(op);
		}
		~timer_sentry() {
			swap(prev_op);
		}
	};
}; /* end benchmark namespace */


#endif
