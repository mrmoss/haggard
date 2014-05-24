/*
  Define serializers for vector2d classes.
  This exists so that the serializer doesn't
  need to be included everywhere; only where it's
  useful.
*/
#ifndef __OSL_VECTOR2D_IO_H
#define __OSL_VECTOR2D_IO_H

#include "osl/serializer.h"
#include "osl/vector2d.h"

IO_CLASS_ALIAS(osl::Polar2d,Polar2d,osl::io::SPparen)
IO_CLASS_ALIAS(osl::Vector2d,Vector2d,osl::io::SPparen)
IO_CLASS_ALIAS(osl::Point,Point,osl::io::SPparen)

#endif //__OSL_VECTOR2D_H
