/*
Orion's Standard Library
Orion Sky Lawlor, 2/16/2002
NAME:		osl/movie.h

DESCRIPTION:	C++ moving image library 

This file provides routines for creating, reading in,
writing out, and manipulating movies-- sequences of images.
*/
#ifndef __OSL_MOVIE_H
#define __OSL_MOVIE_H

#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif
#ifndef __OSL_IO_H
#include "osl/io.h"
#endif
#include "osl/format.h"

namespace osl { namespace graphics2d {
using osl::io::InputStream;
using osl::io::InputStreamHolder;
using osl::io::OutputStream;
using osl::io::OutputStreamHolder;
using osl::io::Format;

class AbstractMovieInput;
class AbstractMovieOutput;
class MovieInfo;

class MovieFormat : public Format {
public:
	virtual AbstractMovieInput *newMovieInput(InputStream &is) const =0;
	virtual AbstractMovieOutput *newMovieOutput(OutputStream &is,const MovieInfo &info) const =0;
};

class MovieTable : public osl::io::FormatTableT<MovieFormat> {
public:
	MovieTable();
};
extern MovieTable movieFormats;
MovieFormat *makeAviMovieFormat(void);
MovieFormat *makeMpegMovieFormat(void);

class MovieInputStream : public InputStreamHolder {
	const MovieFormat *format;
public:
	MovieInputStream(const char *fName);
	MovieInputStream(InputStream &sref_,const MovieFormat *fmt)
		:InputStreamHolder(sref_), format(fmt) {}
	
	inline const MovieFormat *getFormat(void) const 
		{return format;}
};
class MovieOutputStream : public OutputStreamHolder {
	const MovieFormat *format;
public:
	MovieOutputStream(const char *fName);
	MovieOutputStream(OutputStream &sref_,const MovieFormat *fmt)
		:OutputStreamHolder(sref_), format(fmt) {}
	
	//Return the image format, extracted from the file name if needed:
	inline const MovieFormat *getFormat(void) const  
		{return format;}
};

/// Describes a movie, used when creating or opening a movie file.
class MovieInfo {
public:
	double quality; // Compression quality: 1.0 is lossless; 0.0 is crap
	Point size; //Pixel size of video frames
	double timePerFrame; //Seconds per video frame of movie
	int nFrames; //Number of video frames in movie, or -1 if unknown
	bool hasAudio;
	
	MovieInfo() :quality(1.0), size(0,0), 
		timePerFrame(0.0), nFrames(-1), hasAudio(false) {}
	MovieInfo(Point size_,double tpf=1.0/30.0,int nFrames_=-1,bool hasAudio_=false)
		:quality(1.0), size(size_), timePerFrame(tpf), nFrames(nFrames_), hasAudio(hasAudio_) {}
	//Default copy constructor, assignment operator
	
	///Get pixel size of video frames
	const Point &getSize(void) const {return size;}
	
	///Get seconds per video frame of movie
	double getTimePerFrame(void) const {return timePerFrame;}
	
	///Get number of video frames in movie, or -1 if unknown
	int getFrames(void) const {return nFrames;}
	
	///Return true if the movie includes audio
	bool getAudio(void) const {return hasAudio;}
};

/// Describes one frame of the movie.
class FrameInfo {
public:
	double time; ///< Seconds after start of movie to *begin* displaying frame
	enum {
		flag_progressive=0, ///< Progressive (simple top-down) frame
		flag_even=1, ///< Interlaced frame: even-numbered lines
		flag_odd=2, ///< Interlaced frame: odd-numbered lines
		flag_evenodd=3 ///< Interlaced frame with mixed even and odd lines.
	} flags;
	FrameInfo(double time_) :time(time_), flags(flag_progressive) {}
};

/********************************************
An input movie is a source of frame-images.
These are decoded one frame at a time, even if
internally some formats (such as MPEG) do 
frame reordering.
*/
class AbstractMovieInput {
public:
	virtual ~AbstractMovieInput();
	
	virtual const MovieInfo &getInfo(void) =0;
	
	//Get the next frame in the movie
	virtual void nextFrame(Raster &dest,FrameInfo *pf=NULL) =0;
};

/**
 Destination for images to be encoded.
*/
class AbstractMovieOutput {
public:
	virtual ~AbstractMovieOutput();
	
	//Write the next frame in the movie
	virtual void nextFrame(const Raster &src,const FrameInfo &f) =0;
};


/**
 A source of frame-images from some serialized file format.
*/
class MovieInput : public AbstractMovieInput {
	AbstractMovieInput *mov; //Real implementation
	MovieInputStream *s; //File stream
	FrameInfo f;
public:
	MovieInput(MovieInputStream *s_);
	virtual ~MovieInput();
	
	virtual const MovieInfo &getInfo(void) {return mov->getInfo();}
	
	//Get the next frame in the movie
	// May throw any I/O exception, or FormatException
	virtual void nextFrame(Raster &dest,FrameInfo *pf=NULL) 
		{ mov->nextFrame(dest,pf?pf:&f); }
	inline const FrameInfo &getFrame(void) const {return f;}
};

/**
  Destination for images to be encoded to some serialized file format.
*/
class MovieOutput : public AbstractMovieOutput {
	AbstractMovieOutput *mov; //Real implementation
	MovieOutputStream *s; //Source stream
public:
	MovieOutput(MovieOutputStream *s_,const MovieInfo &info);
	virtual ~MovieOutput();
	
	//Write the next frame in the movie
	virtual void nextFrame(const Raster &src,const FrameInfo &f) {return mov->nextFrame(src,f);}
};


/********* Movie Capture Interface **********/
class MovieCaptureOptions {
public:
	int device; /* video device number (0 for first device) */
	Point size; /* video capture size (0==default) */
	MovieCaptureOptions() :device(0),size(0,0) {}
};
/* Create a new video capture device */
AbstractMovieInput *makeMovieCapture(const MovieCaptureOptions &o,const char *deviceName=0);


}; }; //end namespace osl::graphics2d
#endif //__OSL_RASTER_H

