/**
	C++ Source file: represent a series of JPEG files as 
one big MJPEG stream separated by the following ASCII HTTP multipart 
separators:

--myboundary
Content-Length: 23300

This file does not require any of the rest of OSL, though if you 
include "osl/raster.h" before this file, you get OSL raster decoders.

Dr. Orion Sky Lawlor, olawlor@acm.org, 2008-10-06 (Public Domain)
*/
#ifndef __MJPEG_H
#define __MJPEG_H

#include <iostream>
#include <vector>
#include <string.h>

/* Like std::getline, but accepts any of several delimiters */
void getline_multi(std::istream &is,std::string &dest,const char *delims) 
{
	dest="";
	char c;
	while (is.read(&c,1)) {
		if (strchr(delims,c))
		{ /* hit a delimiter--end line */
			/* Skip over any other non-identical delimiters */
			while (1) {
				char n=is.peek(); /* Check the next character */
				if (!strchr(delims,n)) break; /* not a delimiter */
				if (n==c) break; /* same as before (e.g., duplicate newlines) */
				is.read(&n,1); /* consume the delimiter */
			}
			return;
		}
		dest+=c;
	}
}

/**
Takes an istream containing a series of (movie) images, encapsulated with:

... ascii crap ...
Content-Length: <decimal byte count>
... more ascii crap ...
<blank line>
<byte count binary data>

And splits out separate frames.
*/
class frame_splitter {
	std::istream &s;
public:
	frame_splitter(std::istream &is) :s(is)  {}
	
	/* Return the binary JPEG data of the next frame into this vector. 
	   Returns a zero-length vector on errors.
	*/
	void next_frame(std::vector<unsigned char> &jpeg_data) {
		int length=0;
		bool hit_content=false; /* have we hit the Content-Length field yet? */
		while (true) { /* Find the Content-Length field, ignore everything else until the blank line. */
			std::string line;
			getline_multi(s,line,"\r\n");
			//std::cout<<"Read line: '"<<line<<"'\n";
			if (!s) {jpeg_data.resize(0); return; /* I/O error */}
			if (line=="" && hit_content) break; /* end of header */
			if (0==line.find("Content-length:") ||
				0==line.find("Content-Length:"))
			{
				sscanf(line.c_str(),"%*s%d",&length);
				hit_content=true;
			}
		}
		jpeg_data.resize(length);
		s.read((char *)&jpeg_data[0],length);
		
		if (length<100) { /* Invalid length in HTTP header--read again */
			next_frame(jpeg_data);
			return;
		}
	}
}; 

/** Merge a series of JPEG frames into a C++ ostream, separated by boundaries.
So named because it emits (spits) a stream of frames.
*/
class frame_spitter {
	std::ostream &s;
public:
	frame_spitter(std::ostream &os) :s(os) {
		s<<	"HTTP/1.0 200 OK\r\n"
			"Content-Type: multipart/x-mixed-replace; boundary=--myboundary\r\n"
			"\r\n";
	}
	
	void next_frame(const void *src,int len) {
		time_t now=time(0); struct tm t;
		localtime_r(&now,&t);
		s<<	"--myboundary\r\n"
			"Content-Type: image/jpeg\r\n"
			"Encoded-UTC: "<<now<<"\r\n"
			"Encoded-ctime: "<<1900+t.tm_year<<"-"<<1+t.tm_mon<<"-"<<t.tm_mday<<" "<<t.tm_hour<<":"<<t.tm_min<<":"<<t.tm_sec<<"\r\n"
			"Content-Length: "<<len<<"\r\n"
			"\r\n";
		s.write((char *)src,len);
		s.flush(); /* don't delay for I/O buffers! */
	}
};

/** Return the first byte of this array that contains actual JPEG JFIF data */
int find_jpeg_start(const unsigned char *data,int len) {
	for (int i=0;i+3<len;i++)
		if (data[i]==0xff && data[i+1]==0xd8 && data[i+2]==0xff && (data[i+3]==0xdb||data[i+3]==0xe0))
			return i; /* SOI marker */
	return 0; /* nothing found--just write all the data */
}

#ifdef __OSL_RASTER_H

/* Turn a vector of JPEG'd bytes into a decoded image */
void decode_jpeg(const std::vector<unsigned char> &data, osl::graphics2d::RgbaRaster &dest)
{
	int gooddata=find_jpeg_start(&data[0],data.size());
	osl::io::ByteArrayInputStream is(&data[gooddata],data.size()-gooddata);
	osl::graphics2d::RasterInputStream ri(
		is,osl::graphics2d::RasterFormat("jpeg")
	);
	dest.read(ri);
}
/** Read this rgbaraster from this stream. 
    Throws osl::Exception on error or EOF. */
void next_frame(frame_splitter &fs, osl::graphics2d::RgbaRaster &dest) 
{
	std::vector<unsigned char> data;
	fs.next_frame(data);
	decode_jpeg(data,dest);
}
/** Read and return a rgbaraster from this stream. 
    Throws osl::Exception on error or EOF. */
osl::graphics2d::RgbaRaster next_frame(frame_splitter &fs) 
{
	std::vector<unsigned char> data;
	fs.next_frame(data);
	osl::graphics2d::RgbaRaster dest;
	decode_jpeg(data,dest);
	return dest;
}

/* Adapt std::vector to OutputStream interface */
template <class byte>
class VectorOutputStream : public osl::io::OutputStream {
	std::vector<byte> &out;
public:
	VectorOutputStream(std::vector<byte> &out_) :out(out_) {}
	virtual void write(const void *src,int nBytes) {
		byte *bs=(byte *)src; int bn=nBytes/sizeof(byte);
		for (int i=0;i<bn;i++) out.push_back(bs[i]);
	}
	void seek(osl::int64 to) {out.resize(to);}
	osl::int64 tell(void) {return out.size();}
	const char *getDescription(void) {return "vector output stream";}
};

/* Turn an image into a vector of JPEG'd bytes */
void encode_jpeg(const osl::graphics2d::RgbaRaster &src,std::vector<unsigned char> &dest)
{
	dest.resize(0);
	VectorOutputStream<unsigned char> os(dest);
	osl::graphics2d::RasterOutputStream ri(
		os,osl::graphics2d::RasterFormat("jpeg")
	);
	src.write(ri);
}

/** Read and return a rgbaraster from this stream. 
    Throws osl::Exception on error or EOF. */
void next_frame(frame_spitter &fs,const osl::graphics2d::RgbaRaster &data) 
{
	std::vector<unsigned char> out;
	encode_jpeg(data,out);
	fs.next_frame(&out[0],out.size());
}

#endif


#endif
