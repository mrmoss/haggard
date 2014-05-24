/*
Orion's Standard Library
Orion Sky Lawlor, 5/10/2002
NAME:		psobj.h

DESCRIPTION:	C++ PostScript Interpreter Library

This file defines the basic object type used throughout the
interpreter-- ps::Object.

*/
#ifndef __OSL_PSOBJ_H
#define __OSL_PSOBJ_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif

namespace osl { namespace ps {


///Utility class used by interp
template<class T,int max>
class Stack
{
protected:
	T data[max];//Stack (grows up)
	T *bot,*top;//First invalid pointer in either direction
	T *cur;//Pointer to topmost unused element
	void pushErr(void) {throw new PsException(PsException::cantPush);}
	void popErr(void) {throw new PsException(PsException::cantPop);}
public:
	Stack() {
		cur=data;
		bot=data;
		top=data+(max-1);
	}
	inline bool cantPush(void) const {return cur==top;}
	inline void push(const T &t) {
		if (cantPush()) pushErr();
		*(cur++)=t;
	}
	inline bool cantPop(void) const {return cur==bot;}
	inline const T &pop(void) {
		if (cantPop()) popErr();
		return *(--cur);
	}
};
///A Stack that permits random access to its elements
template<class T,int max>
class RamStack : public Stack<T,max>
{
public:
	inline int size(void) const {return this->cur-this->bot;}
	inline T &operator[](int i) {return this->data[i];}
	inline const T &operator[](int i) const {return this->data[i];}
	inline T &peek(void) {return *(this->cur-1);}
	inline const T &index(int n) const {return *(this->cur-1-n);}
	//Pop the top n stack elements
	void popMultiple(int n) { this->cur=this->cur-n; }
};



/******************* Interpreter Internals *******************/

/**
 * This enum lists all the possible types of Object. Postscript
 * doesn't have user-defined Object types, so this is a fixed list.
 */
typedef enum {
	null_ot=0,integer_ot=1,real_ot=2,boolean_ot=3,
	array_ot=4,dictionary_ot=5,string_ot=6,
	name_ot=7,operator_ot=8,font_ot=9,
	file_ot=10,mark_ot=11,save_ot=12,gsave_ot=13, 
	directproc_ot=14,invalid_ot
} ObjectType;

typedef enum {
	attrib_literal=0,attrib_exec=1
} AttribType;

//A unique value identifying this name/font:
typedef const char *NameID;
class FontID;

//Forward declarations:
class Interp;
class VM;
//class psInputStream;
class PsGraphicsState;
class DictionaryEntry;
class Dictionary; class String; class Array;

/**
 * A Postscript operator, implemented as a native language function
 * pointer.  The operator is expected to pop its own input values off the
 * interpreter's stack, do its own type checking, and push its own results.
 *
 * Almost all of the hundreds of built-in Postscript operators are 
 * implemented as static functions of this type.  For example, to 
 * register the Postscript operator "foo" as the C++ function foo, we
 * might do:
 *    interp->systemdict.def(interp,interp->intern("foo"),Object(foo));
 */
typedef void (*Operator)(Interp *p);

/// Helper class for Object.
template <class T>
class ZeroInitialized {
	T v;
public:
	ZeroInitialized() :v(0) {}
	ZeroInitialized(T v_) :v(v_) {}
	operator T&(void) {return v;}
	operator const T&(void) const {return v;}
};

/** 
 * Object is the central class manipulated by the interpreter--
 * kinds of Objects include:
 *   -Numbers, like integers and reals
 *   -Containers of other Objects, like Arrays and Dictionarys
 *   -Postscript operators (function pointers)
 *   -Fonts and Strings
 *   -Bizarre stuff like Files, saved VM or graphics states
 *
 * For easy storage and value semantics, these different types of
 * Object are all represented by this single class, which has a 
 * type code and a "union".  Objects should thus always be 
 * 8 bytes (on 32-bit machines).
 * 
 * Because of this, we're continually checking object type codes;
 * using an Object of the wrong type usually results in an exception
 * (postscript error).  Note also the "overlay" classes Array, Dictionary,
 * String which provide a better typed interface to the basic Object fields.
 */
class Object {
	ZeroInitialized<char> type; //ObjectType
	ZeroInitialized<char> attrib; //Access AttribType
protected:
	ZeroInitialized<unsigned short> len; //Length of composite object
	union {
		//unsigned int null_v; //Null_ot
		int integer_v;
		float real_v;
		bool boolean_v;
		Object *array_v; //VM-stored objects in array
		char *string_v; //VM-stored, non-null-terminated string data
		DictionaryEntry *dictionary_v; //VM-stored dictionary data
		NameID name_v; //Unique name data
		Operator operator_v; //Function pointer
		FontID *font_v; //VM-allocated font
		psInputStream *file_v; //Non VM-allocated file
		VM *save_v; //Heap-allocated VM data
		PsGraphicsState *gsave_v;//VM-stored graphics state
		
		//These probably don't need any data:
		//Mark mark_v; //[, as it lives on the stack
	} v;
public:
	Object(int i) :type(integer_ot) {v.integer_v=i;}
	Object(double f) :type(real_ot) {v.real_v=(float)f;}
	Object(bool b) :type(boolean_ot) {v.boolean_v=b;}
	Object(ObjectType t,Object *arr,int len_) :type(t),len(len_) {v.array_v=arr;}
	Object(char *str,int len_) :type(string_ot),len(len_) {v.string_v=str;}
	Object(DictionaryEntry *d,int len_) :type(dictionary_ot),len(len_)
		{v.dictionary_v=d;}
	Object(NameID l) :type(name_ot) {v.name_v=l;}
	Object(psInputStream *f) :type(file_ot) {v.file_v=f;}
	Object(Operator op) :type(operator_ot),attrib(attrib_exec) {v.operator_v=op;}
	Object(VM *s) :type(save_ot) {v.save_v=s;}
	Object(PsGraphicsState *s) :type(gsave_ot) {v.gsave_v=s;}
	Object(FontID *f) :type(font_ot) {v.font_v=f;}
	
	Object(ObjectType t) :type(t) {}
	Object(void) :type(null_ot) {}
	
	inline void setAttrib(AttribType t) {attrib=t;}
	inline AttribType getAttrib(void) const {return (AttribType)(char)attrib;}
	
	inline ObjectType getType(void) const {return (ObjectType)(char)type;}
	inline bool isInt(void) const {return type==integer_ot;}
	inline bool isReal(void) const {return type==real_ot;}
	
	//These accessors only work right if the object is of the right type:
	inline int getLength(void) const {return len;}
	int getInt(void) const {return v.integer_v;}
	double getReal(void) const {return v.real_v;}
	bool getBool(void) const {return v.boolean_v;}
	NameID getName(void) const {return v.name_v;}
	Object *getArrayData(void) const {return v.array_v;}
	const char *getStrData(void) const {return v.string_v;}
	psInputStream *getFile(void) const {return v.file_v;}
	Operator getOperator(void) const {return v.operator_v;}
	VM *getSave(void) const {return v.save_v;}
	FontID *getFont(void) const {return v.font_v;}
	
	//Throw typecheck if we don't have this type:
	const Object &check(Interp *interp,ObjectType required_ot) const;
	
	//Return this object's data pointer, or NULL if simple
	void *getPointer(void) const;
	
	//Throw typecheck if any of our pointers are greater than this:
	void checkSave(Interp *interp,void *firstInvalid) const;
	
	//Return our numeric value; else error("typecheck")
	double getValue(Interp *i) const;
	
	//Convert to various types:
	String &asString(void) {return *(String *)this;}
	const String &asString(void) const {return *(const String *)this;}
	Dictionary &asDictionary(void) {return *(Dictionary *)this;}
	const Dictionary &asDictionary(void) const {return *(const Dictionary *)this;}
	Array &asArray(void) {return *(Array *)this;}
	const Array &asArray(void) const {return *(const Array *)this;}
	
	//Give a debugging printout
	void print(Interp *interp) const;
};

///A composite object-- one with a sensible length like String or Array
class Composite : public Object {
public:
	inline int size(void) const {return len;}
	void check(Interp *interp,int idx);
};

///A character array
class String : public Composite {
public:
	char &operator[](int i) {return v.string_v[i];}
	const char &operator[](int i) const {return v.string_v[i];}
	void forall(Interp *interp,const Object &proc);
};

///A contiguous list of objects
class Array : public Composite {
public:
	Object &operator[](int i) {return v.array_v[i];}
	const Object &operator[](int i) const {return v.array_v[i];}
	void forall(Interp *interp,const Object &proc);
};

///A complete dictionary, implemented as a hashtable
class Dictionary : public Object {
	static DictionaryEntry *allocate(Interp *interp,int nEnt);
	
	DictionaryEntry *entry(int i) const;
	int nEntries(void) const {return len;}
public:
	Dictionary(void) :Object((DictionaryEntry *)NULL,0) {}
	bool isEmpty(void) const {return v.dictionary_v==NULL;}

	//Set up new dictionary here
	Dictionary(Interp *interp,int nEnt) 
		:Object(allocate(interp,nEnt),nEnt) {}
	
	//Compare these two dictionaries
	bool compare(const Dictionary &d) const {return v.dictionary_v==d.v.dictionary_v;}
	
	//Return the number of bound values in this dictionary
	int usedLength(void);
	void forall(Interp *interp,const Object &proc);
	
	//Copy values from this array.  Throws rangecheck if we aren't empty or don't have room.
	void copyFrom(Interp *interp,const Dictionary &src);
	
	//Return the object bound to this name, else NULL if name unbound
	Object *lookupPtr(NameID name) const;
	
	//Return the object bound to this name, else error("undefined")
	Object &lookup(Interp *i,NameID name) const;
	
	//(re)Bind this name to this value.
	//  error("dictfull") if no more room.
	void def(Interp *i,NameID name,const Object &value);
	//Unbind this name
	void undef(Interp *i,NameID name);
	//Return true if this name is bound
	bool known(NameID name) {return NULL!=lookupPtr(name);}
	
	void printElts(Interp *interp);
};

/******************* Parser Internals ********************/
ObjectType parseNumber(const char *cstr,int *iDest,double *rDest);

}; };

#endif /* defined(thisHeader) */
