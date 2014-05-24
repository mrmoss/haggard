/*
Orion's Standard Library
Orion Sky Lawlor, 3/20/2002
NAME:		psinterp.h

DESCRIPTION:	C++ PostScript Interpreter Library

This internal postscript interpreter file has the actual
declaration of the core interpreter itself.

*/
#ifndef __OSL_PSINTERP_H
#define __OSL_PSINTERP_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#ifndef __OSL_MATRIX2D_H
#  include "osl/matrix2d.h"
#endif
#ifndef __OSL_HASHTABLE_H
#  include "osl/hashtable.h"
#endif
#ifndef __OSL_PSPARSE_H
#  include "osl/psparse.h"
#endif
#ifndef __OSL_PSOBJ_H
#  include "osl/psobj.h"
#endif
#ifndef __OSL_PSGRAPHICS_H
#  include "osl/psgraphics.h"
#endif

namespace osl { namespace ps {

/**
 * Virtual Memory, the (never-reclaimed) heap for a postscript program. 
 */
class VM {
	char *storage;//Start of memory
	char *cur;//Next unused byte in memory
	char *end;//1+end of memory
	bool isGlobal;
public:
	VM(bool global,int nBytes);
	~VM();
	
	bool getGlobal(void) const {return isGlobal; }
	
	int usedBytes(void) const { return cur-storage; }
	int maxBytes(void) const { return end-storage; }
	
	char *allocate(Interp *interp,int nBytes);
	VM *save(void);
	
	/// Get the first pointer that would be invalidated 
	/// if we restored from this image.
	void *checkRestore(VM *from) { return storage+(from->cur-from->storage); }
	
	void restore(VM *from);
};

/** 
 * Create a new C++ object in Postscript VM storage.  This provides a 
 * nice method for C++ objects (with nice virtual methods) to use 
 * the Postscript garbage collection machinery; preventing nasty 
 * memory leaks.
 *
 * One serious caveat is that destructors are generally
 * *not* called, and (as you might expect) calling delete[] on a 
 * VM_NEW'd object is disasterous.
 *
 * You might this macro like:
 *    int *i=OSL_PS_VM_NEW(interp,foo)(17,32); //like "new foo(17,32)"
 */
#define OSL_PS_VM_NEW(interp,T) new (interp->vm->allocate(interp,sizeof(T))) T



class CString {
	enum {bufLen=255};
	char buf[bufLen+1];
public:
	CString(Interp *interp,const String &s);
	operator const char *() const {return buf;}
};

/// An ObjectTokenSink is the plumbing that connects the parser
/// to the running Interpreter.
class ObjectTokenSink : public TokenSink {
	int procLevel; //number of nested procedures
	Interp *interp;
public:
	ObjectTokenSink(Interp *interp_);
	
//TokenSink interface:
	virtual psInputStream *swapStreams(psInputStream *s);
	virtual void newline(int lineNo);
	
	virtual void comment(char *body);
	
	virtual void integer(int i);
	virtual void real(double f);
	virtual void literalName(const char *str);
	virtual void immediateName(const char *str);
	virtual void execName(const char *str);
	virtual void string(const char *str,int len);
	
	virtual void beginProc(void);
	virtual void endProc(void);
};


/**
 * The actual postscript interpreter.  This is a huge and truly
 * hideous class, which includes the entire Postscript virtual machine,
 * execution environment, and dozens of little utility routines.
 */
class ExecStackPush;
class Interp : public InterpGraphics {
public:
	enum {operandMax=8192,dictMax=20,
		userdictLen=200,systemdictLen=400,statusdictLen=20,
		execMax=250,recurseMax=10,saveMax=15,
		pathMax=1500,dashMax=11,vmMax=1000*1024};
	
private:
	Interp *interp; //==this, for convenience
	
	//Internalized (canonicalized) names:
	// Hash key as well as object are a heap-allocated string.
	class namePool_t {
		hashtable table;
	public:
		namePool_t();
		~namePool_t();
		NameID intern(const char *cStr);
	};
	namePool_t namePool;
	Operator lastoperator;
	Dictionary dollarError; //$error dict.
	Dictionary errordict;
	void errorInit(void); //Called during startup
	
	RamStack<Object,operandMax> operand;
	
	friend class ExecStackPush;
	RamStack<Object,execMax> execstack;
public:
	TokenSink *curSink; //Sink that's currently accepting (for dsc Comments)
	int curLineNo;//For debugging

	Dictionary systemdict,userdict,statusdict;

	//NameID's for predefined strings returned by "type" operator
	NameID nulltype,integertype,realtype,booleantype,
		arraytype,dictionarytype,stringtype,
		nametype,operatortype,fonttype,
		filetype,marktype,savetype;
	
	RamStack<Dictionary,dictMax> dictionary;
	RamStack<VM *,saveMax> save;
	
	int loopingLevel; //number of recursive "exit"-able calls
	int stopLevel; //number of recursive "stop"-able calls
	
	//Exceptions that should never propagate outside:
	class ExitException {};
	class StopException {};
	
	//The current virtual memory
	VM *vm;
	Object emptyArray;
	
	//The current input file
	psInputStream *inStream;

	Interp(int vmSize=vmMax);
	virtual ~Interp();
	
	//Convert name to ID:
	inline NameID intern(const char *csymName) {
		return namePool.intern(csymName);
	}
	NameID intern(const char *symName,int symLen);
	//Convert name/string to key, or throw typecheck
	NameID getKey(const Object &o);
	
	void parse(::osl::io::InputStream &is);
	void parse(const char *src); 
	
	//Do dictionary lookup:
	const Object &lookup(NameID n);
	const Object *lookupPtr(NameID n);
	
	//Execute this object
	virtual void exec(const Object &o);
	void exec(NameID n);
	
	//Fatal error-- execute postscript handler and throw error
	virtual void error(const char *errName,const char *errDetail=NULL);
	
	inline const Object &pop(void) {
		if (operand.cantPop()) error("stackunderflow");
		return operand.pop();
	}
	const Object &pop(ObjectType ot);
	void pop(int n);//Pop top n elements
	int popInt(void); //Pop an int, or throw typecheck.
	const Array &popArray(void); //Pop an array, or throw typecheck.
	double popValue(void); //Pop a number, or throw typecheck.
	Vector2d popPoint(void); //Pop two numbers, or throw typecheck.
	
	inline void push(const Object &o) {
		if (operand.cantPush()) error("stackoverflow");
		operand.push(o);
	}
	
	//Incoming Document Structuring Convention comments:
	virtual void dscComment(char *keyword);
	virtual void dscCommentValues(char *keyword,char *values);

//The rest are rather bizarre utility functions and accessors, needed
// to let me keep most of my data private.
	//Print all objects on the stack
	void print(void);
	
	Array buildArray(int len) {
		Object *data=(Object *)vm->allocate(this,len*sizeof(Object));
		Object o(array_ot,data,len);
		return o.asArray();
	}
	Object buildProc(int len) {
		Object *data=(Object *)vm->allocate(this,len*sizeof(Object));
		Object o(directproc_ot,data,len);
		return o;
	}
	Dictionary buildDictionary(int len) {
		if (len<0 || len>65536) error("rangecheck");
		Dictionary d(interp,len);
		return d;
	}
	void beginDictionary(const Dictionary &d) { dictionary.push(d); }
	void endDictionary(void) { 
		if (dictionary.size()<=2) error("dictstackunderflow");
		dictionary.pop(); 
	}
	
	//Extract the n'th element from the stack (0==tos)
	const Object &index(unsigned int nDown);
	//Roll top n elements to the right by one space
	void roll(int n);
	int count(void) {return operand.size();}
	void restore(VM *save);
	
	//Return the number of stack items on top of the mark
	int counttomark(void);
	
	//Pop the top n objects into this location
	void pop(int n,Object *dest);
	//Push these n objects
	void push(int n,const Object *src);
};

/// Pop a file_ot, string_ot, or string-returning procedure--
/// used by "filter" and image commands.
::osl::io::InputStream *popInputStream(Interp *interp);
::osl::io::InputStream *createInputStream(Interp *interp,const Object &src);

}; };

#endif /* defined(thisHeader) */
