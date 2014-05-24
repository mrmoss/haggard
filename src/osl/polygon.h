/*
Orion's Standard Library
Orion Sky Lawlor, 9/1/2002
NAME:		osl/polygon.h

DESCRIPTION:	C++ 2D polygon class
*/
#ifndef __OSL_POLYGON_H
#define __OSL_POLYGON_H

#ifndef __OSL_VECTOR2D_H
#include "osl/vector2d.h"
#endif

namespace osl { 

//Second-order 2D Moments (and products) of inertia 
// (does NOT belong here...)
class Moments2d {
public:
	double I; //Integral of 1 == Area of shape
	double Ix,Iy; //Integral of x, y
	//These last three are assumed evaluated from the center of mass
	double Ixx, Ixy, Iyy; //Integral of x^2, x y, y^2
	
	Moments2d() {I=Ix=Iy=Ixx=Ixy=Iyy=0;}
	Moments2d(double I_,double x,double y,double xx,double xy,double yy)
		:I(I_), Ix(x), Iy(y), Ixx(xx), Ixy(xy), Iyy(yy) {}
	
	inline double getArea(void) const {return I;}
	inline Vector2d getCOM(void) const {return Vector2d(Ix,Iy)*(1.0/getArea());}
	Vector2d getPrincipleAxis(void) const;
};

//Describes a 2D polygon
class Polygon {
	int n;
	const Vector2d *pts;
protected:
	inline void setPoints(int n_,const Vector2d *pts_) {
		n=n_; pts=pts_;
	}
public:
	Polygon() {setPoints(0,0);}
	Polygon(int n_,const Vector2d *pts_) {setPoints(n_,pts_);}
	
	inline int size(void) const {return n;}
	
	//In-bounds accessor
	inline const Vector2d &operator[](int i) const {return pts[i];}
	
	//Out-of-bounds accessor:
	inline const Vector2d &getWrap(int i) const {
		while (i<0) i+=n;
		while (i>=n) i-=n;
		return pts[i];
	}
	
	//Compute our (signed) area.  Clockwise winding -> positive
	double area(void) const;
	
	//Compute the center of mass (assuming constant density)
	Vector2d centerOfMass(void) const;
	
	//Compute the moments of inertia of this shape:
	Moments2d getMoments(void) const;
};

//A dynamically allocated polygon
class AllocPolygon : public Polygon {
	enum {smallSize=10}; //Polygons smaller than this are statically allocated
	Vector2d small[smallSize]; //If (n<=smallSize), the points.
	Vector2d *dyn; //If not NULL, the heap-allocated points.
	Vector2d *pts; //The points
public:
	AllocPolygon(int n) {
		if (n<=smallSize) {
			pts=small; dyn=0;
		}
		else {
			pts=dyn=new Vector2d[n];
		}
		setPoints(n,pts);
	}
	~AllocPolygon() {if (dyn) delete[] dyn;}
	
	inline Vector2d &operator[](int i) {return pts[i];}
	inline const Vector2d &operator[](int i) const {return pts[i];}
};



}; //end namespace osl

#endif //def(thisHeader)

