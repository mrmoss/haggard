/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 2003/8/25

A set of highly templated C++ serialization/deserialization
routines.  These are designed to be flexible, 
but easy to use. 

This is the only header you need when writing a serializable class.
*/
#ifndef __OSL_SERIALIZER_H
#define __OSL_SERIALIZER_H

#include <typeinfo>
#include "osl/osl.h" /* for osl::int64 type */

namespace osl { namespace io {

class Serializer;

/*************** Serializer Type system: ***************/
#if OSL_NEED_INT64
#  define OSL_NEED_INT64_YES(x) x
#else
#  define OSL_NEED_INT64_YES(x) /* empty */
#endif
#if OSL_HAS_LONG_DOUBLE
#  define OSL_HAS_LONG_DOUBLE_YES(x) x
#else
#  define OSL_HAS_LONG_DOUBLE_YES(x) /* empty */
#endif

#define OSL_SERIALIZER_MAP_TYPE_3(fn1,fn2,fn3) \
	fn1(bool,bool) \
	fn1(char,char) fn1(signed char,schar) fn1(unsigned char,uchar) \
	fn1(short,short) fn1(unsigned short,ushort) \
	fn1(int,int) fn1(unsigned int,uint) \
	fn2(long,long) fn2(unsigned long,ulong) \
	fn3(float,float) fn3(double,double) \
OSL_NEED_INT64_YES( fn2(osl::int64,int64) fn2(osl::uint64,uint64) ) \
OSL_HAS_LONG_DOUBLE_YES( fn3(long double,longdouble) )

#define OSL_SERIALIZER_MAP_TYPE(fn) OSL_SERIALIZER_MAP_TYPE_3(fn,fn,fn)

/// Basic type system for serializers.
typedef enum {
	Tinvalid=0, //< Invalid datatype
	
#define OSL_IO_ENUM_DATATYPE(type,name) T##name,
	/// Datatypes like "Tint", "Tuchar", etc.
	OSL_SERIALIZER_MAP_TYPE(OSL_IO_ENUM_DATATYPE)
	Tlast, //< Last datatype (length of per-datatype arrays, etc.)
} Ttype;

/// Serialization property flags.
typedef enum {
	SPparen=1<<10 ///< Object wants (short) parenthesis, not curly-braces.
} SPflags;


/**
A Serializer writes (fill==false) or reads (fill==true) variables.
This class is abstract-- it will be inherited from, and 
the "io" method will be filled out.
*/
class Serializer {
	bool fill;
public:
	Serializer(bool fill_) :fill(fill_) {}
	/** Return true if this Serializer fills in values passed to it
	    (i.e., it's called on uninitialized objects to fill them out).
	    For example, a file reader has fill==true; a 
	    file writer has fill==false 
	 */
	inline bool isFill(void) const {return fill;}
	virtual ~Serializer();
	
	/**
	 * Read or write this variable.
	 *  @param var The variable to read or write.
	 *  @param fieldName The name of the variable.
	 *  @param Ttype The datatype of the variable.
	 */
	virtual void io(void *var,const char *fieldName,Ttype type) =0;
	
	/**
	 * Note the beginning or ending of an object. 
	 *  @param typeName The datatype being serialized.
	 *  @param fieldName The name of the object being serialized, or 0 at object end.
	 *  @param flags Flags from SPflags, above.
	 * The default implementation does nothing-- object start/stops are ignored.
	 */
	virtual void ioObject(const char *typeName,const char *fieldName,int flags);
};


/**
 * This macro passes a variable to the Serializer "s".
 *  Users call it on a local like:
 *     IO(foo);
 */
#define IO(v) ioCallSerializer(s,&v,#v)

/**
 * This macro passes a pointer-accessed variable to the Serializer "s".
 *  Users call it on a local like:
 *     IO_PTR(fooPtr);
 */
#define IO_PTR(v) ioCallSerializer(s,v,#v)


/**
Cleans up a compiler-generated <typeinfo>
name to be human-readable.  Used by the default ioCallSerializer.
*/
const char *DataTypeCleanupRTTIname(const char *srcName);

/**
 Defines an ioCallSerializer for most objects.
 You must declare this macro outside any namespaces; otherwise
 your ioCallSerializer routine will override all others.
*/
#define IO_CLASS(type,flags) \
inline void ioCallSerializer(osl::io::Serializer &s,type *var,const char *fieldName) { \
	s.ioObject(#type,fieldName,flags); \
	var->io(s); \
	s.ioObject(#type,0,flags); \
}

/**
 Defines an ioCallSerializer for objects that are named one thing, but saved as another.
 You must declare this macro outside any namespaces.
*/
#define IO_CLASS_ALIAS(type,typeName,flags) \
inline void ioCallSerializer(osl::io::Serializer &s,type *var,const char *fieldName) { \
	s.ioObject(#typeName,fieldName,flags); \
	var->io(s); \
	s.ioObject(#typeName,0,flags); \
}

}; }; /* end namespace osl::io */


/// Declare direct ioCallSerializers for the builtin types:
#define OSL_IO_DECLARE_DATATYPE(type,name) \
  inline void ioCallSerializer(osl::io::Serializer &s,type *t,const char *fieldName) { \
     s.io(t,fieldName,osl::io::T##name); \
  }
OSL_SERIALIZER_MAP_TYPE(OSL_IO_DECLARE_DATATYPE)


/**
 Default ioCallSerializer, for all objects without an IO_CLASS declaration.
 Extracts the datatype name using RTTI.
*/
template <class T>
inline void ioCallSerializer(osl::io::Serializer &s,T *var,const char *fieldName) {
	const char *typeName=osl::io::DataTypeCleanupRTTIname(typeid(T).name());
	s.ioObject(typeName,fieldName,0);
	var->io(s);
	s.ioObject(typeName,0,0);
}



#endif
