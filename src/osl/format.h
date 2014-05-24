/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 7/21/2002
NAME:		osl/format.h

DESCRIPTION:	C++ File format library 

Captures the pattern used by raster image, movie, and AVI
"file format handlers"-- keep a static, readonly-after-registration
table (FormatTable) of const, abstract handlers (Format) around.
*/
#ifndef __OSL_FILE_FORMAT_H
#define __OSL_FILE_FORMAT_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif

namespace osl { namespace io {

/*
Format: an abstract description of a file format.
A FormatTable has to provide a subclass of this class
that actually has "read the file" or "return a file reader"
commands.
*/
class Format {
public:
	virtual ~Format();
	
	///Return a short human-readable description of this format
	///  e.g., "JPEG"
	virtual const char *getDesc(void) const =0;
	
	///Return a long human-readable description of this format
	///  e.g., "JPEG Image (ISO DIS 10918), by jpeglib-6b"
	virtual const char *getDescription(void) const =0;
	
	///Return a (null-terminated) machine-readable list of the 
	///  file types we can handle--filename extensions in lowercase.
	///  e.g., {"jpg","jpeg","jfif", NULL}
	virtual const char **getExtensions(void) const =0;
	
	///Return true if we can handle a stream with these initial bytes
	/// initLen is guaranteed to be at least 8
	virtual bool matchesInitial(const byte *initData,int initLen) const =0;
};

/*
FormatTable: a list of FormatHandlers.  Typically, there will be one
static instance of this class for each kind of file: Images, Movies, etc.
*/
class implFormatTable;
class FormatTable {
	implFormatTable *impl;
public:
	FormatTable();
	~FormatTable();
	
	///Register this handler with our table.
	///  The handler is now owned and will be deleted by us.
	void addFormat(Format *h);
	
	///Lookup the handler for this file
	/// if forRead is true, bases lookup on initial bytes of file (byInitial);
	/// if forRead is false, bases lookup on filename extension (byExtension).
	///These all throw a FormatException if the format is unrecognized
	const Format *byFileName(const char *fName,bool forRead) const;
	const Format *byInitial(const byte *initData,int initLen) const;
	const Format *byExtension(const char *lowercaseExt) const;
	
	///Iterate through registered handlers:
	int getFormats(void) const;
	const Format *getFormat(int i) const;
};

//As above, but only accepting some subclass of format:
template<class Fmt>
class FormatTableT : public FormatTable {
public:
	inline void addFormat(Fmt *h) 
	  {FormatTable::addFormat(h);}
	inline const Fmt *byFileName(const char *fName,bool forRead) const
	  {return (const Fmt *)FormatTable::byFileName(fName,forRead);}
	inline const Fmt *byInitial(const byte *initData,int initLen) const
	  {return (const Fmt *)FormatTable::byInitial(initData,initLen);}
	inline const Fmt *byExtension(const char *extension) const
	  {return (const Fmt *)FormatTable::byExtension(extension);}
	inline const Fmt *getFormat(int i) const
	  {return (const Fmt *)FormatTable::getFormat(i);}
};


}; }; /*end namespace osl::io*/
#endif /* def(thisHeader) */
