/*
Orion's Standard Library
Orion Sky Lawlor, 6/30/2002
NAME:		osl/riff.h

DESCRIPTION:	C++ RIFF (Resource Interchange File Format) library 

This file provides routines for creating, reading in,
writing out, and manipulating RIFF files: for example,
WAV sound files or Microsoft AVI movies.
*/
#ifndef __OSL_RIFF_H
#define __OSL_RIFF_H

#ifndef __OSL_IO_H
#  include "osl/io.h"
#endif
#include <vector>

namespace osl { namespace riff {

using osl::io::InputStream;
using osl::io::BufferedInputStream;

//A "Tag" is a 4-byte block ID; normally ASCII
class Tag {
	char c[4];
public:
	Tag() {}
	Tag(char c0,char c1,char c2,char c3) {
		c[0]=c0; c[1]=c1; c[2]=c2; c[3]=c3;
	}
	Tag(const char *src) {
		c[0]=src[0]; c[1]=src[1]; c[2]=src[2]; c[3]=src[3];
	}
	/*default copy constructor, assignment operator*/
	
	char operator[](int i) const {return c[i];}
	
	//This truly sucks: return-reference-to-static-buffer
	const char *getStr(void) const {
		static char r[5];
		r[0]=c[0]; r[1]=c[1]; r[2]=c[2]; r[3]=c[3]; r[4]=0;
		return r;
	}
	
	bool operator==(const Tag &t) const {
		return c[0]==t.c[0] && c[1]==t.c[1] && c[2]==t.c[2] && c[3]==t.c[3];
	}
	bool operator!=(const Tag &t) const {return !((*this)==t);}
	bool operator==(const char *t) const {return (*this)==Tag(t);}
	bool operator!=(const char *t) const {return (*this)!=Tag(t);}
};

typedef unsigned int Len;

//This is the header on every RIFF chunk (used for writing only)
struct Header {
	Tag tag;
	osl::io::Lil32 len;
	
	Header() {}
	Header(const Tag &tag_,Len len_) :tag(tag_), len(len_) {}
};
struct FileHeader : public Header {
	Tag sub;
	FileHeader(const Tag &sub_) :Header("RIFF",(Len)-1), sub(sub_) {}
};
struct ListHeader : public Header {
	Tag sub;
	ListHeader(const Tag &sub_,Len len_=(Len)-1) 
		:Header("LIST",len_), sub(sub_) {}
};

template <class T> 
class ChunkT : public Header {
public:
	T v; //Value of chunk
	ChunkT(const Tag &tag_)
		:Header(tag_,sizeof(T)) {}
};

template <class T> 
class ListT : public ListHeader {
public:
	T v; //Value of list
	ListT(const Tag &tag_)
		:ListHeader(tag_,sizeof(T)+sizeof(Tag)) {}
};




/**
 * You inherit from this class to get RIFF tags out.
 *   beginList is called when a new RIFF "LIST" is encountered,
 *  and endList is called when the list is finished.
 *   Each RIFF tag results in a call to "chunk".
 */
class Dest {
public:
	virtual ~Dest();
	virtual void beginList(Tag listTag,Len bodyLength);
	/**
	 Read the RIFF tag with this tag and length.
	
	 You *MUST* read exactly bodyLength bytes from "data"
	 (even if you don't care about them!), or you'll immediately
	 get a "chunk exceeds its list" exception from the RIFF Parser.
	 You probably want a big "else data.skip(bodyLength);".
	*/
	virtual void chunk(Tag tag,Len bodyLength,InputStream &data) =0;
	virtual void endList(Tag listTag);
};

//RIFF Parser: reads one RIFF tag at a time
class Parser {
	BufferedInputStream is;
	Tag fileTag;
	
	class List {
		Tag tag;
		Len len; //Bytes remaining in list
	public:
		List(const Tag &t,Len l) :tag(t), len(l) {}
		List() :tag("none"), len(0) {}
		Tag getTag(void) const {return tag;}
		inline void consume(Len nBytes) {
			if (nBytes>len) {
				OSL_THROW(FormatException,"RIFF file: chunk exceeds its list");
			}
			else len-=nBytes;
		}
		bool isDone(void) const {return len==0;}
	};
	List curList;
	std::vector<List> listStack;
	
	Tag readTag(void) {
		Tag r;
		is.read(&r,sizeof(r));
		return r;
	}
	Len readLen(void) {
		osl::io::Lil32 l;
		is.read(&l,sizeof(l));
		//if (l>1024*1024) bad("FIXME: Overconsumption!");
		return (Len)l;
	}
public:
	Parser(InputStream &is_);
	Tag getFileTag(void) {return fileTag;}
	
	//Parse the next item from the file
	bool next(Dest &d);
};


}; }; /*end namespace osl::riff*/
#endif /* def(thisHeader) */
