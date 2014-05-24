/*
Orion's Standard Library
Orion Sky Lawlor, 1/12/2002
NAME:		osl/io.h

DESCRIPTION:	C++ I/O library

The big differences with the C++ standard I/O library are:
-Focus on binary I/O.  This means we don't have to worry about
locales, templating over char_t, controlling output format, etc.
-Much simpler (I think).
-Easy to extend (unlike the horror of streambuf).
*/
#ifndef __OSL_IO_H
#define __OSL_IO_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#include <stdio.h> /* for FILE declaration */
#include "osl/io_types.h"

namespace osl { namespace io {

class IOException : public Exception {
public:
	OSL_EXCEPTION_DECL(IOException,Exception)
};
class FileNotFoundException : public IOException {
public:
	OSL_EXCEPTION_DECL(FileNotFoundException,IOException)
};
class EOFException : public IOException {
public:
	OSL_EXCEPTION_DECL(EOFException,IOException)
};
class UnseekableException : public IOException {
public:
	OSL_EXCEPTION_DECL(UnseekableException,IOException)
};

//Abstract base classes for byte-oriented binary I/O:
class Stream : public Noncopyable {
public:
	//Read or write nBytes of the given data.
	// Throws an IOException if the operation could not complete.
	// Default implementation in terms of streamPartial.
	virtual void stream(void *data,int nBytes);
	
	//As above, but will only return a partial result rather than throwing
	// an EOFException
	virtual int streamPartial(void *data,int nBytes) =0;
	
	//Seek the stream to this point
	// Throws an UnseekableException if the stream doesn not support seeking.
	virtual void seek(int64 firstByte);
	
	//Return the current location in the file.
	// Returns -1 if the file does not support seeking.
	virtual int64 tell(void);

	/// Skip over the next n bytes in the file.
	/* virtual */ void skip(int64 n);
	
	//Close the stream
	virtual ~Stream();
	
	//Flush any cached data (implicit on close)
	virtual void flush(void);
	
	//Give a one-word description of this stream (e.g., "foo.txt")
	virtual const char *getDescription(void);
};

class InputStream : public Stream {
public:
	/*Read nBytes from this stream.
	  If the read could not be completed, throw an IOException.
	  Default implementation is in terms of readPartial.
	 */
	virtual void read(void *dest,int nBytes);
	
	/*As above, but return the number of bytes read successfully;
	  never throws an EOFException.
	 */
	virtual int readPartial(void *dest,int nBytes) =0;
	
	//Calls read
	virtual void stream(void *data,int nBytes);
	virtual int streamPartial(void *data,int nBytes);
};

class OutputStream : public Stream {
public:
	/*Write nBytes to this stream.
	  firstByte works like InputStream::read.
	  Throws an IOException on error.
	 */
	virtual void write(const void *src,int nBytes) =0;
	
	//Calls write
	virtual void stream(void *data,int nBytes);
	virtual int streamPartial(void *data,int nBytes);
};

//Stream bytes to/from a flat memory array.  
//  The array is not deleted by us when done.
class ByteArrayInputStream : public InputStream {
	const byte *start, *cur, *end;
public:
	ByteArrayInputStream(const void *src,int len_);
	virtual int readPartial(void *dest,int nBytes);
	void seek(int64 to) {cur=start+to;}
	int64 tell(void) {return cur-start;}
	
	const char *getDescription(void) {return "byte array input stream";}
};
class ByteArrayOutputStream : public OutputStream {
	byte *start, *cur, *end;
public:
	ByteArrayOutputStream(void *dest,int len_);
	virtual void write(const void *src,int nBytes);
	void seek(int64 to) {cur=start+to;}
	int64 tell(void) {return cur-start;}
	
	const char *getDescription(void) {return "byte array output stream";}
};

/// Describes the name of a file or directory.
class implFileStatus;
class File {
	String name;
	bool checkedStatus;
	implFileStatus *status;
	implFileStatus *getStatus(void);
	inline void initStatus(void) { checkedStatus=false; status=NULL; }
public:
	/// This character is used to divide paths, in string and character forms.
	///  For example, on a UNIX machine, separator=="/".
	const static String separator;
	const static Char separatorChar;

	File(const String &name_);
	File(const char *name_);
	File(const File &src);
	/// Create this file 
	File(const String &name_,const String &extension_);
	File &operator=(const File &src);
	~File();
	operator const String &() const {return name;}
	operator const char *() const {return name.c_str();}
	
	/// Return this file's name as a C string.
	const char *toString(void) const;
	
	///  Return the name of our parent (enclosing) directory.
	String getParent(void) const;
	
	///  Return this File's base name, without the 
	///  directory portion.
	String getName(void) const;
	
	/// File properties:
	bool deleteFile(void);
	bool exists(void) {return canRead();}
	bool canRead(void);
	bool canWrite(void);
	bool isDirectory(void);
	bool isFile(void);
	bool isHidden(void);
	int64 length(void);
};

//File I/O:
class implFileDescriptor; //Implementation class
class FileInputStream : public InputStream {
	implFileDescriptor *file;
public:
	//Based on abstract File name (portable)
	FileInputStream(const File &name);
	//Based on UNIX file descriptor (UNIX only)
	FileInputStream(int fd,bool closeFdWhenDone=true,const char *desc_="unix fd input");
#if 1 /* #ifdef _STDIO_H <- to prevent "Unknown symbol FILE" errors*/
	//Based on stdio file pointer
	FileInputStream(FILE *f,bool closeFWhenDone=true,const char *desc_="stdio input");
#endif
	
	virtual ~FileInputStream();
	virtual int readPartial(void *dest,int nBytes);
	virtual void seek(int64 firstByte);
	virtual int64 tell(void);
	virtual const char *getDescription(void);
};
class FileOutputStream : public OutputStream {
	implFileDescriptor *file;
public:
	//Based on abstract File name (portable)
	FileOutputStream(const File &name);
	//Based on UNIX file descriptor (UNIX only)
	FileOutputStream(int fd,bool closeFdWhenDone=true,const char *desc_="unix fd output");
#if 1 /* #ifdef _STDIO_H <- to prevent "Unknown symbol FILE" errors*/
	//Based on stdio file pointer
	FileOutputStream(FILE *f,bool closeFWhenDone=true,const char *desc_="stdio output");
#endif
	
	virtual ~FileOutputStream();
	virtual void write(const void *src,int nBytes);
	virtual void seek(int64 firstByte);
	virtual int64 tell(void);
	virtual const char *getDescription(void);
};

//Buffered I/O
class implBuffer;
class BufferedInputStream : public InputStream {
	implBuffer *buf;
public:
	BufferedInputStream(InputStream *s_,bool freeWhenDone=false,int bufLen=8192);
	BufferedInputStream(InputStream &s,int bufLen=8192);
	~BufferedInputStream();
	virtual void read(void *dest,int nBytes);
	virtual int readPartial(void *dest,int nBytes);
	virtual void seek(int64 firstByte);
	virtual int64 tell(void);
	virtual const char *getDescription(void);
};
class BufferedOutputStream : public OutputStream {
	implBuffer *buf;
public:
	BufferedOutputStream(OutputStream *s_,bool freeWhenDone=false,int bufLen=8192);
	BufferedOutputStream(OutputStream &s,int bufLen=8192);
	~BufferedOutputStream();
	virtual void write(const void *src,int nBytes);
	virtual void flush(void);
	virtual void seek(int64 firstByte);
	virtual int64 tell(void);
	virtual const char *getDescription(void);
};


//Smart pointer classes to hold streams-- 
//  Useful as a consistent method for allocation/free, and to automatically
// open a file during a function call.
class InputStreamHolder : public Noncopyable {
	osl::io::InputStream *s;
	bool closeOnExit;
public:
	InputStreamHolder(InputStream &sref_) 
		:s(&sref_),closeOnExit(false) {}
	InputStreamHolder(InputStream *s_,bool close=false) 
		:s(s_),closeOnExit(close) {}
	InputStreamHolder(const char *fName) 
		:s(new FileInputStream(fName)),closeOnExit(true) {}
	InputStreamHolder(const File &fName) 
		:s(new FileInputStream(fName)),closeOnExit(true) {}
	virtual ~InputStreamHolder();
	InputStream &getStream(void) const {return *s;}
};
class OutputStreamHolder : public Noncopyable {
	osl::io::OutputStream *s;
	bool closeOnExit;
public:
	OutputStreamHolder(OutputStream &sref_) 
		:s(&sref_),closeOnExit(false) {}
	OutputStreamHolder(OutputStream *s_,bool close=false) 
		:s(s_),closeOnExit(close) {}
	OutputStreamHolder(const char *fName) 
		:s(new FileOutputStream(fName)),closeOnExit(true) {}
	OutputStreamHolder(const File &fName) 
		:s(new FileOutputStream(fName)),closeOnExit(true) {}
	virtual ~OutputStreamHolder();
	OutputStream &getStream(void) const {return *s;}
};

}; };

#endif /* defined(thisHeader) */
