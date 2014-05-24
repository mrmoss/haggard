/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 2004/11/14

*/
#ifndef __OSL_COLOR_NORMAL_H
#define __OSL_COLOR_NORMAL_H

#ifndef __OSL_COLOR_H
#  include "osl/color.h"
#endif

#ifndef __OSL_VECTOR3D_H
#  include "osl/vector3d.h"
#endif

namespace osl {

using osl::graphics2d::Color;

/**
  Return an RGB color to encode this unit normal vector.
*/
inline Color colorFmNormal(const Vector3d &v) {
	return Color(0.5+0.5*v.x, 0.5+0.5*v.y, 0.5+0.5*v.z);
}

/**
  Return a (not quite unit) normal vector from this RGB color.
  Not quite a unit normal because of quantization and interpolation.
*/
inline Vector3d normalFmColor(const Color &v) {
	return Vector3d((v.r-0.5)*2, (v.g-0.5)*2, (v.b-0.5)*2);
}

};

#endif
