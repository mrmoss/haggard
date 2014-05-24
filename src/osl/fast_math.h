/*
Math routines that use various hacks to provide 
amazing speed.

Orion Sky Lawlor, olawlor@acm.org, 2004/2/17
*/
#ifndef __OSL_FAST_MATH_H
#define __OSL_FAST_MATH_H

namespace osl {

/********************* Floating point hackery ********************/
/**
 * Generic floating point to integer conversion routine.
 *  returns floor((src+delta)*(1<<shift));
 *
 * Uses the IEEE floating point mantissa normalization trick--
 * by adding a large floating point number, we can shift the 
 * integer bits we're after into any bit position in the mantissa,
 * then extract them.  This means one floating-point add 
 * serves as both an add and a bit shift.  This is about 4x faster 
 * than a plain (int) cast, and about 6x faster than calling "floor".
 *
 * WARNING #1:
 * Only returns the low 22 bits of the signed result (to +-4 million); 
 * over or underflow will corrupt the exponent and give a totally 
 * bogus answer.
 *
 * WARNING #2:
 * With -msse3, the bitwise crap isn't any faster than just calling "floor".
 *
 * WARNING #3:
 * With gcc 4.0, the fake "return (*(int *)&f)" isn't actually honored.
 */
inline int fastIntegerConvert(double src,double delta,int shift)
{
#if 0
	return (int)floor((src+delta)*(1<<shift));
#else
	const double hakBasic=1.5*(1<<23)-0.5;  /* shift integer bits to end of word */
	const double hakShift=hakBasic/(1<<shift)+delta;
	const int hakExponent=127+23-shift; /* IEEE exponent field */
	const int hakMantissa=1<<22; /* IEEE mantissa: for the ".5" in 1.5 */
	const int hakMask=(hakExponent<<23)|hakMantissa; /* float version of 1.5*2^(23-shift) */
	float f=(float)(src+hakShift); /* convert to float */
	// return (*(int *)&f)-hakMask; /* cast to integer and subtract off exponent (gcc 3.x) */
	// New gcc 4.0-compatible float-to-int code:
	union {
		float f;
		int i;
	} float_to_int;
	float_to_int.f=f;
	return float_to_int.i-hakMask;
#endif
}

/**
 * Round this floating point value *down* to the nearest integer.
 *  E.g., fastFloor(3.8)=3, but fastFloor(-3.8)=-4.
 * WARNING: only works up to 22 significant bits; larger values will freak out.
 */
inline int fastFloor(double src) {
	return fastIntegerConvert(src,0,0);
}

/**
 * Round this floating point value to the nearest integer.
 *  E.g., fastFloor(3.8)=4, fastFloor(3.4)=3.
 * WARNING: only works up to 22 significant bits; larger values will freak out.
 */
inline int fastRound(double src) {
	return fastIntegerConvert(src,0.5,0);
}

/**
 * Round this floating point value *down* to the nearest .8 fixed-point integer.
 *  E.g., fastFloor8(3.0)=3*(1<<8);
 * WARNING: only works up to 22 significant bits; larger values will freak out.
 */
inline int fastFloor8(double src) {
	return fastIntegerConvert(src,0,8);
}

/**
 * Round this floating point value *up* to the nearest integer.
 *  E.g., fastCeil(3.2)=4, but fastCeil(-3.2)=-3.
 * WARNING: only works up to 22 significant bits; larger values will freak out.
 */
inline int fastCeil(double src) {
	return fastIntegerConvert(src,0.9999,0);
}

/**
 * Take this floating-point number modulo 1.0.
 * This is a "positive mod"-- the result is always on [0,1).
 *  e.g., fastMod1(1.2)=0.2, but fastMod1(-1.2)=0.8.
 * WARNING: only works up to 22 significant bits; larger values will freak out.
 */
inline double fastMod1(double src) {
	return src-fastIntegerConvert(src,0,0);
}

};

#endif
