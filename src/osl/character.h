/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 2003/8/25

A set of C++ character writers/parsers.
I'd love to use something standard like 
stdio for this, but I need the ability to 
read/write from arbitrary sources (regular
files, memory, network, etc.)
*/
#ifndef __OSL_CHARACTER_H
#define __OSL_CHARACTER_H

#include "osl/io.h" /* for osl::int64 type */
#ifdef getchar
#  undef getchar
#endif

namespace osl { namespace io {

typedef enum { 
  ARRAY=10, //A linear array (use begin/end)
  ITEM,     //An item of an array (only begin is needed)

  STRING,   //Copy characters literally during string (use begin/end)
  LINE,     //Delimit area with a newline (only end is needed)

  STRUCT,   //A small type (use begin/end)
  CLASS,    //A large or composite type (use begin/end)
  SECTION,  //An even larger group (use begin/end)
  MODULE    //A still larger group (use begin/end)
} SerializerState;

enum {
  //Applicable to both read & write:
  MARK_BEGIN   = 1u<<1,  //Include human-readable separators for begin/end [default OFF]

  HASH_COMMENT = 1u<<2,  //Treat lines that start with # as comments [default ON]
  BANG_COMMENT = 1u<<3,  //Treat lines that start with ! as comments [default OFF]
  
  FORCE_0xHEX  = 1u<<4,  //Force hexadecimal integer representation: 0x1B3F [default OFF]  
  FORCE_HEX    = 1u<<5,  //Force hexadecimal, but no preceeding 0x: 1B3F [default OFF]  
  FORCE_OCTAL  = 1u<<6,  //Force octal integer representation [default OFF] 
  
  URL_ESCAPE   = 1u<<7, //Use URL-style escape codes: %20, %3F [default OFF]  
  
  //Applicable only to read:
  SKIP_WHITE   = 1u<<10, //Ignore white space before items [default ON]
  SKIP_NEWLINE = 1u<<11, //Ignore newlines before items [default ON] 

  NUM_HEX      = 1u<<12, //Accept integers preceeded with 0x as hex [default ON] 
  NUM_OCTAL    = 1u<<13, //Accept integers preceeded with 0 as octal: 0777 [default OFF] 
  
  BS_ESCAPE    = 1u<<14, //Read C-style backslash escape codes: \n, \t, \073 [default OFF]
  
  READ_STRINGS = 1u<<15, //Match strings literally-- io(const char *) [default ON]

  //Applicable only to write:
  WRITE_STRINGS= 1u<<20, //Write strings literally-- io(const char *) [default ON]
  
};
typedef unsigned int CharacterState;

typedef enum {
	UNKNOWN_EOLN=0, //Don't know yet
	DOS_EOLN=1, // CR-LF ('\r','\n')
	MAC_EOLN=2, // CR ('\r')
	UNIX_EOLN=3 // LF ('\n')
} EolnType;

class CharacterSerializer {
	bool iAmRead;
protected:
	CharacterState state;
	EolnType eoln;
public:
	enum {START_STATE=HASH_COMMENT+SKIP_WHITE+SKIP_NEWLINE+
		NUM_HEX+BS_ESCAPE+READ_STRINGS+WRITE_STRINGS};
	CharacterSerializer(bool isRead,EolnType eoln_)
		:iAmRead(isRead),state(START_STATE),eoln(eoln_) {}
	inline bool isRead(void) const { return iAmRead; }
	
	CharacterState get(void) const {return state;}
	void set(CharacterState to) {state=to;}
	CharacterState add(CharacterState what) {
		CharacterState old=state;
		state|=what;
		return old;
	}
	CharacterState sub(CharacterState what) {
		CharacterState old=state;
		state&=~what;
		return old;
	}
	CharacterState remove(CharacterState what) {return sub(what);}
	bool has(CharacterState what) const {return 0!=(state&what);}
	
	void setEoln(EolnType lineEnd) {eoln=lineEnd;}
	EolnType getEoln(void) const {return eoln;}
	
	virtual int bytesUsed(void) const;
};

class NumberFormatException : public IOException {
public:
	OSL_EXCEPTION_DECL(NumberFormatException,IOException)
};

class CharacterReader : public CharacterSerializer {
	InputStream &s;
	bool instring; //While in string, return white space & escapes
	Char curc;
	int lineNo;
	
	void bad(const char *why);
	void skipIfNeeded(void);
	Char peek(void) {if (curc==0) next(); return curc;}
	//Advance to the next character
	void next(void);
	//Consume the current character
	void flush(void) {curc=0;}
	Char getchar(void) {Char c=peek(); flush(); return c;}
	
	//Read an unsigned hex or octal number.  Ret must be initialized to 0;
	// number of digits actually read is returned.
	int readhex(int maxdigits,int *ret);
	int readoct(int maxdigits,int *ret);
	
	//Read a sign character (+ or -) and return +1 or -1
	int readsign(void);
	uint64 readdec(void);
	
	void matchStr(const char *str);
	
public:
	CharacterReader(InputStream &s_);
	virtual void io(char &v);
	virtual void io(short &v) { int64 i; io(i); v=i; }
	virtual void io(int &v)   { int64 i; io(i); v=i; }
#if OSL_NEED_INT64
	virtual void io(long &v)  { int64 i; io(i); v=i; }
#endif
	virtual void io(int64 &v);
	virtual void io(signed char &v) { int64 i; io(i); v=i; }
	virtual void io(unsigned char &v)  { uint64 i; io(i); v=i; }
	virtual void io(unsigned short &v) { uint64 i; io(i); v=i; }
	virtual void io(unsigned int &v)   { uint64 i; io(i); v=i; }
#if OSL_NEED_INT64
	virtual void io(unsigned long &v)  { uint64 i; io(i); v=i; }
#endif
	virtual void io(uint64 &v);
	
	virtual void io(float &v)       { double i; io(i); v=i; }
	virtual void io(double &v);
	virtual void io(bool &v);
	virtual void io(void *str,int n);
	virtual void io(const char *str);
	
	virtual bool begin(SerializerState kind,void *what,char *description);
	virtual void end(SerializerState kind);
	
	//Read until non-white encountered.  Return true if characters were skipped
	bool skipWhite(void); 
	//Read until this line ends
	void skipLine(void); 
	
	//Process characters after a backslash:
	Char processBsEscape(void);
	//Process characters after an URL-style percent:
	Char processUrlEscape(void);
	
};

class CharacterWriter : public CharacterSerializer {
	OutputStream &s;
	enum {floatFmtLen=20};
	char floatFmt[floatFmtLen];
	virtual void space(void);
	virtual void write(const char *str);
	void io_uint(uint64 v);
public:
	CharacterWriter(OutputStream &s_);
	virtual void io(char &v);
	virtual void io(short &v) { int64 i=v; io(i); }
	virtual void io(int &v)   { int64 i=v; io(i); }
#if OSL_NEED_INT64
	virtual void io(long &v)  { int64 i=v; io(i); }
#endif
	virtual void io(int64 &v);
	virtual void io(signed char &v) { int64 i=v; io(i); }
	virtual void io(unsigned char &v)  { uint64 i=v; io(i); }
	virtual void io(unsigned short &v) { uint64 i=v; io(i); }
	virtual void io(unsigned int &v)   { uint64 i=v; io(i); }
#if OSL_NEED_INT64
	virtual void io(unsigned long &v)  { uint64 i=v; io(i); }
#endif
	virtual void io(uint64 &v);
	virtual void io(float &v)       { double i=v; io(i); }
	virtual void io(double &v);
	virtual void io(bool &v);
	virtual void io(void *str,int n);
	virtual void io(const char *str);
	
	virtual bool begin(SerializerState kind,void *what,char *description);
	virtual void end(SerializerState kind);
	
	//Set the output format, a la printf: "%.8f" for 8 decimal places; "%.0f" to omit decimal,
	// "%.17g" for scientific notation, etc.
	void setFloatFormat(const char *str);
};



}; }; /* end namespace osl::io */

#endif
