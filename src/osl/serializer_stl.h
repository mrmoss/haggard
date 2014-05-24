/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 2003/8/25

Serializer.h implementations for STL classes string and vector.
Because Koenig lookups actually foil a "fair fight" between
overloads, this is all dumped to the top-level namespace.
*/
#ifndef __OSL_SERIALIZER_STL_H
#define __OSL_SERIALIZER_STL_H

#include <string>
#include <vector>
#include "osl/serializer.h"

/** Support for stl::vector */
template <class T>
void ioCallSerializer(osl::io::Serializer &s,std::vector<T> *v,const char *fieldName) {
	const char *typeName="vector";
	s.ioObject(typeName,fieldName,0);
	int size=v->size();
	IO(size);
	if (s.isFill()) v->resize(size);
	for (int i=0;i<size;i++) {
		T &value=(*v)[i];
		IO(value);
	}
	s.ioObject(typeName,0,0);
}

/** Support for std::string */
inline void ioCallSerializer(osl::io::Serializer &s,std::string *v,const char *fieldName) {
	const char *typeName="string";
	s.ioObject(typeName,fieldName,0);
	int length=v->size();
	IO(length);
	if (s.isFill()) v->resize(length);
	for (int i=0;i<length;i++) {
		char &value=(*v)[i];
		IO(value);
	}
	s.ioObject(typeName,0,0);
}

#endif
