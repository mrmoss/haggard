/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 2/20/2000

Image correlation routines.
*/
#ifndef __OSL_IMAGE_MATCH_H
#define __OSL_IMAGE_MATCH_H

// #include <math.h>
#include "osl/graphics.h"

namespace osl { namespace match {

using osl::graphics2d::Raster;
using osl::graphics2d::FloatRaster;
using osl::graphics2d::Color;
using osl::graphics2d::FlatRasterT;
using osl::Vector2d;

//This is a complex image-- it's used for FFT'ing.
//  I'm using this rather than std::complex to avoid
//  portability problems.
class fComplex {
public:
	float real;
	float imag;
	fComplex(float Nreal,float Nimag) {real=Nreal;imag=Nimag;}
	fComplex(float Nreal) {real=Nreal;imag=0.0;}
	fComplex() {real=imag=0.0;}
	fComplex operator~(void) const //Conjugate
		{return fComplex(real,-imag);}
	fComplex &operator*=(double d)
		{real*=d;imag*=d;return *this;}
	double magSqr(void) const {return real*real+imag*imag;}
	double mag(void) const {return sqrt(magSqr());}
};
inline fComplex operator+(const fComplex &a,const fComplex &b)
	{return fComplex(a.real+b.real,a.imag+b.imag);}
inline fComplex operator*(const fComplex &a,const fComplex &b)
{
	return fComplex(a.real*b.real-a.imag*b.imag,
		a.real*b.imag+a.imag*b.real);
}

/**
  This is an image consisting of complex Pixels.
*/
class ComplexRaster : public FlatRasterT<fComplex> {
	typedef FlatRasterT<fComplex> super;
	int mx,my;  //wid==1<<mx; ht==1<<my;
	void setSize(int w,int h);
public:
	/// Create an empty image of this size.
	/// Wid and ht must be powers of two!
	ComplexRaster(int w,int h);
	
	/// Set the real part of us to this raster's channel,
	///   and set the imaginary part to zero.
	/// If the image is not a power of two size,
	///   pad with zeros out to next largest power of two
	ComplexRaster(const Raster &r,int channel,int atLeastW=0,int atLeastH=0);
	
	/// Shallow copy this subimage
	ComplexRaster(int w,int h,ComplexRaster &parent,int x,int y);
	
	/**
	  Replace this image with its FFT.
	  The image DC coefficient is stored at 
	  pixel (0,0); the x-and-y-Nyquist coefficient 
	  is at pixel (wid/2,ht/2).
	*/
	void fft(void);
	void ifft(void);
	
	//All these operations assume a presized buffer
	//Always legal for dest to alias this or b.
	//Set dest=this + b.  
	void sum(const ComplexRaster &b,ComplexRaster &dest) const;
	//Set dest=this * b. 
	void product(const ComplexRaster &b,ComplexRaster &dest) const;
	//Set dest=this * ~b. 
	void conjugateProduct(const ComplexRaster &b,ComplexRaster &dest) const;
	
	//Extract into this presized amplitude image
	void getAmplitude(FloatRaster &dest) const;
	
	//These don't make much sense
	virtual Color getColor(int x,int y) const;
	virtual void setColor(int x,int y,const Color &c);
};

//Add this FloatRaster into the accumulator
void floatAccum(const FloatRaster &src,FloatRaster &accum);

//Estimate the amplitude Peak of this image
class Peak {
public:
	Vector2d offset;
	double strength;
	Peak(const FloatRaster &f);
};


/**
* Use this when you will be correlating several 
*  small patches of the same size against the same big patch.
* Saves the fft'd big patch and scaling factors.
*
* If useNormalized is true, calculates correlation image as
*    corr = sum (big * lil) / (sum(big)+epsilon)
* This is a rather botched attempt at normalized correlation,
*    corr =    sum((big-bigAve)*(lil-lilAve)) / 
*           sqrt(sum(big-bigAve)^2 * sum(lil-lilAve)^2)
* Normalized correlation works even when the two images have
* different intrinsic brightnesses.
*
* If useNormalized is false, calculates correlation as the 
* sum-of-squared differences:
*    corr = sum((big - lil)^2)=sum(big^2 - 2*lil*big +lil^2)
*/
class Correlator {
	ComplexRaster fftBig;//FFT'd big image (8 bytes per Pixel, power-of-two)
	bool useNormalized;
	/* This is a normalization map over the "big" image:
	    useNormalized==true -> scale==1.0/(sum(big)+epsilon)
	    false -> scale==sum(big^2)
	*/
	FloatRaster scale;//Correlation normalization image (4 bytes per Pixel)
	int channel;//Color channel to use
	
	//Make an image whose Pixels compensate a correlation over the given image
	void setScale(const Raster &big,int channel,int lilw,int lilh);
	
	//Normalize this correlation image by multiplying by the given scale image.
	void normalizeCorr(const ComplexRaster &corr,const Raster &lil,FloatRaster &nCorr) const;
public:
	Correlator(const Raster &big,int channel,int lilw,int lilh,bool useNormalized=true);
	virtual ~Correlator();
	
	/// Compute the correlation image of big with lil.
	///  Returns the correlation image in corr.
	virtual void correlate(const Raster &lil,FloatRaster &corr) const;
	
	/// Compute the correlation image of big with lil, given this lil mask.
	///   (mask==0 -> ignore lil; mask==1 -> use lil)
	///  Returns the correlation image in corr.
	virtual void correlateMask(const Raster &lil,const Raster &mask,
		FloatRaster &corr) const;
	
	/// Compute the offset from big to lil
	virtual Peak correlatePeak(const Raster &lil) const;
};

/// As above, but use all three Color channels
class ColorCorrelator : public Correlator{
	Correlator g,b;//Green and blue channel Correlators
public:
	ColorCorrelator(const Raster &big,int lilw,int lilh,bool useNormalized=true);
	//Compute the correlation image of big with lil
	virtual void correlate(const Raster &lil,FloatRaster &corr) const;
};

/// Return the offset between these two color images.
///  The offset is the topleft location of spot in ref's image.
template <class SpotRaster>
inline Vector2d ColorCorrelate(const Raster &ref,const SpotRaster &spot,bool useNormalized=true) {
	// Shink spot a bit (prevents image edges from interfering)
	int dx=spot.wid/10, dy=spot.ht/10;
	SpotRaster lilSpot(spot.wid-2*dx,spot.ht-2*dy, *(SpotRaster *)&spot, dx,dy);
	ColorCorrelator cc(ref,lilSpot.wid,lilSpot.ht,useNormalized);
	return cc.correlatePeak(lilSpot).offset-Vector2d(dx,dy);
}


//-------- old C API ---------
/*
rgbMatch: finds the offset which makes the 
 given small image line up best inside the given big image.
 (offX,offY) is the topleft corner of "lil" in 
 "big" image when they both line up.
*/
void rgbMatch(const Raster &big,
	const Raster &lil,
	const Raster *lilMask,
	double *offX,double *offY);

/*
greyMatch: as rgbMatch, but greyscale.
*/
void greyMatch(int channel,
	const Raster &big,
	const Raster &lil,
	const Raster *lilMask,
	double *offX,double *offY);

//********** Implementation utilties: ***************

/*Convert the given search and spot into a 
complex correlation image.  (x,y) in double component 
of correlation image will be the product of search 
and a shifted version of spot so the topleft of
spot in search coordinates is (x,y).
*/
void greyCorr(int channel,
	const Raster &search,
	const Raster &spot,
	const Raster *spotMask,
	FloatRaster &correlation);

/*Return the location of the maximum value of 
the double component of the given Complex image.
Returns the value of the correlation Peak
*/
double corrPeak(const FloatRaster &corr,
	double *x,double *y);


}; }; //End namespace

#endif /*def(thisHeader)*/

