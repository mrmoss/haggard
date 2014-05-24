/*  
Spherical trigonometry routines.

Orion Sky Lawlor, olawlor@acm.org, 2004/7/6
*/
#ifndef __OSL_SPHERETRIG_H
#define __OSL_SPHERETRIG_H

#include "osl/math_header.h"

/**
Quick Review of Relevant Spherical Trigonometry:

On a sphere, we measure "straight line" distances
(along a geodesic, a great circle of the unit sphere) 
using an angle A.

Aa sphere-disk is the set of points less than some
angle, call it R, from the center.  The area
of a disk of radius R (an angle) is
	Disk(R) = 2 pi (1 - cos R)
this varies between 0 (R=0) and 4 pi (R=pi) steradians.

A sphere-disk-wedge is the fraction of a sphere-disk 
that makes an angle of W from the center, the area of 
a wedge is
	Wedge(R,W) = Disk(R) * W/(2 pi) = W (1-cos R)

A sphere-triangle is bounded by 3 points.  The area
of a triangle is given by the interior angles
(*not* the edge lengths).  This is Girard's Theorem:
	Triangle(a,b,c) = a + b + c - pi

Spherical Sine Theorem:
  Given a spherical triangle, the edge lengths A, B, C and
  corresponding angles a,b,c satisfy  
      sin a / sin A = sin b / sin B = sin c / sin C

Spherical Cosine Theorem:
  As for sine theorem, but 
     cos B = cos A cos C + sin A sin C cos b

Spherical Pythagorean Theorem: (special case of cosines)
  Given a right spherical triangle, edge lengths A, B, C (hypotenuse)
  satisfy    
       cos C = cos A cos B

*/
namespace spheretrig {

/**
  Return the area (in steradians) of a disk on a sphere.
    \param cosRadius cosine of the radius angle of the disk.
*/
inline double disk(double cosRadius) {
	return 2*M_PI*(1-cosRadius);
}

/**
  Return the area (in steradians) of a wedge of a disk on a sphere.
    \param cosRadius cosine of the radius angle of the disk.
    \param subRadians subtended angle about disk center.
*/
inline double wedge(double cosRadius,double subRadians) {
	return subRadians*(1-cosRadius);
}

/**
  Return the area (in steradians) of a triangle on a sphere.
    \param a angle (in radians) between the first two edges.
    \param b angle (in radians) between the next two edges.
    \param c angle (in radians) between the remaining two edges.
*/
inline double triangle(double a,double b,double c) {
	return a+b+c-M_PI;
}

/**
  Return the sin of this angle given the cosine.
*/
inline double sinFmCos(double cosAng) {
	//  Proof: 1=sinAng*sinAng+cosAng*cosAng
	return sqrt(1-cosAng*cosAng);
}
inline double cosFmSin(double sinAng) { /* ditto */ return sinFmCos(sinAng);}

/**
  Return the area (in steradians) of the portion
  of the disk A also covered by the disk B, where 
  the angle D is the distance between the disk centers.
  
We're interested in computing the area of the intersection
of two disks.  We compute this area by decomposing into 
the shapes above: one wedge plus the other wedge minus
the triangle in between them.
*/
double intersect(double cosA,double cosB,double cosD);

};

#endif
