/*
Orion's Standard Library
Orion Sky Lawlor, 11/3/1999
NAME:		Matrix_util.cpp

DESCRIPTION:	C++ Matrix library (no templates)

*/
#include "osl/matrix2d.h"
#include "osl/matrix3d.h"
using namespace osl;

/*********** Matrix3d *********/
Vector3d Matrix3d::apply(const Vector3d &in) const {
   Vector3d dest;
   dest.x=data[0][0]*in.x+data[0][1]*in.y+data[0][2]*in.z+data[0][3];
   dest.y=data[1][0]*in.x+data[1][1]*in.y+data[1][2]*in.z+data[1][3];
   dest.z=data[2][0]*in.x+data[2][1]*in.y+data[2][2]*in.z+data[2][3];
   return dest;
}

//Apply this Matrix to the given direction vector (no offset added)
Vector3d Matrix3d::applyDirection(const Vector3d &in) const {
   Vector3d dest;
   dest.x=data[0][0]*in.x+data[0][1]*in.y+data[0][2]*in.z;
   dest.y=data[1][0]*in.x+data[1][1]*in.y+data[1][2]*in.z;
   dest.z=data[2][0]*in.x+data[2][1]*in.y+data[2][2]*in.z;
   return dest;
}
//Apply this Matrix to the given homogenous vector
Vector4d Matrix3d::applyHomogenous(const Vector4d &in) const {
   Vector4d dest;
   dest.x=data[0][0]*in.x+data[0][1]*in.y+data[0][2]*in.z+data[0][3]*in.w;
   dest.y=data[1][0]*in.x+data[1][1]*in.y+data[1][2]*in.z+data[1][3]*in.w;
   dest.z=data[2][0]*in.x+data[2][1]*in.y+data[2][2]*in.z+data[2][3]*in.w;
   dest.w=data[3][0]*in.x+data[3][1]*in.y+data[3][2]*in.z+data[3][3]*in.w;
   return dest;
}
void Matrix3d::identity(flt s) {
	data[0][0]=s;data[0][1]=0;data[0][2]=0;data[0][3]=0;
	data[1][0]=0;data[1][1]=s;data[1][2]=0;data[1][3]=0;
	data[2][0]=0;data[2][1]=0;data[2][2]=s;data[2][3]=0;
	data[3][0]=0;data[3][1]=0;data[3][2]=0;data[3][3]=1;
}

//Scale this Matrix up by the specified factors
void Matrix3d::scale(const Vector3d &fac)	
  {data[0][0]*=(flt)fac.x;data[0][1]*=(flt)fac.y;data[0][2]*=(flt)fac.z;
   data[1][0]*=(flt)fac.x;data[1][1]*=(flt)fac.y;data[1][2]*=(flt)fac.z;
   data[2][0]*=(flt)fac.x;data[2][1]*=(flt)fac.y;data[2][2]*=(flt)fac.z;}

//Make this Matrix a rotation Matrix, right-handed about +x axis
// (Matrix must be freshly initialized to identity)
void Matrix3d::rotateX(double radians)
{
	flt c=(flt)cos(radians),s=(flt)sin(radians);
	data[1][1]=c;data[1][2]=-s;
	data[2][1]=s;data[2][2]=c;
}
void Matrix3d::rotateY(double radians)
{
	flt c=(flt)cos(radians),s=(flt)sin(radians);
	data[0][0]=c;data[0][2]=s;
	data[2][0]=-s;data[2][2]=c;
}
void Matrix3d::rotateZ(double radians)
{
	flt c=(flt)cos(radians),s=(flt)sin(radians);
	data[0][0]=c;data[0][1]=-s;
	data[1][0]=s;data[1][1]=c;
}
