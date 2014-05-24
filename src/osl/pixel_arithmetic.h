/*
Orion's Standard Library
Orion Sky Lawlor, 2004/7/28

Highly optimized pixel arithmetic routines.
These are fairly hairy, as they deal with fixed-point 
directly for speed.  If you want dynamic range and simplicity
(at the cost of some speed), use osl/color.h, which is 
floating-point.
*/
#ifndef __OSL_PIXEL_ARITHMETIC_H
#define __OSL_PIXEL_ARITHMETIC_H

#ifndef __OSL_FAST_MATH_H
#include "osl/fast_math.h"
#endif

namespace osl { namespace graphics2d {

/***************** Fixed-point Basics *************/
/**
This is a fixed-Point number-- the low bits 
are an 8 bit fraction field. 
   0 -> 0.0; 255 -> 1.0
*/
class fix8Fast {
	unsigned int v;
public:
	const static fix8Fast one;
	explicit fix8Fast(double x) {v=fastRound(x*255);}
	//Prescaled input (& default conversion)
	fix8Fast(unsigned int x) {v=x;}
	explicit fix8Fast(unsigned char c) {setByte(c);}
	explicit fix8Fast(unsigned short c) {setShort(c);}
	
	//From byte-- maps 0..255 to 1..256 efficiently
	inline void setByte(unsigned char c) { v=c; }
	inline void setShort(unsigned short c) { v=(c>>8); }
	unsigned int toInt(void) const {return v;}
	unsigned int val(void) const {return v;}
	fix8Fast &operator=(int x) {v=x;return *this;}
	
	/// Return a fix8Fast decreased by this alpha:
	inline fix8Fast alpha(unsigned short a) const
	{
		return fix8Fast(((v+1)*(0xffffu-a))>>16u);
	}
};
inline fix8Fast operator+(const fix8Fast &a,const fix8Fast &b)
		{return fix8Fast(a.val()+b.val());}
inline fix8Fast operator-(const fix8Fast &a,const fix8Fast &b)
		{return fix8Fast(a.val()-b.val());}
/**
  Multiply two fixed-point values.
  Subtle: we have to add 1 to one of the values, because
  we truncate out the low 8 bits of the product.  We take
  the convention that we add 1 to the first operand.
  Note that this makes multiplication slightly (by +-1) non-commutative;
  but not for 0 or 1, which are the really important values.
*/
inline fix8Fast operator*(const fix8Fast &a,const fix8Fast &b)
		{return fix8Fast(((1+a.val())*b.val())>>8);}

/***************** Pixel Basics *************/
/// Addition without overflow check
inline RgbaPixel operator+(const RgbaPixel &a,const RgbaPixel &b)
		{return RgbaPixel(a.val()+b.val());}

/** 
  Higher-precision (16 bit channel) version of a 32-bit,
  for fast and accurate sums and products of pixels.
*/
class PixelAccum16 {
	/// These two SWAR words store the 4 channels of the color,
	///    as 4 16-bit back-to-back channels, 2 in each int.
	/// Which channels go in which int depends on the
	///    exact pixel layout, and is normally irrelevant.
	unsigned int even, odds;
	
	/// Mask out components of an RgbaPixel color.
	enum { 
	   evenMask=0x00ff00ffu,
	   oddsMask=0xff00ff00u,
	   maskRound=0x007f007fu
	};
public:
	inline PixelAccum16() :even(0), odds(0) {}
	inline PixelAccum16(unsigned int e,unsigned int o) :even(e), odds(o) {}
	
	/// Initialize us from this 32-bit pixel value.
	///  The pixel must have 8 bit fields, but other than
	///  that, nothing matters.  It can be Rgba, Rgb, Bgra, etc.
	inline PixelAccum16(unsigned int pixel32) 
		:even((pixel32&evenMask)),
		 odds((pixel32&oddsMask)>>8) {}
	
	/// Add this accumulator to us.  Exact if sum is in-range.
	inline void add(const PixelAccum16 &p) {
		even+=p.even;
		odds+=p.odds;
	}
	/// Scale the values in this accumulator by this value.
	///   Exact if product is in 16 bit range.
	inline void multiply(unsigned int by) {
		even*=by;
		odds*=by;
	}
	
	/**
	  Truncate and copy out high-order 8 bits of each channel
	  as a 32-bit pixel.  Low-order bits are dropped.
	*/
	inline unsigned int outHigh(void) const {
		return  ((even>>8)&evenMask)+
			((odds   )&oddsMask);
	}
	
	/**
	  Truncate and copy out high-order 8 bits of each channel
	  as a 32-bit pixel.  Low-order bits are rounded to nearest.
	*/
	inline unsigned int outHighRound(void) const {
		return  (((even+maskRound)>>8)&evenMask)+
			(((odds+maskRound)   )&oddsMask);
	}
	
	
	
	/**
	  Truncate and copy out low-order 8 bits of each channel
	  as a 32-bit pixel.
	  WARNING: high-order bits ignored.
	*/
	inline unsigned int outLow(void) const {
		return  ((even)&evenMask)+
			((odds<<8)&oddsMask);
	}
};

/**
  Scale each channel of a by a constant .8 fixed-point value
  Equivalent to:
  	return RgbaPixel(Color(a)*b);
*/
inline RgbaPixel operator*(const RgbaPixel &a,const fix8Fast &b)
{
	PixelAccum16 prod(a);
	prod.multiply(b.val()+1); /* the +1 is because we truncate */
	return RgbaPixel(prod.outHigh());
}
inline RgbaPixel operator*(const fix8Fast &b,const RgbaPixel &a)
  { return a*b; }
inline RgbaPixel operator*(double b,const RgbaPixel &a)
  { return a*fix8Fast(b); }


/**
  Return d=s+d*f
  Does not handle overflow.
*/
inline RgbaPixel scaleAdd(const RgbaPixel &s,const RgbaPixel &d,
		fix8Fast f) 
{
		return s+d*f;
}

/**
  Compose two premultiplied-alpha Pixels-- source over dest.
   This is equivalent to 
        d = RgbaPixel(Color(d).blend(Color(s)));
    \param s Pixel to read top color from, which will be blended over d.
    \param d Pixel to read under color from, and write blend to.
*/
inline void blend(const RgbaPixel &s,RgbaPixel &d) {
	unsigned int sa=s.a();
	if (sa==255u)
		d=s;
	else
		d=s+d*fix8Fast(255u-sa);
}

/**
  Compose two premultiplied-alpha Pixels-- source over dest.
   This is equivalent to 
        d = RgbaPixel(Color(d).blend(Color(s).scaleAlpha(overAlpha)));
    \param s Pixel to read top color from, which will be blended over d.
    \param d Pixel to read under color from, and write blend to.
    \param overAlpha Scaling to apply to s's alpha, in .16 format.
                     65535 -> s alpha unchanged; 32768 -> s alpha drops by half.
*/
inline void blend(const RgbaPixel &s,RgbaPixel &d,unsigned short overAlpha) {
	unsigned int sa=s.a();
	if (sa==255u && overAlpha==65535u)
		d=s;
	else if (overAlpha==65535u) {
		d=s+d*fix8Fast(255u-sa);
	} else {
		fix8Fast overA(overAlpha);
		RgbaPixel sp=s*overA;
		d=sp+d*fix8Fast(255u-sp.a());
	}
}

/********************* Bilinear Interpolation ********************/

/**
   Fetch and interpolate four pixels:
      (ixL,iyU)  (ixR,iyU)
      (ixL,iyD)  (ixR,iyD)
   Interpolate along x by dx, along y by dy (dx, dy both .8 fixed-point).
   Normally called like
      fix8Interpolate(src, x,x+1, y,y+1, dx,dy);
   e.g., dx==0, dy==0 -> return (ixL,iyU)
         dx==1, dy==1 -> return (ixR,iyD)
*/
inline RgbaPixel fix8Interpolate(const RgbaRaster &src,
	int ixL,int ixR,int iyU,int iyD,unsigned int dx,unsigned int dy)
{
	const RgbaPixel *up=&src.at(0,iyU);
	const RgbaPixel *dn=&src.at(0,iyD);
#if 0 /* Confuses gcc4.0: /tmp/ccDt5llz.s:285: Error: missing ')' */

&& 
OSL_USE_MMX_GNU /*x86 MMX Instructions; GNU assembler format*/
	/* Implements: for all channels,
		u=up[0]+dx*(up[1]-up[0]);
		l=lo[0]+dx*(lo[1]-lo[0]);
		result=u+dy*(l-u);
	The business with shifting by 4 is needed because we can only
	get the low 16 or high 16 bits of a product-- the low 16 bits
	cuts off the sign bit, so we shift up and use the high 16 bits.
	*/
	unsigned int result;
	RgbaPixel ul=up[ixL], ur=up[ixR];
	RgbaPixel dl=dn[ixL], dr=dn[ixR];
	unsigned long long int ldx=(dx)>>1;
	unsigned long long int ldy=(dy)<<7;
	// Make 4 copies of dx and dy:
	ldx+=(ldx<<16); ldx+=(ldx<<32);
	ldy+=(ldy<<16); ldy+=(ldy<<32);
	unsigned long long int lround=0x003f003f003f003fllu;
	asm (
		"pxor %%mm4, %%mm4\n\t" // zero mm4
		
		//Load source Pixels
		"movd %1, %%mm0 \n\t punpcklbw %%mm4, %%mm0\n\t"// mm0 = ul
		"movd %2, %%mm1 \n\t punpcklbw %%mm4, %%mm1\n\t"// mm1 = ur
		"movd %3, %%mm2 \n\t punpcklbw %%mm4, %%mm2\n\t"// mm2 = dl
		"movd %4, %%mm3 \n\t punpcklbw %%mm4, %%mm3\n\t"// mm3 = dr
		
		"movq     (%5), %%mm5\n\t" // mm5 = dx/* 7 bit unsigned */
		"movq     (%6), %%mm6\n\t" // mm6 = dy
		
		//Horizontal blends-- up and lo
		"psubw  %%mm0, %%mm1\n\t"  // mm1 = ur-ul/* 8 bit signed */
		"pmullw %%mm5, %%mm1\n\t"  // mm1 = dx*(ur-ul)/* 15 bit */
		"psllw     $7, %%mm0\n\t" // bias ul up to 15 bits
		"paddw  %%mm0, %%mm1\n\t" // mm1 = u = ul+dx*(ul-ur)
		
		"psubw  %%mm2, %%mm3\n\t"  // mm1 = dr-dl/* ditto */
		"pmullw %%mm5, %%mm3\n\t" // mm1 = dx*(dr-dl)
		"psllw     $7, %%mm2\n\t" // bias dl up to 15 bits
		"paddw  %%mm2, %%mm3\n\t" // mm1 = d = dl+dx*(dr-dl)
		
		"movq     (%7), %%mm5\n\t"  // mm5 = lround/* rounding field */
		
		//Vertical blend
		"psubw  %%mm1, %%mm3\n\t"  // mm3 = l-u/* 15 bit */
		"pmulhw %%mm6, %%mm3\n\t"  // mm3 = dy*(l-u)/* 14 bit */
		"paddw  %%mm3, %%mm3\n\t" // bias product up to 15 bit 
		"paddw  %%mm5, %%mm1\n\t" // add dx (random bias, avoids truncation error) 
		"paddw  %%mm1, %%mm3\n\t" // mm3 = result = u+dy*(l-u)
		"psrlw     $7, %%mm3\n\t" // truncate down to 8 bit
		
		//Repack result
		"packuswb %%mm4, %%mm3\n\t"
		"movd %%mm3, %0\n\t"
		"emms\n\t" //<-- allows us to use floating-Point registers again
		: "=r" (result) : 
			"g" (ul), "g" (ur), "g" (dl), "g" (dr), 
			"g" (&ldx), "g" (&ldy), "g" (&lround)
	);
        return RgbaPixel(result);
#else
	fix8Fast x(dx), y(dy);
	fix8Fast ex(255u-dx),   ey(255u-dy);
	return  up[ixL]*ex*ey+up[ixR]* x*ey+
	        dn[ixL]*ex* y+dn[ixR]* x* y;
#endif
}

/// Fetch and interpolate bytes around (in-bounds) ix,iy
/// Interpolate in x by dx, in y by dy (both .8 fixed-point)
inline byte fix8Interpolate(const AlphaRaster &r,int ixL,int ixR,
	int iyU,int iyD,int dx,int dy)
{
	const byte *up=&r.at(0,iyU);
	const byte *lo=&r.at(0,iyD);
	int upAve=up[ixL]+((dx*(up[ixR]-up[ixL]))>>8);
	int loAve=lo[ixL]+((dx*(lo[ixR]-lo[ixL]))>>8);
	return (byte)(upAve+((dy*(loAve-upAve))>>8));
}


/**
  Perform a fixed-point bilinear interpolation on this raster.
  (x,y) is the coordinate to read; pixel (1,2) is centered at (1.5,2.5).
  Coordinate need not be in-bounds; out-of-bounds pixels will be pinned
  to the boundary.
      \param p Raster to read from.
      \param x Fixed-point x pixel location, .8 format.  
      \param y Fixed-point y pixel location, .8 format.
*/
template <class RET,class RAST>
inline RET fix8Pin(const RAST &p,int x,int y) {
	x-=128;y-=128;//Shift by half a Pixel
	if (x<0) x=0; if (y<0) y=0; 
	unsigned int ix=x>>8,     iy=y>>8;
	         int dx=x&0xff, dy=y&0xff;
	if (ix>=(unsigned int)(p.wid-1)) {ix=p.wid-2;dx=0xff;}
	if (iy>=(unsigned int)(p.ht -1)) {iy=p.ht -2;dy=0xff;}
	return fix8Interpolate(p,ix,ix+1,iy,iy+1,dx,dy);
}

/**
  Perform a fixed-point bilinear interpolation on this raster.
  (x,y) is the coordinate to read; pixel (1,2) is centered at (1.5,2.5).
  Coordinate need not be in-bounds; out-of-bounds pixels will 
  wrap around the boundary IF the image size is a power of two.
      \param p Raster to read from.
      \param x Fixed-point x pixel location, .16 format.  
      \param y Fixed-point y pixel location, .16 format.
*/
template <class RET,class RAST>
inline RET fix8Wrap(const RAST &p,int x,int y) {
	x-=128;y-=128;//Shift by half a Pixel
	unsigned int ix=(x>>8)&(p.wid-1), iy=(y>>8)&(p.ht-1);
	         int dx=x&0xff, dy=y&0xff;
	return fix8Interpolate(p,ix,(ix+1)&(p.wid-1),iy,(iy+1)&(p.ht-1),dx,dy);
}




/// Take the product of the components of these two pixel values.
///   For example, a could be the incoming light, and b the albedo.
///   Alpha channel is copied directly from b.
inline RgbaPixel modulate(RgbaPixel a,RgbaPixel b) {
        return RgbaPixel(
                (a.r()*(b.r()+1))>>8, 
                (a.g()*(b.g()+1))>>8, 
                (a.b()*(b.b()+1))>>8,
		b.a()
        );
}
inline RgbaPixel operator*(RgbaPixel a,RgbaPixel b) {
	return modulate(a,b);
}

/**
   Interpolate two Pixels a and b by f-- 
   	a if f is 0.0, and b if f is 1.0.
   Equivalent to
       return RgbaPixel(Color(a)*(1.0-f)+Color(b)*f);
    \param a,b Pixels to read color from.
    \param f Fraction of s to return.
*/
inline RgbaPixel lerp(const RgbaPixel &a,const RgbaPixel &b,fix8Fast f)
{
	return a*(255u-f)+b*f;
}


/***************** Colors ****************/

/// Linearly interpolate these two colors: return s+f*(d-s)
inline Color lerp(const Color &a,const Color &b,float f)
{
	return Color(
		a.r+f*(b.r-a.r),
		a.g+f*(b.g-a.g),
		a.b+f*(b.b-a.b),
		a.a+f*(b.a-a.a)
	);
}

/****************** 16-bit pixels ***********/
/**
   Return a+bFrac8*(b-a), where bFrac8 is .8 fixed-point.
*/
inline RGBA_Pixel16 lerp(RGBA_Pixel16 a,RGBA_Pixel16 b,unsigned int bFrac8)
{
	return RGBA_Pixel16(
		a.r()+(((b.r()-a.r())*bFrac8)>>8),
		a.g()+(((b.g()-a.g())*bFrac8)>>8),
		a.b()+(((b.b()-a.b())*bFrac8)>>8),
		a.a()+(((b.a()-a.a())*bFrac8)>>8)
	);
}


}; };


#endif
