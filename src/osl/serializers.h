/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 2003/8/25

These are the routines you'll need to serialize
classes.
*/
#ifndef __OSL_SERIALIZERS_H
#define __OSL_SERIALIZERS_H

#include "osl/io.h"
#include "osl/serializer.h"

namespace osl { namespace io {

/**
A PrintSerializer writes the incoming objects as ASCII 
to a stdio FILE.
*/
class PrintSerializer : public Serializer {
	BufferedOutputStream f;
	int paren; /* parenthesis nesting count */
	int indent; /* indentation nesting count */
	int fields; /* field output counter */
	bool prevParen;
	void printindent(void);
	void print(const char *str);
	void beginField(const char *fieldName);
	void endField(void);
public:
	PrintSerializer(OutputStream &f_);
	virtual void io(void *var,const char *fieldName,Ttype type);
	virtual void ioObject(const char *typeName,const char *fieldName,int flags);
};


/// Print this class to this output stream:
template <class T>
inline void print(const T &t,OutputStream &os) {
	PrintSerializer s(os);
	T *tc=(T *)&t; // const_cast, since "io" routine isn't const.
	tc->io(s);
}

/// Write this class to stdout:
template<class T>
inline void print(const T &t) {
	FileOutputStream os(stdout,false); print(t,os);
}

/// Save this class to this file:
template <class T>
inline void write(const T &t,const File &name) {
	FileOutputStream os(name); print(t,os);
}



class ScanSerializerImpl;
/**
A ScanSerializer reads the incoming objects as ASCII 
from a stdio FILE.  The general format accepted is 

	variable := value ;

or 
	variable := compound {
		...
	};

or (equivalently)
	variable := compound ( value, value ... );

The ':', '=', ';', ',', and even '(' and ')' are optional.
The only things that are required are variable, value,
and '{' and '}'.
*/
class ScanSerializer : public Serializer {
	ScanSerializerImpl *impl;
public:
	ScanSerializer(InputStream &f);
	~ScanSerializer();
	virtual void io(void *var,const char *fieldName,Ttype type);
	virtual void ioObject(const char *typeName,const char *fieldName,int flags);
};


/// Read this class from this file:
template <class T>
inline void read(T &t,InputStream &f) {
	ScanSerializer s(f);
	t.io(s);
}

/// Read this class from this file:
template <class T>
inline void read(T &t,const File &f) {
	FileInputStream is(f); 
	BufferedInputStream bis(is);
	read(t,bis);
}



}; }; /* end namespace osl::io */

#endif
