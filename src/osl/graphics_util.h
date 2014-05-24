/*
Orion's Standard Library
Orion Sky Lawlor, 12/27/2002
NAME:		osl/graphics_util.h

DESCRIPTION:	C++ Graphics library utility classes

Little utilities for graphics tasks.
*/
#ifndef __OSL_GRAPHICS_UTIL_H
#define __OSL_GRAPHICS_UTIL_H

#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif
#ifndef __OSL_MATRIX2D_H
#  include "osl/matrix2d.h"
#endif

namespace osl { namespace graphics2d {

class Raster;

/**
 * A 2D, integer coordinates rectangle.  This is often used for
 * clipping.
 *
 * Pixels in the rectangle lie on [left,right-1] x [top,bottom-1]
 */
class Rect {
	static inline int min(int a,int b) {return (a<b)?a:b;}
	static inline int max(int a,int b) {return (a>b)?a:b;}	
public:
	int left,top,right,bottom;//Clip boundaries
	/// Build a rectangle with undefined initial coordinates
	Rect() { }
	/// Build a rectangle from the origin to this width and height.
	Rect(int wid,int ht) 
		:left(0),top(0),right(wid),bottom(ht) { }
	/// Build a rectangle with this (left,top) and (right,bottom) corners.
	/// Remember, the rightmost, bottommost pixel will be (right-1,bottom-1).
	Rect(int l,int t,int r,int b) 
		:left(l),top(t),right(r),bottom(b) { }
	/// Build a rectangle with this topleft and bottomright corners.
	/// Remember, the rightmost, bottommost pixel will be (br.x-1,br.y-1).
	Rect(const Point &tl,const Point &br) 
		:left(tl.x),top(tl.y),right(br.x),bottom(br.y) { }
	
	/// Build a rectangle the same size as this image, from the origin.
	Rect(const Raster &r); //Declared after Raster is defined
	
	/// Move this rectangle by the shift p
	Rect getShift(const Point &p) const {
		return Rect(left+p.x,top+p.y,right+p.x,bottom+p.y);
	}
	Rect operator+(const Point &p) const {return getShift(p);}

	Rect getIntersect(const Rect &o) const {
		return Rect(max(left,o.left),max(top,o.top),
			min(right,o.right),min(bottom,o.bottom));
	}
	Rect getInset(int shift) const {
		return Rect(left+shift,top+shift,right-shift,bottom-shift);
	}
	bool operator==(const Rect &b) const {
		return left==b.left && top==b.top && right==b.right && bottom==b.bottom;
	}
	
	Point getMin(void) const {return Point(left,top);}
	Point getMax(void) const {return Point(right,bottom);}
	
	bool isEmpty(void) const {
		return (right<=left)||(bottom<=top);
	}
	inline int getWidth(void) const {return right-left;}
	inline int getHeight(void) const {return bottom-top;}
	inline double getArea(void) const {
		return getWidth()*getHeight();
	}
	
//Clip utility functions
	/// Return true if this x value is out of bounds
	inline bool oobX(int x) const 
	  {return (x<left)||(x>=right);}
	/// Return true if this y value is out of bounds
	inline bool oobY(int y) const 
	  {return (y<top)||(y>=bottom);}
	
	/// Return true if the integer point (x,y) lies inside this rectangle.
	inline bool inbounds(int x,int y) const {
		if (oobX(x)) return false;
		if (oobY(y)) return false;
		return true;
	}
	/// Return true if this integer Point lies inside this rectangle.
	inline bool inbounds(const Point &p) const {
		return inbounds(p.x,p.y);
	}
	
	/// Return the nearest x that lies inside a 1-pixel enlarged rectangle.
	inline int clipX(int x) const
	  {if (x<left) return left; if (x>right) return right; return x;}
	inline int clipY(int y) const
	  {if (y<top) return top; if (y>bottom) return bottom; return y;}
};

typedef Rect Rectangle; //they're all the same to me

/** 
 * This macro lets you easily loop over the pixels in a rectangle.
 * This macro is used like
 *    osl::Rect r1=...;
 *    int x,y;
 *    OSL_RECT_LOOP(r1,x,y) foo(x,y).bar();
 *
 *  @param r The name of the rectangle variable to loop over.
 *           WARNING: because this is a macro, r will be evaluated
 *           over and over again in the loop.  DO NOT make r a function
 *           call, increment, etc.
 *  @param x The name of the integer x coordinate variable.
 *  @param y The name of the integer y coordinate variable.
 */
#define OSL_RECT_LOOP(r,x,y) \
	for (y=(r).top;y<(r).bottom;y++) \
	for (x=(r).left;x<(r).right;x++) 




/// Represents a cubic Bezier curve, with four control points
class Bezier {
	Vector2d A,B,C,D; //Control points
public:
	/// Build a bezier curve out of these points.
	/// The curve starts at A, has control points B and C, and ends at D.
	Bezier(const Vector2d &A_,const Vector2d &B_,
	       const Vector2d &C_,const Vector2d &D_) 
	        :A(A_), B(B_), C(C_), D(D_) {}
	
	/// Build a bezier to approximate the arc of a unit circle
	///  between these angles.
	Bezier(double startAng,double endAng);
	
	/// Apply a transformation matrix to this curve
	///  Works by just transforming each control point.
	void apply(const Matrix2d &m) {
		A=m.apply(A);
		B=m.apply(B);
		C=m.apply(C);
		D=m.apply(D);
	}
	
	const Vector2d &getStart(void) const {return A;}
	const Vector2d &getCtrl1(void) const {return B;}
	const Vector2d &getCtrl2(void) const {return C;}
	const Vector2d &getFinal(void) const {return D;}
	
	/// Return the location of the curve at this parameter value.
	///   u can vary between 0 (returns A) and 1 (returns D).
	Vector2d at(double u) const {
		double u2=u*u;
		double u3=u2*u;
		double mu=1-u;
		double mu2=mu*mu;
		double mu3=mu2*mu;
		return A*mu3+B*(3*u*mu2)+C*(3*u2*mu)+D*u3;
	}
	
	/// Return the tangent vector to the curve at this paramter value.
	/// The tangent is oriented in the direction of increasing u.
	Vector2d tangent(double u) const {
		double a=-3*(1-u)*(1-u);
		double b=(1-u)*(1-u)-2*u*(1-u);
		double c=2*u*(1-u)-u*u*u;
		double d=3*u*u;
		return A*a+B*(3*b)+C*(3*c)+D*d;
	}
};


}; }; /* end namespace */


#endif

