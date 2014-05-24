/*
Orion Sky Lawlor, olawlor@acm.org, 2004/9/29

Utilities for integrating shifted cosine lobes 
against (low-order) spherical harmonic bases.
*/
#ifndef __OSL_SH_UTIL_H
#define __OSL_SH_UTIL_H

#include "vector3d.h"

namespace sh_cosine {

/**
 A cosine lobe sphere function centered on the Z axis.
 Formally defined on a unit direction D as:
    f_alpha(D) = max(0,(1-alpha) + alpha*D.z)
 or (equivalently)
    f(D) = max(0,1 + alpha*(D.z-1))
 or (again equivalently) taking theta = angle between D and Z axis,
    f(theta) = max(0,(1-alpha) + alpha*cos(theta))
 
 This reduces to a variety of commonly-used functions
 in computer graphics, including Lambertian shading,
 delta functions, and uniform illumination.
*/
class cosLobe {
public:
	/**
	  Primary shape parameter for lobe.
	  
	  If alpha is 0, the function is uniformly 1.0 in all directions.
	  If alpha is 1, the function is a Lambertian cosine lobe.
	  If alpha is infinity, the function is a delta.
	  
	  The function is never negative if alpha <= 0.5.
	*/
	double alpha;
	
	inline cosLobe() {alpha=1;}
	inline cosLobe(double a) :alpha(a) {}
	
	/// Evaluate the function at a unit vector with this Z component.
	///  Dz can also be interpreted as the cosine of the angle between
	///  the unit vector and the Z axis.
	inline double eval(double Dz) const {
		double ret=(1-alpha)+alpha*Dz;
		if (ret<0) return 0;
		else return ret;
	}
	
	/// Evaluate the function at this unit vector direction.
	inline double eval(const osl::Vector3d &D) const {
		return eval(D.z);
	}
};

/**
  Return the integral, as a fraction of the unit sphere, 
  of the function f. 
  
  If you want the integral in stradians, 
  multiply the return value by 4 pi.
*/
inline double unitIntegral(const cosLobe &f) {
	if (f.alpha>0.5) /* hits zero-- modify limits of integration */
		return 0.25/f.alpha;
	else /* f.alpha<=0.5, f nonzero over whole hemisphere */
		return (1-f.alpha);
}

/**
  Return the integral, in radians, of f with 
  the zero'th spherical harmonic basis.
*/
inline double y0_integral(const cosLobe &f) {
	const double y0_scale=4*M_PI/sqrt(4*M_PI);
	return y0_scale*unitIntegral(f);
}

/**
  Return the scale factor for the integral, as a fraction 
  of the unit sphere, of the product of the function f 
  and the function s defined as:
      s(D) = D dot V;  
  
  The value of the integral is the return value * V.z.
  If you want the integral in stradians, multiply the 
  return value by V.z 4 pi.
*/
inline double linearIntegral(const cosLobe &f) {
	const double oneThird=1.0/3.0;
	if (f.alpha>0.5) { /* hits zero-- modify limits of integration */
		return 0.25*(f.alpha-oneThird)/(f.alpha*f.alpha);
	}
	else /* f.alpha<=0.5, f nonzero over whole hemisphere */
		return oneThird*f.alpha;
}

/**
  Return the integral, in radians, of f with 
  the first spherical harmonic basis,
  DIVIDED by the dot product of the direction of
  the lobe and the direction of the basis.
*/
inline double y1_integral(const cosLobe &f) {
	const double y1_scale=4*M_PI*sqrt(3/(4*M_PI));
	return y1_scale*linearIntegral(f);
}

};

#endif
