/*
Orion's Standard Library
Orion Sky Lawlor, 3/20/2002
NAME:		psinterp.h

DESCRIPTION:	C++ PostScript Interpreter Library

This is the lowest layer of the Postscript interpreter--
it tokenizes the incoming stream according to the standard
Postscript rules.
*/
#ifndef __OSL_PSPARSE_H
#define __OSL_PSPARSE_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#ifndef __OSL_IO_H
#  include "osl/io.h"
#endif
#ifndef __OSL_PS_H
#  include "osl/ps.h"
#endif

namespace osl { namespace ps {


//This inputstream always keeps 1 character ready in its buffer.
class LookaheadInputStream : public ::osl::io::InputStream {
	::osl::io::InputStream &in;
	char c;
	bool needsChar;
	bool hitEOF;
	//Advance to the next character
	void nextchar(void);
public:
	inline bool atEOF(void) { return hitEOF; }
	
	//Return the next character in the stream
	inline char peek(void) {
		if (needsChar) nextchar();
		return c;
	}
	
	//Consume this character
	inline void consume(void) {
		needsChar=true;
	}
	
	LookaheadInputStream(::osl::io::InputStream &in_); 
	virtual int readPartial(void *dest,int len);
};

typedef LookaheadInputStream psInputStream;

/******************* Parser Externals ********************/
class TokenSink {
public:
	virtual ~TokenSink();
	
	///Begin reading from this input stream.
	/// Returns the old stream.  Default is a no-op.
	virtual psInputStream *swapStreams(psInputStream *s);
	
	///Starting work on a new line-- the first line is numbered 1
	virtual void newline(int lineNo) {}
	
	///Encountered a line of postscript comment, less the initial % and trailing newline
	/// Default implementation just ignores comment.
	virtual void comment(char *body);
	
	///Encountered something that doesn't make any sense here
	/// Default implementation calls bad() and exits. 
	virtual void parseError(const char *desc);
	
	///Encountered a literal integer:
	virtual void integer(int i) =0;
	
	///Encountered a literal double number:
	virtual void real(double f) =0;
	
	///Encountered a literal name, \foo:
	virtual void literalName(const char *str) =0;
	
	///Encountered an immediate name, \\foo:
	virtual void immediateName(const char *str) =0;
	
	///Encountered an executable name, foo:
	virtual void execName(const char *str) =0;
	
	///Encountered a postscript string, less the isolating ()'s or <>'s
	///  <> strings are converted to binary.
	///  () strings have escapes removed.
	virtual void string(const char *str,int len) =0;
	
	///Encountered { or }
	virtual void beginProc(void) =0;
	virtual void endProc(void) =0;
};



class buf_t; //A little auto-terminating, bounds-checking character buffer

/**
 * This is the postscript parser itself--it sends tokens to dest.
 */
class Parser {
	LookaheadInputStream &src; //Source of characters
	inline bool atEOF(void) { return src.atEOF(); }
	inline char peek(void) { return src.peek(); }
	inline void consume(void) { src.consume(); }
	
	TokenSink &dest; //Destination for encountered tokens
	int curLineNo;
	void skipNewline(void);
	bool eofError(void);
	
	void error(const char *why);
	void parseString(void);
	void parseHexString(void);
	void parseComment(void);
	void copyIdentifier(buf_t &buf);
	void handleUnknown(const char *str);
public:
	Parser(LookaheadInputStream &src_,TokenSink &dest_);
	
	/// Parse the entire input file, until EOF
	void parse(void);
	
	/// Parse the next token in the file.  Returns false if we're at EOF
	bool parseNext(void);
	
	/// Return the current line number in the file
	int getLineNo(void) const { return curLineNo; }
};




/**
 * Parse postscript tokens, putting them into the given destination.
 * Only stops on an error or EOF.
 * Returns true if the interpretation worked.
 */
bool parse(::osl::io::InputStream &s,::osl::ps::TokenSink &dest);

/**
 * As above, but does not trap exceptions.
 */
void parseMayThrow(::osl::io::InputStream &s,::osl::ps::TokenSink &dest);

/******************* Parser Internals ********************/
//Return true if a PostScript "int" can safely hold this value
bool inIntRange(double d);

//Convert alphanumeric to actual value (0..35)
// returns -1 for non-alphanumeric character.
int toVal(char c);

}; };

#endif /* defined(thisHeader) */
