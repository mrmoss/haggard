/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 6/13/2001

Calculate areas and moments of a polygon.
*/
#include <stdio.h>
#include <string.h>
#include "osl/math_header.h"
#include "osl/polygon.h"
#include "osl/integrate.h"

using namespace osl;


//A basicIntegral_t, used in implementation of Polygon::area();
inline double bi_1(double x,double m,double b) //Integrate 1 over trapezoid: 
{ //Integral[x, (m x+b) ] == m x^2/2+b x
	const static double oneHalf=1.0/2.0;
	return (oneHalf*m*x+b)*x;
}

//Compute area of polygon
double Polygon::area(void) const
{
	return ::osl::integrate::integrateDouble(*this,bi_1);
}


//Compute the center of mass (assuming constant density)
Vector2d Polygon::centerOfMass(void) const
{
	::osl::integrate::centerOfMass_t com; 
	::osl::integrate::integrateAccum(*this,com);
	return com.getCOM();
}

//Add the moments of inertia of this shape:
Moments2d Polygon::getMoments(void) const
{
	::osl::integrate::centerOfMass_t com;
	::osl::integrate::integrateAccum(*this,com);
	Vector2d c(com.getCOM());
	
	AllocPolygon p(size());
	for (int i=0;i<size();i++) p[i]=pts[i]-c;
	::osl::integrate::momentsOfInertia_t mi;
	::osl::integrate::integrateAccum(p,mi);
	
	return Moments2d(com.getArea(),com.getIx(),com.getIy(),
		mi.getIxx(),mi.getIxy(),mi.getIyy());
}

Vector2d Moments2d::getPrincipleAxis(void) const
{
/* Define the new axes:
	x'=r.x*x+r.y*y
	y'=-r.y*x+r.x*y

We want to pick r so x',y' are the principle axes-- i.e., so Ix'y'==0.
The definition of Ixy gives us:
  Ix'y'=I(r.x*x+r.y*y)(-r.y*x+r.x*y)=I(-r.x r.y x^2+(r.x^2-r.y^2) xy+r.x r.y y^2)
       =r.x r.y (Iyy-Ixx) + (r.x^2-r.y^2) Ixy

Since Ix'y' will be zero for any multiple of r, we're free to pick r.y==1.0.
*/
	Vector2d r(0,1.0);
/*  
Then
  Ix'y'=r.x (Iyy-Ixx)+(r.x^2-1) Ixy
This means r.x satisfies the quadratic (a x^2+b x+c) with:
*/
	double a=Ixy, b=Iyy-Ixx, c=-Ixy;
	if (a==0) {//Already aligned with a principle axis 
		r.x=0;
	} else {
		double det=b*b-4*a*c;
		/*det can't be negative, since it's (Iyy-Ixx)^2+4*Ixy^2 */
		/*The two quadratic solutions are 90deg apart-- pick the larger one*/
		r.x=(-b+sqrt(det))/(2*a);
	}
	return r.dir();
}
