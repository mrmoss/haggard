/**
3D vector class, with overloaded operators and other handy functions.

Copied directly from osl/vector3d.h and osl/vec4.h and slightly tweaked.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-04 (Public Domain)
*/
#ifndef __CYBERALASKA__VEC3_H
#define __CYBERALASKA__VEC3_H

#include "../cyberalaska/pup.h"
#include <math.h> /* for sqrt */

namespace cyberalaska {

/// Vector3d is a cartesian vector in 3-space-- an x, y, and z.
///  For cross products, the space is assumed to be right-handed (x cross y = +z)
template <class real>
class Vector3dT {
	typedef Vector3dT<real> vec;
public:
	real x,y,z;
	Vector3dT(void) {}//Default consructor
	/// Simple 1-value constructors
	explicit Vector3dT(int init) {x=y=z=(real)init;}
	explicit Vector3dT(float init) {x=y=z=(real)init;}
	explicit Vector3dT(double init) {x=y=z=(real)init;}
	
	/// 3-value constructor
	Vector3dT(const real Nx,const real Ny,const real Nz) {x=Nx;y=Ny;z=Nz;}
	/// real array constructor
	Vector3dT(const real *arr) {x=arr[0];y=arr[1];z=arr[2];}

	/// Constructors from other types of Vector:
	Vector3dT(const Vector3dT<float> &src) 
	  {x=(real)src.x; y=(real)src.y; z=(real)src.z;}
	Vector3dT(const Vector3dT<double> &src) 
	  {x=(real)src.x; y=(real)src.y; z=(real)src.z;}
	Vector3dT(const Vector3dT<int> &src) 
	  {x=(real)src.x; y=(real)src.y; z=(real)src.z;}

	// Copy constructor & assignment operator are by default
	
	/// This lets you typecast a vector to a real array
	operator real *() {return (real *)&x;}
	operator const real *() const {return (const real *)&x;}

//Basic mathematical operators	
	int operator==(const vec &b) const {return (x==b.x)&&(y==b.y)&&(z==b.z);}
	int operator!=(const vec &b) const {return (x!=b.x)||(y!=b.y)||(z!=b.z);}
	vec operator+(const vec &b) const {return vec(x+b.x,y+b.y,z+b.z);}
	vec operator-(const vec &b) const {return vec(x-b.x,y-b.y,z-b.z);}
	vec operator*(const real scale) const 
		{return vec(x*scale,y*scale,z*scale);}
	vec operator/(const real &div) const
		{real scale=1.0/div;return vec(x*scale,y*scale,z*scale);}
	vec operator-(void) const {return vec(-x,-y,-z);}
	void operator+=(const vec &b) {x+=b.x;y+=b.y;z+=b.z;}
	void operator-=(const vec &b) {x-=b.x;y-=b.y;z-=b.z;}
	void operator*=(const real scale) {x*=scale;y*=scale;z*=scale;}
	void operator/=(const real div) {real scale=1.0/div;x*=scale;y*=scale;z*=scale;}

//Vector-specific operations
	/// Return the square of the magnitude of this vector
	real magSqr(void) const {return x*x+y*y+z*z;}
	/// Return the magnitude (length) of this vector
	real mag(void) const {return sqrt(magSqr());}
	
	/// Return the square of the distance to the vector b
	real distSqr(const vec &b) const 
		{return (x-b.x)*(x-b.x)+(y-b.y)*(y-b.y)+(z-b.z)*(z-b.z);}
	/// Return the distance to the vector b
	real dist(const vec &b) const {return sqrt(distSqr(b));}
	
	/// Return the dot product of this vector and b
	real dot(const vec &b) const {return x*b.x+y*b.y+z*b.z;}
	/// Return the cosine of the angle between this vector and b
	real cosAng(const vec &b) const {return dot(b)/(mag()*b.mag());}
	
	/// Return the "direction" (unit vector) of this vector
	vec dir(void) const {return (*this)/mag();}
	/// Return the right-handed cross product of this vector and b
	vec cross(const vec &b) const {
		return vec(y*b.z-z*b.y,z*b.x-x*b.z,x*b.y-y*b.x);
	}
	/// Make this vector have unit length
	void normalize(void) { *this=this->dir();}
	
	/// Return the largest coordinate in this vector
	real max(void) const {
		real big=x;
		if (big<y) big=y;
		if (big<z) big=z;
		return big;
	}
	/// Make each of this vector's coordinates at least as big
	///  as the given vector's coordinates.
	void enlarge(const vec &by) {
		if (x<by.x) x=by.x;
		if (y<by.y) y=by.y;
		if (z<by.z) z=by.z;     
	}
	
	template <class PUP> 
	void pup(PUP &p) {
		pup(p,x,"x");
		pup(p,y,"y");
		pup(p,z,"z");
	}
};

/** Utility wrapper routines */
template<class real>
inline real dist(const Vector3dT<real> &a,const Vector3dT<real> &b)
	{ return a.dist(b); }

template<class real>
inline real dot(const Vector3dT<real> &a,const Vector3dT<real> &b)
	{ return a.dot(b); }

template<class real>
inline Vector3dT<real> cross(const Vector3dT<real> &a,const Vector3dT<real> &b)
	{ return a.cross(b); }

/// Allow "3.0*vec" to compile:
template <class scalar_t,class real>
inline Vector3dT<real> operator*(const scalar_t scale,const Vector3dT<real> &v)
		{return Vector3dT<real>(v.x*scale,v.y*scale,v.z*scale);}


typedef Vector3dT<float> vec3;

inline vec3 normalize(const vec3 &v) {return v.dir();}
inline float dot(const vec3 &a,const vec3 &b) {return a.dot(b);}
inline float length(const vec3 &a) {return a.mag();}
inline vec3 reflect(const vec3 &I,const vec3 &N) 
	{return I-2.0*dot(N,I)*N;}

/// Allow "vec*vec" to compile:
inline vec3 operator*(const vec3 &a,const vec3 &b) {return vec3(a.x*b.x,a.y*b.y,a.z*b.z);}

inline vec3 mix(const vec3 &a,const vec3 &b,float f) {return a+f*(b-a);}
inline vec3 min(const vec3 &a,const vec3 &b) {
	vec3 ret=a;
	for (int axis=0;axis<3;axis++) {
		if (ret[axis]>b[axis]) ret[axis]=b[axis];
	}
	return ret;
}
inline vec3 max(const vec3 &a,const vec3 &b) {
	vec3 ret=a;
	for (int axis=0;axis<3;axis++) {
		if (ret[axis]<b[axis]) ret[axis]=b[axis];
	}
	return ret;
}


/** Orthonormal coordinate frame identifies the three axes of a coordinate system.
  It's actually a fairly handy way to do coordinate transformations.
*/
class ortho_frame {
public:
	/** Unit vectors pointing along axes of our frame.
		X cross Y is Z. (right-handed coordinate system)
	*/
	vec3 x,y,z;
	ortho_frame() :x(1,0,0), y(0,1,0), z(0,0,1) {}
	
	/** Reorient this coordinate frame by this far along in the X and Y axes.  
	   "dx" is the distance along the z axis to push the x axis; 
	   "dy" the distance along the z axis to push the y axis.
	*/
	void nudge(double dx,double dy) {
		x+=dx*z;
		y+=dy*z;
		orthonormalize();
	}
	
	/** Project a global-coordinates point into this local coordinate frame */
	vec3 project_in(const vec3 &G) {
		return vec3(dot(G,x),dot(G,y),dot(G,z));
	}
	
	/** Project a local-coordinates point back out into global coordinates */
	vec3 project_out(const vec3 &L) {
		return L.x*x+L.y*y+L.z*z;
	}
	
	/** Reconstruct an orthonormal frame from modified X and Y axes.
	  Z is primary (will not change), X is secondary (follows Z), Y is tertiary.
	*/
	void orthonormalize(void) {
		z=normalize(z);
		y=normalize(cross(z,x));
		x=normalize(cross(y,z));
	}
	
	template <class PUP> 
	void pup(PUP &p) {
		pup(p,x,"x");
		pup(p,y,"y");
		pup(p,z,"z");
	}
};




}; /* end namespace */

#endif /* end include guard */
