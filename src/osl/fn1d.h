/*
 Templated 1-D numerical zero-finding routines.
 
 Orion Sky Lawlor, olawlor@acm.org, 2003/5/27
*/
#ifndef __OSL__FN1D_H
#define __OSL__FN1D_H

namespace osl {

/**
 * Use a generalized bisection method to find a zero of this
 * 1D function.  The sign of the function must differ at the
 * two endpoints. 
 *   @param btw Chooses test locations, given the previous bounds.
 *   @param f The function whose zero we seek.
 *   @param l The leftmost boundary of the search interval.
 *   @param r The rightmost boundary of the search interval.
 *   @param fl f(l).
 *   @param fr f(r).
 *   @param tol If fabs(f(x))<tol, x is considered a zero.
 * For example, zero(secant, fn, 0, 1, fn(0), fn(1)) returns the 
 * zero of fn on the interval [0,1].
 */
template <class BETWEEN,class FN,class real>
real zero(BETWEEN btw,FN f,real l,real r,real fl,real fr,real tol=1.0e-7) {
	// vassert(fl*fr<0, "Function's sign must differ at endpoints");
	while (fabs(l-r)>tol) {
		real x=btw(l,r,fl,fr);
		real fx=f(x);
		if (fabs(fx)<tol) return x;
		if (fl*fx<0) /* sign change between left and x */
			{ r=x; fr=fx; }
		else /* sign change must be between x and right */
			{ l=x; fl=fx; }
	}
	return l; // Left and right endpoints have collapsed.
}

/**
 * Use a generalized bisection method to find a zero of this
 * 1D function.  The sign of the function must differ at the
 * two endpoints. 
 *   @param btw Chooses test locations, given the previous bounds.
 *   @param f The function whose zero we seek.
 *   @param l The leftmost boundary of the search interval.
 *   @param r The rightmost boundary of the search interval.
 *   @param tol If fabs(f(x))<tol, x is considered a zero.
 * For example, zero(secant, fn, 0, 1) returns the 
 * zero of fn on the interval [0,1].
 */
template <class BETWEEN,class FN,class real>
inline real zero(BETWEEN btw,FN f,real l,real r,real tol=1.0e-7) {
	return zero(btw,f,l,r,f(l),f(r),tol);
}

/// A BETWEEN function that just bisects the interval.
///  This should be templated, but this seems to crash old gcc's.
inline double bisect(double l,double r,double fl,double fr) 
	{ return 0.5*(l+r); }

/// A BETWEEN function that uses the secant (linear approximation) method.
inline double secant(double l,double r,double fl,double fr) 
{ 
	if (fr-fl == (double)0) return bisect(l,r,fl,fr);
	return l - fl*(r-l)/(fr-fl);
}

};

#endif
