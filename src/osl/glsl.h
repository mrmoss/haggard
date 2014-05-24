/**
A software-only implementation of the OpenGL Shading Language
(GLSL), as implemented by the 
	GL_ARB_shading_language_100
OpenGL extension.

Why use GLSL in software?
  - Much better debuggers exist for C++ than for hardware, 
    so it's easier to debug code on the software side.
  - Fewer silly resource limits in software (e.g., maximum texture size,
     available shader precision).
  - More portable code--able to run on headless supercomputers
     and machines with ancient OpenGL drivers.
  - Compiled GLSL is much faster than the interpreted GLSL in drivers;
     even if it's still slower than real hardware.

What's missing:
  - Swizzles.  A big (512-entry) array of macros could do them.
  - "inout" and "out" parameters.  They're in the wrong place to 
    be #defined to "&".
  - Implicit constructor for user-defined structs.
  - The excellent dFdx, dFdy, and fwidth functions.  Some deep 
    template magic could probably handle these.

Orion Sky Lawlor, olawlor@acm.org, 2005/11/20 (Public Domain)
*/
#ifndef __SWGLSL_H
#define __SWGLSL_H

#include <math.h>
#include "matrixT.h" /* templated matrix type */
#include "vector2d.h"
#include "vector3d.h"
#include "vector4d.h"

namespace glsl {

/********** Basic Types ************
  "void", "bool", "int", and "float" are inherited from C++.
  "struct" is inherited from C++, although the implicit constructor isn't.
*/
typedef osl::Vector2d vec2;
typedef osl::Vector3d vec3;
typedef osl::Vector4d vec4;

/** * and / are defined component-wise for vectors */
inline vec2 operator*(const vec2 &a,const vec2 &b) {return vec2(a.x*b.x,a.y*b.y);}
inline vec3 operator*(const vec3 &a,const vec3 &b) {return vec3(a.x*b.x,a.y*b.y,a.z*b.z);}
inline vec4 operator*(const vec4 &a,const vec4 &b) {return vec4(a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w);}
inline vec2 operator/(const vec2 &a,const vec2 &b) {return vec2(a.x/b.x,a.y/b.y);}
inline vec3 operator/(const vec3 &a,const vec3 &b) {return vec3(a.x/b.x,a.y/b.y,a.z/b.z);}
inline vec4 operator/(const vec4 &a,const vec4 &b) {return vec4(a.x/b.x,a.y/b.y,a.z/b.z,a.w/b.w);}

// FIXME: mat2, mat3, mat4


/** map_to_scalar just calls "fn" on each component of the input. 
  It's used in the implementation of the zillion routines below.
*/
template <class fn_scalar>
inline float map_to_scalar(float src,fn_scalar fn) {
	return fn(src);
}
template <class fn_scalar>
inline vec2 map_to_scalar(const vec2 &src,fn_scalar fn) {
	return vec2(fn(src.x),fn(src.y));
}
template <class fn_scalar>
inline vec3 map_to_scalar(const vec3 &src,fn_scalar fn) {
	return vec3(fn(src.x),fn(src.y),fn(src.z));
}
template <class fn_scalar>
inline vec4 map_to_scalar(const vec4 &src,fn_scalar fn) {
	return vec4(fn(src.x),fn(src.y),fn(src.z),fn(src.w));
}
/* 2-argument versions */
template <class fn_scalar>
inline float map_to_scalar(float a,float b,fn_scalar fn) {
	return fn(a,b);
}
template <class fn_scalar>
inline vec2 map_to_scalar(const vec2 &a,const vec2 &b,fn_scalar fn) {
	return vec2(fn(a.x,b.x),fn(a.y,b.y));
}
template <class fn_scalar>
inline vec3 map_to_scalar(const vec3 &a,const vec3 &b,fn_scalar fn) {
	return vec2(fn(a.x,b.x),fn(a.y,b.y),fn(a.z,b.z));
}
template <class fn_scalar>
inline vec4 map_to_scalar(const vec4 &a,const vec4 &b,fn_scalar fn) {
	return vec2(fn(a.x,b.x),fn(a.y,b.y),fn(a.z,b.z),fn(a.w,b.w));
}
/* 3-argument versions */
template <class fn_scalar>
inline float map_to_scalar(float a,float b,float c,fn_scalar fn) {
	return fn(a,b,c);
}
template <class fn_scalar>
inline vec2 map_to_scalar(const vec2 &a,const vec2 &b,const vec2 &c,fn_scalar fn) {
	return vec2(fn(a.x,b.x,c.x),fn(a.y,b.y,c.y));
}
template <class fn_scalar>
inline vec3 map_to_scalar(const vec3 &a,const vec3 &b,const vec3 &c,fn_scalar fn) {
	return vec2(fn(a.x,b.x,c.x),fn(a.y,b.y,c.y),fn(a.z,b.z,c.z));
}
template <class fn_scalar>
inline vec4 map_to_scalar(const vec4 &a,const vec4 &b,const vec4 &c,fn_scalar fn) {
	return vec2(fn(a.x,b.x,c.x),fn(a.y,b.y,c.y),fn(a.z,b.z,c.z),fn(a.w,b.w,c.w));
}

/**
  This macro makes each call work on any vector type, by 
  defining a template that calls the above map_to_scalar routines.
*/
#define SOFTWARE_GLSL_VEC_1arg(name,expression) \
inline float name##_scalar(float a) {return expression;} \
template<class genType> inline genType name(const genType &a) {map_to_scalar(a,name##_scalar);}
#define SOFTWARE_GLSL_VEC_2arg(name,expression) \
inline float name##_scalar(float a,float b) {return expression;} \
template<class genType> inline genType name(const genType &a,const genType &b) {map_to_scalar(a,b,name##_scalar);}
#define SOFTWARE_GLSL_VEC_3arg(name,expression) \
inline float name##_scalar(float a,float b,float c) {return expression;} \
template<class genType> inline genType name(const genType &a,const genType &b,const genType &c) {map_to_scalar(a,b,c,name##_scalar);}

SOFTWARE_GLSL_VEC_1arg(radians,a*(M_PI/180));
SOFTWARE_GLSL_VEC_1arg(degrees,a*(180/M_PI));
SOFTWARE_GLSL_VEC_1arg(sin,sin(a));
SOFTWARE_GLSL_VEC_1arg(cos,cos(a));
SOFTWARE_GLSL_VEC_1arg(tan,tan(a));
SOFTWARE_GLSL_VEC_1arg(asin,asin(a));
SOFTWARE_GLSL_VEC_1arg(acos,acos(a));
SOFTWARE_GLSL_VEC_2arg(atan,atan2(a,b));
SOFTWARE_GLSL_VEC_1arg(atan,atan2(a,1.0f));
SOFTWARE_GLSL_VEC_2arg(pow,pow(a,b));
SOFTWARE_GLSL_VEC_1arg(exp,exp(a));
SOFTWARE_GLSL_VEC_1arg(log,log(a));
SOFTWARE_GLSL_VEC_1arg(exp2,exp2(a));
SOFTWARE_GLSL_VEC_1arg(log2,log2(a));
SOFTWARE_GLSL_VEC_1arg(sqrt,sqrt(a));
SOFTWARE_GLSL_VEC_1arg(inversesqrt,1.0f/sqrt(a));
SOFTWARE_GLSL_VEC_1arg(abs,fabs(a));
SOFTWARE_GLSL_VEC_1arg(sign,(a>0.0f)?1.0f:((a<0.0f)?-1.0f:0.0f));
SOFTWARE_GLSL_VEC_1arg(floor,floor(a));
SOFTWARE_GLSL_VEC_1arg(ceil,ceil(a));
SOFTWARE_GLSL_VEC_1arg(fract,a-floor(a));
SOFTWARE_GLSL_VEC_2arg(mod,fmod(a,b)); // float flavors come for free
SOFTWARE_GLSL_VEC_2arg(min,(a<b)?a:b);
SOFTWARE_GLSL_VEC_2arg(max,(a>b)?a:b);
SOFTWARE_GLSL_VEC_3arg(clamp,(a>c)?c:((a<b)?b:a));
SOFTWARE_GLSL_VEC_3arg(mix,a*(1.0-c)+b*c);
SOFTWARE_GLSL_VEC_3arg(step,(b<a)?0.0f:1.0f);
inline float smoothstep_inner(float edge0,float edge1,float x) {
	float t=clamp((x-edge0)/(edge1-edge0),0.0f,1.0f);
	return (float)(t*t*(3-2*t));
}
SOFTWARE_GLSL_VEC_3arg(smoothstep,smoothstep_inner(a,b,c));

inline float length(float x) {return abs(x);}
template<class vec> inline float length(const vec &v) {return v.mag();}
inline float distance(float x,float y) {return abs(y-x);}
template<class vec> inline float distance(const vec &v,const vec &u) {return v.dist(u);}
inline float dot(float x,float y) {return x*y;}
template<class vec> inline float dot(const vec &v,const vec &u) {return v.dot(u);}
inline vec3 cross(const vec3 &v,const vec3 &u) {return v.cross(u);}
inline float normalize(float x) {return 1.0f;}
template<class vec> inline vec normalize(const vec &v) {return v.dir();}
template<class vec> inline vec faceforward(const vec &N,const vec &I,const vec &Nref) 
	{if (dot(Nref,I)<0) return N; else return -N;}
template<class vec> inline vec reflect(const vec &I,const vec &N) 
	{return I-2*dot(N,I)*N;}
template<class vec> inline vec refract(const vec &I,const vec &N,float eta) 
{
	float k=1.0-eta*eta*(1.0-dot(N,I)*dot(N,I));
	if (k<0.0) return vec(0.0);
	else return eta*I-(eta*dot(N,I)+sqrt(k))*N;
}

/// Componentwise matrix multiplication.
template<class mat>
inline mat matrixCompMul(const mat &x,const mat &y) {
	mat ret;
	for (int r=0;r<mat::n_rows;r++)
	for (int c=0;c<mat::n_cols;c++)
		ret(r,c)=x(r,c)*y(r,c);
	return ret;
}


/* FIXME: vector relational operators */
/* FIXME: samplers & texture routines */
/* FIXME: noise routines */


}; /* end namespace */

#endif
