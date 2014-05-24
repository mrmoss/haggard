/*
Orion's Standard Library
Orion Sky Lawlor, 10/9/2001

DESCRIPTION: Standard utility routines
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "osl/osl.h"

/***************** OS-Independent Utility Routines ************/

osl::Exception::Exception(ExceptionString why_,const SourceLocation &where_) 
	:why(why_), where(where_)
{}
osl::Exception::~Exception() {}
const char *osl::Exception::toString(void) const {
	
	return why.c_str();
}

void osl::Exception::printStackTrace(void) {
	fprintf(stderr,"%s (%s:%d)\n",why.c_str(),
		where.getFile(),where.getLine());
}

void osl::vassert_failed(const char *why,const char *file,int line)
{
	bad("Assertation failed: ",why,file,line);
}

#undef MALLOC
extern "C" void *MALLOC(size_t nBytes) {
	void *ret=malloc(nBytes);
	if (ret==NULL) {
		char len[40];
		sprintf(len,"%z bytes",nBytes);
		osl::bad("Malloc failure-- could not allocate ",len);
	}
	return ret;
}
#undef FOPEN
FILE *FOPEN(const char *fName,const char *perm) {
	FILE *f=fopen(fName,perm);
	if (f==NULL) {
		if (perm[0]=='r')
			osl::bad("Could not read file ",fName);
		else /* 'w' or 'a' */
			osl::bad("Could not create file ",fName);
	}
	return f;
}

double osl::TimerClass::time(timerMethod meth,int nIter)
{
	double start=osl::time();
	(this->*meth)(nIter);
	return osl::time()-start;
}

//Return the total time in seconds per iteration, using 
//  nSpend seconds to estimate.
double osl::TimerClass::timePerIter(timerMethod meth,double nSpend)
{
	double overhead=time(meth,0);
	double t,nThresh=0.5*nSpend;
	int nIt=1;
	while ((t=time(meth,nIt))<nThresh) {
		nIt*=2;
	}
	return (t-overhead)/nIt;
}
osl::String osl::TimerClass::descPerIter(timerMethod meth,double nSpend)
{
	double t=timePerIter(meth,nSpend);
	const char *tu="s", *fu="/s";
	if (t<0.1) {t*=1.0e3;tu="ms";fu="k/s";}
	if (t<0.1) {t*=1.0e3;tu="us";fu="m/s";}
	if (t<0.01) {t*=1.0e3;tu="ns";fu="g/s";}
	double f=1.0/t;
	char buf[200];
	sprintf(buf,"%.3f%s  %.3f%s",t,tu,f,fu);
	return String(buf);
}

/****************** Progress bar ***********************/
namespace osl {
class SlashRprogress : public Progress {
	typedef Progress super;
	bool printed;
protected:
	virtual void refresh(double fractionDone) {
		const int wid=75;
		char buf[wid+5];
		int i;
		int nDone=(int)(wid*fractionDone+0.5);
		for (i=0;i<nDone;i++) buf[i]='-';
		for (i=nDone;i<wid;i++) buf[i]=' ';
		buf[wid]=0;
		char prog[100];
		sprintf(prog," %.1f%% done",100.0*fractionDone);
		strcpy(&buf[wid/2-strlen(prog)/2],prog);
		buf[strlen(buf)]=' ';
		printf("|>%s<|\r",buf);
		fflush(stdout);
		printed=true;
	}
public:
	SlashRprogress(double max) :Progress(max) { 
		printed=false;
	}
	virtual ~SlashRprogress() {
		if (printed) printf("\n");
	}
};

Factory<Progress,double> *osl::Progress::ProgressFactory=
	new NewFactory<SlashRprogress,Progress,double>;

osl::Progress *osl::Progress::New(double maxVal) {
	return ProgressFactory->New(maxVal);
}
Factory<Progress,double> *osl::Progress::replaceFactory(Factory<Progress,double> *f) {
	Factory<Progress,double> *ret=ProgressFactory;
	ProgressFactory=f;
	return ret;
}

osl::Progress::Progress(double max_)
{
	startTime=lastDisplay=osl::time();
	displayInterval=0.5; //2 updates per second (default)
	max=max_;
}
void osl::Progress::update(double cur) {
	double curTime=osl::time();
	if (curTime>lastDisplay+displayInterval) {
		lastDisplay=curTime;
		double fractionDone=cur/max;
		if (fractionDone<0.0) fractionDone=0.0;
		if (fractionDone>1.0) fractionDone=1.0;
		refresh(fractionDone);
	}
}
osl::Progress::~Progress() {}

}; /*end namespace osl*/

#ifdef WIN32
/************** Win32-Specifics **************/
#include <sys/timeb.h>
#include <windows.h>

int osl::strcasecmp(const char *a,const char *b) 
{
	while (*a!=0) {
		if (*a != *b) {
			int au=toupper(*a), bu=toupper(*b);
			if (au!=bu) {
				if (au<bu) return -1; //a is earlier
				else return 1; //b is earlier or a is longer
			}
		}
		a++;b++;
	}
	if (*b!=0) return -1; //a is shorter
	return 0;
}

void osl::sleep(int secs)
{
	Sleep(secs*1000);
}

void osl::msleep(int msecs)
{
	Sleep(msecs);
}

double osl::time(void) { //This seems to give terrible resolution (60ms!)
	struct _timeb t;
	_ftime(&t);
	return t.millitm*1.0e-3+t.time*1.0;
}

#else
/************* UNIX-Specifics **************/
#  include <sys/types.h>
#  include <sys/time.h> //For gettimeofday time implementation
#  include <sys/select.h>
#  include <unistd.h>

void osl::sleep(int secs)
{
	::sleep(secs);
}
 
void osl::msleep(int msecs) {
	struct timeval tv;
	tv.tv_sec=msecs/1000;
	tv.tv_usec=1000*(msecs%1000);
	select(0,NULL,NULL,NULL,&tv);
}

//Wall-clock time-- seconds past epoch
double osl::time(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_usec*1.0e-6+tv.tv_sec*1.0;
}

#endif

