/*
Orion's Standard Library-- Audio I/O
  Orion Sky Lawlor, olawlor@acm.org, 5/11/2000

Cross-platform audio interface description.
*/
#ifndef __OSL_AUDIO_H
#define __OSL_AUDIO_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif

namespace osl { namespace audio {

////////////////////////// Audio Samples ///////////////////////////

//This class represents a single audio Sample--
// sound pressure at a time at some Point in space.
//  It can represent integer values from min to max,
//  and 0.0 translates to "mean".
template <class storageType, int min,int mean,int max>
class Sample {
protected:
	typedef Sample<storageType,min,mean,max> thisType;
	storageType sto;
public:
	Sample(void) {}
	Sample(int val) {clip(val);}
	Sample(float val) {clip(val);}
	Sample(double val) {clip(val);}
	Sample(const thisType &other) {sto=other.sto;}
	thisType &operator=(int val) {clip(val);return *this;}
	thisType &operator=(float val) {clip(val);return *this;}
	thisType &operator=(double val) {clip(val);return *this;}
	thisType &operator=(const thisType &other) {sto=other.sto;return *this;}
	operator int() const {return int(sto)-mean;}
	operator float() const {return float(sto)-mean;}
	operator double() const {return double(sto)-mean;}
	void clip(int val)
	{
		val+=mean;
		if (val<min) sto=storageType(min); 
		else if (val>max) sto=storageType(max);
		else sto=storageType(val);
	}
	void clip(float val)
	{
		clip((double)val);
	}
	void clip(double val)
	{
		val+=mean;
		if (val<min) sto=storageType(min); 
		else if (val>max) sto=storageType(max);
		else sto=storageType(val);
	}
	void byteOrder(void) {//Swap the byte order in sto
		unsigned char *s=(unsigned char *)&sto;
		const int n=sizeof(storageType);
		for (int i=0;i<n/2;i++)
		{unsigned char tmp=s[i];s[i]=s[n-i-1];s[n-i-1]=s[i];}
	}
//Arithmetic:
	thisType &operator+=(const thisType &a)
		{sto+=a.sto-mean;return *this;}
	inline double operator+(const thisType &b) const
		{return double(sto)+double(b.sto)-2*mean;}
	inline double operator*(double a) const
		{return a*(double(sto)-mean);}
};

/// Flip "3.0*samp" around so it works the same as "samp*3.0".
template <class storageType, int min,int mean,int max>
inline double operator*(double a,const Sample<storageType,min,mean,max> &b)
		{return b*a;}

typedef Sample<short,-32768,0,32767> Sample16;//Typical 16-bit Sample (native endian-ness)

//This class represents a set of audio Samples--
// sound pressure at a time at two Points in space.
template <class SampleType>
class StereoSample {
protected:
	typedef StereoSample<SampleType> thisType;
public:
	SampleType left,right;
	StereoSample(void) {}
	StereoSample(const SampleType &Nleft,const SampleType &Nright)
		{left=Nleft;right=Nright;}
	void byteOrder(void) {//Swap the byte order in Samples
		left.byteOrder();right.byteOrder();}
	
//Arithmetic:
	thisType &operator+=(const thisType &a)
		{left+=a.left;right+=a.right;return *this;}
	thisType operator+(const thisType &b) const
		{return thisType(left+b.left,right+b.right);}
	thisType operator*(double a) const
		{return thisType(a*left,a*right);}
};

/// Flip "3.0*samp" around so it works the same as "samp*3.0".
template <class SampleType>
inline double operator*(double a,const StereoSample<SampleType> &b)
		{return b*a;}

typedef StereoSample<Sample16> Stereo16;//Typical 16-bit stereo Sample (native endian-ness)


typedef enum { //Sampling rates, in Samples/second
	RATE_8kHz=8000,//Telephone quality
	RATE_11kHz=11025,//Low quality
	RATE_22kHz=22050,//Good quality
	RATE_44kHz=44100,//CD quality
	RATE_48kHz=48000//DAT quality
} RATE_TYPE;

typedef enum {
	FORMAT_unsigned=0x00,//Samples are 0...2^n-1
	FORMAT_signed=0x01,//Samples are -2^(n-1)-1...0...2^n-1
	FORMAT_oneByte=0x02,//Samples are 8 bits each
	FORMAT_twoByte=0x04,//Samples are 16 bits each
	FORMAT_16bit=FORMAT_signed|FORMAT_twoByte//Signed 16-bit Samples
} FORMAT_TYPE;

//A low-level tool for setting the properties of an audio Device
class Control {
protected:
	double rate;
public:
	virtual ~Control();
	//Return the sampling rate actually used
	double getRate(void) {return rate;}

	//Use asynchronous (nonblocking) reads/writes
	virtual bool setAsync(bool doAsync=true);
	//Count the number of remaining buffers not yet played/filled
	virtual int buffersLeft(void);
};

//Abstract superclass for all audio Devices (input, output, or both)
class Device {
protected:
	Control *myControl;
	Device() {}
public:
	virtual ~Device();
	//Return the sampling rate actually used
	virtual double getRate(void);
	
	//Use asynchronous (nonblocking) reads/writes
	virtual bool setAsync(bool doAsync=true);
	//Count the number of remaining buffers not yet played/filled.
	// Only needed for async mode.  You can't touch your buffers
	// until this returns zero.
	virtual int buffersLeft(void);
};

//All the sound input and output Devices use the static constructor idiom.
// These build a new input off the given speaker/microphone number,
// an operating-system dependent number, starting at 0, the default.
// Returns NULL on invalid/nonexistent speaker/mic number.
class InputSample16:public Device {
protected: 
	InputSample16() {}
public: 
	//Build a new input Device listening to the given Mic
	static InputSample16 *create(int desiredRate,int micNo=0);
	virtual int record(int nSamp,Sample16 *dest) =0;
};
class InputStereo16:public Device {
protected: 
	InputStereo16() {}
public: 
	//Build a new input Device listening to the given Mic
	static InputStereo16 *create(int desiredRate,int micNo=0);
	virtual int record(int nSamp,Stereo16 *dest) =0;
};

class OutputSample16:public Device {
protected: 
	OutputSample16() {}
public: 
	//Build a new output Device playing to the given speaker
	static OutputSample16 *create(int desiredRate,int speakerNo=0);
	virtual int play(int nSamp,const Sample16 *src) =0;
};
class OutputStereo16:public Device {
protected: 
	OutputStereo16() {}
public: 
	//Build a new output Device playing to the given speaker
	static OutputStereo16 *create(int desiredRate,int speakerNo=0);
	virtual int play(int nSamp,const Stereo16 *src) =0;
};

//Starts a 100Hz (or so) counter-- polling version
void startIsoCounter(void);
inline double getIsoInterval(void) {return 0.01;}
extern volatile unsigned int isoCount;
inline unsigned int getIsoCount(void) {return isoCount;}

}; }; //<- end namespace osl::audio

#endif /* def(thisHeader) */
