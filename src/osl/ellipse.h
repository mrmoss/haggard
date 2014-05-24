/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 7/9/2000

Ellipse-handling routines.  Made extremely fast for inner loops
by exposing all sorts of weird math.  You probably only want to 
deal with EllipseProperties and nothing else.
*/

#ifndef __OSL_ELLIPSE_H
#define __OSL_ELLIPSE_H

#include "osl/raster.h"
#include "osl/color.h"
#include "osl/vector2d.h"
#include "osl/polygon.h"

namespace osl { namespace graphics3d {

using osl::Vector2d;
using osl::Moments2d;

//The essential properties of a 2D Ellipse
class EllipseProperties {
public:
	Vector2d center;//Center of mass of Ellipse
	double major;//Radius along larger axis (must be positive)
	double minor;//Radius along smaller axis (must be positive)
	double angle;//Angle in radians CCW from +X of major axis (-pi/2,pi/2]
	
	//Estimate the Ellipse bounded by these Points
	EllipseProperties(const Vector2d *pts,int nPts);
	
	//Estimate the Ellipse that has these moments of inertia:
	EllipseProperties(const Moments2d &m);
	
	//Make an ellipse with these properties
	EllipseProperties(const Vector2d &center_,double major_,double minor_,double angle_)
		:center(center_), angle(angle_) 
	{ //Can't use initializer syntax for major(major_) because it's a macro (!)
	  // under Linux (defined in /usr/include/sys/sysmacros.h)
		major=major_; minor=minor_;
	}
	EllipseProperties(void) {}
	
	Vector2d majorAxis(void) {return Polar2d(angle,major);}
	Vector2d minorAxis(void) {return Vector2d(Polar2d(angle,major)).perp();}
	
	/*
	Render in the given color into the given Raster.
	blurFactor determines the degree of antialiasing needed.
	This routine performs any clipping needed-- it can be used with 
	arbitrary ellipses.
	*/
	void render(const osl::graphics2d::Color &c, 
		osl::graphics2d::Raster &dest, double blurFactor=1.0) const;
};

//A slow way to render a 2D Ellipse
class Ellipse {
public:
	Vector2d center;//Center of mass of Ellipse
	double c_xx,c_xy,c_yy;//Coefficients for getting radius
	Ellipse(const EllipseProperties &p);
	
	//Return the scaled and rotated radial distance of 
	// this Point from the Ellipse centre. (Ellipse altitude)
	double radSqr(const Vector2d &v) const {
		Vector2d d=v-center;
		return d.x*d.x*c_xx+d.x*d.y*c_xy+d.y*d.y*c_yy;
	}
};


/*A simple quadratic weighting, used for antialiasing ellipse boundary:
     /     0    r>h, h=sqrt(1+delta-delta^2/4)
w(r)=|     1    r<l, l=1-delta/2
     \  c-a*r^2   else; c and a chosen for continuity and w(1.0)=0.5
*/
class QuadWeight {
public:
	double h2,l2;//Upper and lower limits for quadratic
	double c,a;//Constant and quadratic terms: w(r2)=c-a*r2
	
	QuadWeight(double delta)
	{
		if (delta<=2.0)
		{//Small delta
			a=1.0/(2*delta-0.5*delta*delta);
			c=a+0.5;
			h2=1+(delta-0.25*delta*delta);
			l2=1-(delta-0.25*delta*delta);
		} else {//Large delta
			c=1-0.5*(delta-2);
			a=c-0.5;
			h2=c/a;
			l2=(c-1)/a;
		}
	}
	
	double weight2(double r2) {
		if (r2>h2) return 0.0;
		if (r2<l2) return 1.0;
		else return c-a*r2;
	}
};

//Quick Rasterization utilities for Ellipse
class EllipseRaster {
public:
	Vector2d center;//Center of mass of Ellipse
	double yConst,yLin,yQuad;//Coefficients for finding x-bounds

	EllipseRaster(const Ellipse &e,double r2max);
	
	//Return tight upper and lower bounds for r2<=r2max
	void yExtents(double &yMin,double &yMax)
	{//Y bounds come from determinant of x equation
		double yDel=sqrt(-yConst/yQuad);
		yMin=center.y-yDel;
		yMax=center.y+yDel;
	}
	//Return tight left and right bounds for r2<=r2max at the given y
	void xSpan(double y,double &xMin,double &xMax) const
	{
		y-=center.y;
		double xDel=sqrt(y*y*yQuad+yConst);
		double xCen=center.x-y*yLin;
		xMin=xCen-xDel;
		xMax=xCen+xDel;
	}
	//Return coefficients to turn x into weight
	void x2weightCoeffs(double y,const Ellipse &e,const QuadWeight &q,
		double &xQuad,double &xLin,double &xConst) const
	{
		y-=center.y;
		xQuad=-q.a*e.c_xx;
		xLin=-q.a*e.c_xy*y;
		xConst=q.c-q.a*e.c_yy*y*y;
		
		xConst+= -e.center.x*xLin+e.center.x*e.center.x*xQuad;
		xLin  += -2*e.center.x*xQuad;
	}
};


}; }; //End namespace

#endif

