/*
Orion's Standard Library
Orion Sky Lawlor, 9/23/1999
Public domain source file.

Main include file-- includes common utility
classes; defines handy types.
*/

#ifndef __OSL_H
#define __OSL_H

/**********************************
 System-standard include files*/
#include "osl/config.h"
#include <stdio.h> /*for fopen...*/
#include <stdlib.h> /*for malloc, free, exit...*/
#include <math.h> /*for sin, atan, abs...*/

/*Define M_PI if it isn't already...*/
#ifndef M_PI
#  define M_PI 3.14159265358979323
#endif
#ifndef PI
#  define PI 3.14159265358979323
#endif

#if 0 
//Ignore request for error-checking runtime utilities:
//  use the raw versions.
#define MALLOC(x) malloc(x)
#define FREE(x) free(x)
#define FOPEN(n,p) fopen(n,p)
#else
//Regular, error-checking versions of runtime:
#ifdef __cplusplus
extern "C" {
#endif
  void *MALLOC(size_t nBytes);
# define FREE(x) free(x)
  FILE *FOPEN(const char *fName,const char *perm);
#ifdef __cplusplus
};
#endif
#endif

#ifdef __cplusplus
#include <string>

namespace osl {

/*Basics:*/
	typedef unsigned char byte;
	
/*I'm not sure what to do about these:*/
	typedef std::string String;
	typedef int Char;

	inline String toString(int i) { 
		char buf[50]; sprintf(buf,"%d",i); return buf;
	}
	
	/* Return true if the string A ends with the string B, that is,
	       A == X B
	   for some string X.
	*/
	inline bool ends_with(const std::string &A,const std::string &B) {
		if (A.size()<B.size()) return false;
		else return B==std::string(A,A.size()-B.size());
	}
	
	//Describes a location in the program source code
	class SourceLocation {
		const char *file; int line;
	public:
		SourceLocation(const char *sourceFile,int sourceLine)
			:file(sourceFile), line(sourceLine) {}
		const char *getFile(void) const {return file;}
		int getLine(void) const {return line;}
	};
/*Builds a SourceLocation.  This has to be a macro to use __FILE__ properly.*/
#define OSL_MAKE_SOURCELOCATION osl::SourceLocation(__FILE__,__LINE__)

	typedef const char *ExceptionString;
	class Exception {
		String why;
		SourceLocation where;
	public:
		Exception(ExceptionString why_,const SourceLocation &where_);
#define OSL_EXCEPTION_DECL(myName,parentName) \
		myName(ExceptionString why_,const SourceLocation &where_) \
			:parentName(why_,where_) {}
		~Exception();
		//Just return why, not where
		const char *toString(void) const;
		const SourceLocation &getLocation(void) const {return where;}
		//Print out a stack trace
		void printStackTrace(void);
	};	
	
	class FormatException : public Exception {
	public:
		OSL_EXCEPTION_DECL(FormatException,Exception)
	};
	
#define OSL_THROW(exceptionType,whyString) \
	osl::Throw(new exceptionType(whyString,OSL_MAKE_SOURCELOCATION))
	
	//You can set debugger breakpoints here to see problems before they're thrown
	void Throw(Exception *e);
	
/*Namespace-ize defines from osl/config.h*/
	typedef osl_int16 int16;
	typedef osl_int32 int32;
	typedef osl_int64 int64;
	typedef osl_uint16 uint16;
	typedef osl_uint32 uint32;
#if OSL_HAS_UINT64
	typedef osl_uint64 uint64;
#endif
	
#ifdef WIN32
//Win32 system
	int strcasecmp(const char *a,const char *b);
	void sleep(int secs);
	void msleep(int msecs);
	double time(void);
// Remove stupid MS namespace pollution:
#undef max
#undef min
#else 
//UNIX system
	void sleep(int secs);
	void msleep(int msecs);
	double time(void);
#endif
	inline bool isLittleEndian(void) {
#if OSL_LIL_ENDIAN /*Always little*/
		return true;
#elif OSL_BIG_ENDIAN /*Always big*/
		return false;
#else /*Decide at run time*/
		int i=1;
		return 1==*(char *)&i;
#endif
	}
	template <class T>
	inline T max(T a,T b) { if (a>b) return a; else return b; }
	template <class T>
	inline T min(T a,T b) { if (a<b) return a; else return b; }
	template <class T>
	inline void swap(T &a,T &b) { T tmp(a); a=b; b=tmp; }
	/// Round up to nearest power of two (caution: hangs for giant v!)
	inline int roundUp2(int v) { int ret=1; while (ret<v) ret*=2; return ret; }
	
/*Java compatability:*/
	typedef bool boolean;
#	ifndef null
#	  define null 0
#	endif

/*Debugging utilities:*/
	
	//A more verbose form of "abort()"
	void bad(const char *why1,const char *why2="",
		const char *file=NULL,int line=-1);
	void vassert_failed(const char *why,const char *file,int line);

#ifndef NDEBUG
	//A "verbose" assertation-- gives some indication of why the assertation failed
#define vassert(what,why) do {\
	if (!(what)) osl::vassert_failed((why),__FILE__,__LINE__);\
	} while (0)
#else
#define vassert(what,why) /*empty*/
#endif

/*Pattern support*/
//A Factory generates instances of another class
template <class instance,class arg_t>
class Factory {
public:
	virtual ~Factory() {}
	virtual instance *New(const arg_t &a) =0;
};

template <class subInstance,class superInstance,class arg_t>
class NewFactory : public Factory<superInstance,arg_t> {
public:
	virtual superInstance *New(const arg_t &a) {
		return new subInstance(a);
	}
};

/// Subclasses consume objects of type T.
template <class T>
class VirtualConsumer {
public:  virtual void consume(const T &t) =0;
	virtual ~VirtualConsumer() {} /* <- silence compiler warning */
};

/// This class consumes and ignores objects of type T.
template <class T>
class VirtualIgnorer : public VirtualConsumer<T> {
public:  void consume(const T &t) {}
};

/// Documentation-only class: no copy constructor, no assignment operator.
class Noncopyable {
	//Don't use these:
	Noncopyable(const Noncopyable &c);
	void operator=(const Noncopyable &c);
public:
	Noncopyable() {}
};

/// A simple progress indicator
  class Progress {
	double startTime,lastDisplay;//Times in seconds
	double displayInterval;
	double max;//Value at end of run
	static Factory<Progress,double> *ProgressFactory;
  protected:
	virtual void refresh(double fractionDone) =0;
	Progress(double max_); 
  public:
	//Standard usage: create it...
	static Progress *New(double maxVal=1.0);
	//...call update occasionally...
	virtual void update(double curVal);
	//...when done, delete it.
	virtual ~Progress();

	//Change the time between refreshes
	void setDisplayInterval(double secDelay) {displayInterval=secDelay;}

	//Change the default indicator source to f
	//  Returns the old Factory.
	static Factory<Progress,double> *replaceFactory(Factory<Progress,double> *f);
  };

/// For accurately estimating the time taken per iteration of a method  
  class TimerClass {
  public:
  	typedef double (TimerClass::*timerMethod)(int nIter);
	
  	//Return the total time in seconds per iteration, using 
  	//  nSpend seconds to estimate.
  	double timePerIter(timerMethod meth,double nSpend=0.1);
  	String descPerIter(timerMethod meth,double nSpend=0.1);
  private:
  	double time(timerMethod meth,int nIter);
  };

};
#endif

#endif /* __OSL_H*/

