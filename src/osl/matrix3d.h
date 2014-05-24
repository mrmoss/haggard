/*
Orion's Standard Library
Orion Sky Lawlor, 9/23/1999

Provides routines for creating, reading in,
adding, multiplying, and inverting 3d
rotation/translation/scale/skew matrices.
*/
#ifndef __OSL_MATRIX3D_H
#define __OSL_MATRIX3D_H

#ifndef __OSL_MATRIXT_H
#include "osl/matrixT.h"
#endif
#ifndef __OSL_VECTOR3D_H
#include "osl/vector3d.h"
#endif
#ifndef __OSL_VECTOR4D_H
#include "osl/vector4d.h"
#endif

namespace osl {

typedef float Matrix3d_real;
class Matrix3d : public MatrixT<Matrix3d_real,4,4> {
	typedef MatrixT<Matrix3d_real,4,4> super;
public:
	typedef Matrix3d_real flt;
	Matrix3d() {}//Builds 4x4 identity Matrix)
	
	Matrix3d(flt s) {identity(s);}	
	
	void identity(flt s=1.0);


	/// Create Matrix with given column vectors
	Matrix3d(const Vector3d &x,const Vector3d &y,
		const Vector3d &z,const Vector3d &origin) {
		setX(x);setY(y);setZ(z);setO(origin);
		data[3][0]=data[3][1]=data[3][2]=(flt)0.0;data[3][3]=(flt)1.0;
	}
	/// Create Matrix with given values.  
	///   Entries listed in usual matrix order, i.e.,
	///   x0,y0,z0,w0 form the first row of the matrix.
	Matrix3d(
		flt x0,flt y0,flt z0,flt w0,
		flt x1,flt y1,flt z1,flt w1,
		flt x2,flt y2,flt z2,flt w2,
		flt x3,flt y3,flt z3,flt w3
	) {
		data[0][0]=x0; data[0][1]=y0; data[0][2]=z0; data[0][3]=w0;
		data[1][0]=x1; data[1][1]=y1; data[1][2]=z1; data[1][3]=w1;
		data[2][0]=x2; data[2][1]=y2; data[2][2]=z2; data[2][3]=w2;
		data[3][0]=x3; data[3][1]=y3; data[3][2]=z3; data[3][3]=w3;
	}
	
	void setCol(const Vector3d &a,int colNo) 
		{data[0][colNo]=(flt)a.x;data[1][colNo]=(flt)a.y;data[2][colNo]=(flt)a.z;}
	Vector3d getCol(int colNo) const
		{return Vector3d(data[0][colNo],data[1][colNo],data[2][colNo]);}		
	Vector3d getX(void) const {return getCol(0);}
	Vector3d getY(void) const {return getCol(1);}
	Vector3d getZ(void) const {return getCol(2);}
	Vector3d getO(void) const {return getCol(3);}
	
	void setX(const Vector3d &a) {setCol(a,0);}
	void setY(const Vector3d &a) {setCol(a,1);}
	void setZ(const Vector3d &a) {setCol(a,2);}
	void setO(const Vector3d &a) {setCol(a,3);}	
	
	//Make this Matrix a rotation Matrix, right-handed about +x axis
	// (*must* have just called Matrix3d() constructor)
	void rotateX(double radians);
	void rotateY(double radians);
	void rotateZ(double radians);
	
	//Shift this Matrix' origin by the given distance
	void translate(const Vector3d &off)
	  {data[0][3]+=(flt)off.x;data[1][3]+=(flt)off.y;data[2][3]+=(flt)off.z;}
	//Scale this Matrix up by the specified factors
	void scale(const Vector3d &fac);
	
	//Apply this Matrix to the given vector 
	Vector3d apply(const Vector3d &in) const;
	//Apply this Matrix to the given direction vector (no offset added)
	Vector3d applyDirection(const Vector3d &in) const;
	//Apply this Matrix to the given homogenous vector
	Vector4d applyHomogenous(const Vector4d &in) const;
	
	//Inline versions of the above calls
	Vector3d applyInline(const Vector3d &in) const {
	   Vector3d dest;
	   dest.x=data[0][0]*in.x+data[0][1]*in.y+data[0][2]*in.z+data[0][3];
	   dest.y=data[1][0]*in.x+data[1][1]*in.y+data[1][2]*in.z+data[1][3];
	   dest.z=data[2][0]*in.x+data[2][1]*in.y+data[2][2]*in.z+data[2][3];
	   return dest;
	}
	Vector3d applyDirectionInline(const Vector3d &in) const {
	   Vector3d dest;
	   dest.x=data[0][0]*in.x+data[0][1]*in.y+data[0][2]*in.z;
	   dest.y=data[1][0]*in.x+data[1][1]*in.y+data[1][2]*in.z;
	   dest.z=data[2][0]*in.x+data[2][1]*in.y+data[2][2]*in.z;
	   return dest;
	}
	
	/// Map this halfspace by the inverse of this matrix.
	///     return.side(x) = h.side(this->apply(x));
	/// or  return.side(this->inverse().apply(x)) = h.side(x);
	Halfspace3d applyInverse(const Halfspace3d &h) const {
		return Halfspace3d(Vector3d(
			data[0][0]*h.n.x+data[1][0]*h.n.y+data[2][0]*h.n.z,
			data[0][1]*h.n.x+data[1][1]*h.n.y+data[2][1]*h.n.z,
			data[0][2]*h.n.x+data[1][2]*h.n.y+data[2][2]*h.n.z),
			data[0][3]*h.n.x+data[1][3]*h.n.y+data[2][3]*h.n.z+h.d);
	}
	
	Matrix3d inverse(void) const {Matrix3d ret; invert(ret); return ret;}
	Matrix3d transpose(void) const {Matrix3d ret; super::transpose(ret); return ret;}
};

inline Matrix3d operator+(const Matrix3d &a,const Matrix3d &b)
{
	Matrix3d ret=a; ret.add(b);
	return ret;
}
inline Matrix3d operator*(const Matrix3d &a,const Matrix3d &b)
{
	Matrix3d ret;
	a.product(b,ret);
	return ret;
}
inline Vector4d operator*(const Matrix3d &a,const Vector4d &b) 
	{ return a.applyHomogenous(b); }

}; //end namespace osl
#endif
