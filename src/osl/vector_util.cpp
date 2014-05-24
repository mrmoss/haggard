/*
Orion's Standard Library
Orion Sky Lawlor, 11/3/1999
NAME:		Vector3d.cpp

DESCRIPTION:	C++ 3-Dimentional vector library (no templates)

This file provides various utility routines for easily
manipulating 3-D vectors-- included are arithmetic,
dot/cross product, magnitude and normalization terms. 
Almost all routines are provided right in the header file (for inlining).

*/
#include "osl/vector2d.h"
#include "osl/vector3d.h"
using namespace osl;

/****************** Vector3d ****************/
//Return the polar coordinates of this vector
Polar3d::Polar3d(const Vector3d &v)
{
	theta=atan2(v.y,v.x);
	r=v.mag();
	phi=asin(v.z/r);
}

/***************** LineSegment *****************/
double LineSeg::param(const Vector2d &p) const
{
	if (d.x*d.x>d.y*d.y) //Line mostly horizontal-- use x to find t
		return (p.x-s.x)/d.x;
	else //Line vertical-- use y to find t
		return (p.y-s.y)/d.y;
}

bool LineSeg::intersection(const LineSeg &l2,double &t1,double &t2) const
{
	double det=l2.d.x*d.y-l2.d.y*d.x;
	if (det==0) return false; //Lines never intersect
	Vector2d od=l2.s-s;//Distance between start Points
	double scale=1.0/det;
	t1=(l2.d.x*od.y-l2.d.y*od.x)*scale;
	t2=(   d.x*od.y-   d.y*od.x)*scale;
	return true;
}

//Return true and set the appropriate t values if these lines intersect.
// withEndPts determines whether endpoint-only intersections are accepted.
bool LineSeg::intersects(const LineSeg &l2,bool withEndPts,double *t1Val,double *t2Val) const
{
	double t1,t2;
	if (!intersection(l2,t1,t2))
	{ //Parallel lines: assume they never intersect.
		t1=-100; t2=-100; 
	}
	if (t1Val) {*t1Val=t1;if (t2Val) *t2Val=t2;}
	if (withEndPts) {//End Points count
	  if ((t1<0)||(t1>1)||(t2<0)||(t2>1)) return false;
	} else {//End Points don't count
	  if ((t1<OSL_LINESEG_EPSILON)||(t1>OSL_LINESEG_EPSILON_M1) //Misses line 1
	    ||(t2<OSL_LINESEG_EPSILON)||(t2>OSL_LINESEG_EPSILON_M1)) //Misses line 2
		return false;
	}
	//If we got this far, it's a hit!
	return true;
}
