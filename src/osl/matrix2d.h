/*
Orion's Standard Library
Orion Sky Lawlor, 9/23/1999

Provides routines for creating, reading in,
adding, multiplying, and inverting 2d
rotation/translation/scale/skew matrices.
*/
#ifndef __OSL_MATRIX2D_H
#define __OSL_MATRIX2D_H

#ifndef __OSL_MATRIXT_H
#include "osl/matrixT.h"
#endif
#ifndef __OSL_VECTOR2D_H
#include "osl/vector2d.h"
#endif


namespace osl {

typedef float Matrix2d_real;

class Matrix2d : public MatrixT<Matrix2d_real,3,3> {
public:
	typedef Matrix2d_real flt;
	Matrix2d() {} //Warning--uninitialized!
	Matrix2d(flt s) {identity(s);}
	
	//Create Matrix with given column vectors
	Matrix2d(const Vector2d &x,const Vector2d &y,const Vector2d &origin) {
		setX(x);setY(y);setO(origin);
		data[2][0]=data[2][1]=(flt)0.0; data[2][2]=(flt)1.0;
	}
	
	//Set the given columns of the Matrix
	void setCol(const Vector2d &a,int colNo) 
		{data[0][colNo]=(flt)a.x;data[1][colNo]=(flt)a.y;}
	Vector2d getCol(int colNo) const
		{return Vector2d(data[0][colNo],data[1][colNo]);}
	void setX(const Vector2d &a) {setCol(a,0);}
	void setY(const Vector2d &a) {setCol(a,1);}
	void setO(const Vector2d &a) {setCol(a,2);}	
	
	//Make this Matrix a rotation Matrix, counterclockwise about zero
	// (*must* have just called Matrix2d() constructor)
	void rotate(double radians) {
		flt s=(flt)sin(radians),c=(flt)cos(radians);
		data[0][0]=c;data[0][1]=-s;
		data[1][0]=s;data[1][1]=c;
	}
	
	//Shift this Matrix' origin by the given distance
	void translate(const Vector2d &off)
	  {data[0][2]+=(flt)off.x;data[1][2]+=(flt)off.y;}
	//Scale this Matrix up by the specified factors
	void scale(const Vector2d &fac)
	  {data[0][0]*=(flt)fac.x;data[0][1]*=(flt)fac.y;
	   data[1][0]*=(flt)fac.x;data[1][1]*=(flt)fac.y;}
	
	//Apply this Matrix to the given vector 
	Vector2d apply(const Vector2d &in) const;
	
	//Apply this Matrix to the given direction vector (no offset added)
	Vector2d applyDirection(const Vector2d &in) const;
	
	//Inline versions of the above (for speed)
	Vector2d applyInline(const Vector2d &in) const {
	   Vector2d dest;
	   dest.x=data[0][0]*in.x+data[0][1]*in.y+data[0][2];
	   dest.y=data[1][0]*in.x+data[1][1]*in.y+data[1][2];
	   return dest;
	}
	Vector2d applyDirectionInline(const Vector2d &in) const {
	   Vector2d dest;
	   dest.x=data[0][0]*in.x+data[0][1]*in.y;
	   dest.y=data[1][0]*in.x+data[1][1]*in.y;
	   return dest;
	}
};


inline Matrix2d product(const Matrix2d &a,const Matrix2d &b) {
	Matrix2d ret;
	a.product(b,ret);
	return ret;
}
inline Matrix2d inverse(const Matrix2d &a) {
	Matrix2d ret;
	a.invert(ret);
	return ret;
}

//Apply this Matrix to the given vector 
inline Vector2d Matrix2d::apply(const Vector2d &in) const {
   Vector2d dest;
   dest.x=data[0][0]*in.x+data[0][1]*in.y+data[0][2];
   dest.y=data[1][0]*in.x+data[1][1]*in.y+data[1][2];
   return dest;
}
//Apply this Matrix to the given direction vector (no offset added)
inline Vector2d Matrix2d::applyDirection(const Vector2d &in) const {
   Vector2d dest;
   dest.x=data[0][0]*in.x+data[0][1]*in.y;
   dest.y=data[1][0]*in.x+data[1][1]*in.y;
   return dest;
}

}; //end namespace osl
#endif

