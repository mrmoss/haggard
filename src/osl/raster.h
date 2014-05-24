/*
Orion's Standard Library
Orion Sky Lawlor, 12/22/2000
NAME:		osl/raster.h

DESCRIPTION:	C++ Raster image library 

This file provides routines for creating, reading in,
writing out, and manipulating raster images of any number of
channels (i.e. Colors) and channel data types-- the 
ubiquitous unsigned char, but also shorts, ints, or floats.

The basic contract for a Raster subclass is to provide 
getColor and setColor routines.  The rest can be left as
defaults or optimized for speed, as needs dictate.
*/
#ifndef __OSL_RASTER_H
#define __OSL_RASTER_H

#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif
#ifndef __OSL_RASTERIZER_UTIL_H
#include "osl/rasterizer_util.h"
#endif
#ifndef __OSL_PIXEL_H
#include "osl/pixel.h"
#endif
#ifndef __OSL_IO_H
#include "osl/io.h"
#endif

namespace osl { namespace graphics2d {

class RasterFormatException : public FormatException {
public:
	OSL_EXCEPTION_DECL(RasterFormatException,FormatException)
};

class AbstractRasterFormat;
class RasterFormat {
	const AbstractRasterFormat *fmt; //Cached internal format object
public:
	//Look up the format bound to this name.
	//  Throws a FormatException if that format is unknown.
	RasterFormat(const char *formatName);
	
	//Look up the format matching these initial values of the image
	//  Throws a FormatException if that format is unknown.
	RasterFormat(const byte *initData,int initLen);
	
	//Look up based on this file name
	static RasterFormat byFileName(const char *fName,bool forRead=false);
	
	//Internal constructor and methods
	RasterFormat(const AbstractRasterFormat *fmt_) :fmt(fmt_) {}
	const AbstractRasterFormat *getImplFormat(void) const {return fmt;}
};

class RasterInputStream : public osl::io::InputStreamHolder {
	RasterFormat format;
public:
	RasterInputStream(const char *fName);
	RasterInputStream(osl::io::InputStream &sref_,RasterFormat fmt)
		:InputStreamHolder(sref_), format(fmt) {}
	
	const RasterFormat &getFormat(void) const {return format;}
};
class RasterOutputStream : public osl::io::OutputStreamHolder {
	RasterFormat format;
public:
	RasterOutputStream(const char *fName);
	RasterOutputStream(osl::io::OutputStream &sref_,RasterFormat fmt)
		:OutputStreamHolder(sref_), format(fmt) {}
	
	//Return the image format, extracted from the file name if needed:
	const RasterFormat &getFormat(void) const  {return format;}
};

/******************* Abstract Raster superclass *********************/
//An interface to a 2D Rectangular array of Pixels
class Raster : public Noncopyable {
public:
	////////////////////////////////////////////
	enum { //Properties of a Raster
	COLOR_MASK=0x000000ffu, //Describes Pixel channel (Color) resolution
	COLOR_1bit=1u, //1-bit (b/w) resolution
	COLOR_4bit=4u, //4-bit (16 shades) resolution
	COLOR_5bit=5u, //5-bit (32 shades) resolution
	COLOR_8bit=8u, //8-bit (256 shades) resolution
	COLOR_10bit=10u, //10-bit (1024 shades) resolution
	COLOR_16bit=16u, //16-bit (65,536) resolution
	COLOR_23bit=23u, //23-bit (8 million) resolution
	
	LAYOUT_MASK=0x00000f00u, //Describes Pixel layout in memory [exclusive]
	CONTIGUOUS_ROWS=1u<<8, //Image is formed from regular rows of Pixels
	PIXELS_PAGED=1u<<9, //Image is formed from non-row groups of Pixels
	
	CHANNEL_MASK=0x0000f000u, //Describes channels in Pixel
	HAS_ALPHA=1u<<12, //Alpha channel not always 1.0
	HAS_RGB=1u<<13 //R,G,B meaningful (i.e., not grayscale, alpha, ...)
	};
	//Default returns COLOR_8bit|CONTIGUOUS_ROWS|HAS_RGB
	virtual unsigned int getProperties(void) const;
	unsigned int getColorDepth(void) const 
	{return getProperties()&COLOR_MASK;}
	bool hasRgb(void) const {return 0!=(getProperties()&HAS_RGB);}
	bool hasAlpha(void) const {return 0!=(getProperties()&HAS_ALPHA);}

	Rect getRect(void) const {return Rect(0,0,wid,ht);}
	
	//////////////////////////////////////////////////////
	
	//Copy this Pixel-aligned Raster onto yourself 
	// in the given Region. src(0,0) -> this(ox,oy)
	virtual void alignedCopy(const GraphicsState &s,
			const osl::ru::Region &where,
			int ox,int oy,const Raster &src);
	
	//Fill this shape with the current Color.
	virtual void fill(const GraphicsState &s,
			const osl::ru::Region &where);
	
	//Copy this "Pixel source" onto yourself in the given Region
	virtual void copy(const osl::ru::Region &where,
			const LineSource &from);
	
	virtual LinearPixelSource *getLinearSource(const GraphicsState &s) const;
	
	//////////////////////////////////////////////////////
	int wid,ht;//Size of publically-accessible Region
	Raster() :wid(-1),ht(-1) {}
	Raster(int Nwid,int Nht) :wid(Nwid),ht(Nht) {}
	Raster(const char *fileName) {readNoThrow(fileName);}
	virtual ~Raster();
	virtual void reallocate(int Nwid,int Nht);
	
	//Single-Pixel interface (must be in-bounds)
	virtual Color getColor(int x,int y) const =0;
	virtual void setColor(int x,int y,const Color &c) =0;
	
	//The same as setColor(x,y,getColor(x,y).blend(c))
	virtual void blendColor(int x,int y,const Color &c);
	
	/// Sampling interface (must be strongly in-bounds).
	/// Treats (x+0.5,y+0.5) as non-interpolated center of pixel (x,y).
	virtual Color getBilinear(float x,float y) const;
	
	/// Need not be in-bounds (wraps around cyclically)
	virtual Color getColorWrap(int x,int y) const;
	virtual Color getBilinearWrap(float x,float y) const;
	
	///Need not be in-bounds (pinned to nearest Pixel)
	virtual Color getColorPin(int x,int y) const;
	virtual Color getBilinearPin(float x,float y) const;
	
	//File I/O (format guessed from extention-- .jpg, .ppm, ...)
	// These may throw exceptions:
	virtual void read(const RasterInputStream &file);
	virtual void write(const RasterOutputStream &file) const;

	// These will print an error to stdout and exit on error:
	virtual void readNoThrow(const RasterInputStream &file);
	virtual void readNoThrow(const char *fileName);
	virtual void writeNoThrow(const RasterOutputStream &file) const;

	
	//Row interface-- buffer is [r,g,b...], [r,g,b,a...] or [a;a;a; ...]
	// Row must be in-bounds, and is from (x1,y) to (x2-1,y)
	//Basic access routines
	virtual void getRgbRow(int y,int x1,int x2,byte *dest) const;
	virtual void getRgbaRow(int y,int x1,int x2,byte *dest) const;
	virtual void setRgbRow(int y,int x1,int x2,const byte *src);
	virtual void setRgbaRow(int y,int x1,int x2,const byte *src);
	
	virtual void getGrayRow(int y,int x1,int x2,byte *dest) const;
	virtual void setGrayRow(int y,int x1,int x2,const byte *src);
	
	virtual void getBgrRow(int y,int x1,int x2,byte *dest) const;
	virtual void getBgraRow(int y,int x1,int x2,byte *dest) const;
	virtual void setBgrRow(int y,int x1,int x2,const byte *src);
	virtual void setBgraRow(int y,int x1,int x2,const byte *src);
	
	//Clipping utility-- clips x start and end values;
	// returns true if line has any in-bounds left
	bool clipRow(int y,int &x1,int &x2) const {
		if (y<0 || y>=ht) return false;
		if (x1<0) x1=0;
		if (x2>wid) x2=wid;
		if (x1>=x2) return false;
		return true;
	}

	//Fill us entirely with this color
	virtual void clear(const Color &c);
};

inline Rect::Rect(const Raster &r) 
	:left(0),top(0),right(r.wid),bottom(r.ht) { }

/******************** Simple Raster types *********************/
/**
 A PixelBufferMgr manages the actual Raster data for most Raster types.
 Separating the data source from the Raster class itself allows:
    - Reference-counted buffer sharing, for cheap Raster copies.
    - Multiple Raster "windows" on the same data.
 Most managers *MUST* be allocated dynamically using "new", because
 the last "unref" will use "delete" to dispose of the manager.
*/
class PixelBufferMgr : public Noncopyable {
protected:
	void *data;
	int refCount,rowPixels;
protected:
	/* Only we can delete ourselves */
	virtual ~PixelBufferMgr();
public:
	/**
	  Create a manager to handle this data and row size.
	    \param Ndata Pixel data to manage.
	    \param NrowSize Number of pixels per row of the image.
	*/
	PixelBufferMgr(void *Ndata,int NrowPixels) {
		data=Ndata;
		refCount=0;rowPixels=NrowPixels;
	}
	
	/// Pixels per image line (i.e., line/line Pixel offset)
	int getRowSize(void) {return rowPixels;}
	
	/// Add a reference to this manager.  Must eventually
	///   call the matching unref.
	void *ref(void) {
		refCount++;
		return data;
	}
	/// Release reference to this manager.  Deletes 
	///   the manager when the reference count hits zero.
	void unref(void) {
		if (0>=--refCount)
			delete this; //<- we *must* be heap-allocated!
	}
};

/**
  Dynamically allocates (using malloc) the pixel buffer.
  This is by far the most common kind of PixelBufferMgr.
  It *MUST* be allocated dynamically using "new", because
  the last "unref" will use "delete" to dispose of this object.
*/
class MallocPixelBufferMgr :public PixelBufferMgr {
public:
	MallocPixelBufferMgr(int rowPixels,int ht,int bytesPerPixel);
	~MallocPixelBufferMgr();
private:
	int totalBytes;
};

/**
  Points pixel buffer to some (external) data source
  that isn't created or deleted.  Essentially ignores ref/unref.
  This is the *only* type of PixelBufferMgr that can 
  be safely allocated on the stack or as a member.
*/
class FixedPixelBufferMgr :public PixelBufferMgr {
public:
	FixedPixelBufferMgr(void *data,int Nrow);
};

//A FlatRaster is a Raster implemented as a simple, contiguous
// frame buffer.  (Raster implementation utility only)
class FlatRaster:public Raster {
	void operator=(const FlatRaster &r);//Don't assign raw FlatRasters
protected:
	PixelBufferMgr *mgr;//Storage manager
	void *data;//Image data start (Pixel 0,0)
	int row;//Pixels per row of the image
	
	void setMgr(PixelBufferMgr *Nmgr) 
	  {mgr=Nmgr; if (mgr) {data=mgr->ref();row=mgr->getRowSize();} }
	void freeMgr(void) {if (mgr) {mgr->unref(); mgr=NULL;}}
	virtual int bytesPerPixel(void) const =0;
	
	//Uninitialized Raster
	FlatRaster();
	//Make a Raster of the given size
	FlatRaster(int Nwid,int Nht,int bytesPerPixel);
	//Make a Raster from the given manager
	FlatRaster(int Nwid,int Nht,PixelBufferMgr *Nmgr);
	//Make a (wid x ht) subimage of the given parent (starting at x,y)
	// Shallow copy only-- this and parent refer to same data
	FlatRaster(int Nwid,int Nht,FlatRaster &parent,int x,int y,int bpp);
	//Shallow copy constructor
	FlatRaster(const FlatRaster &parent);
	//Read-from-file constructor
	FlatRaster(const char *fileName) {readNoThrow(fileName);}
public:	
	virtual ~FlatRaster(); //Calls mgr->unref()

	//Make this image wid x ht, destroying any old image
	virtual void reallocate(int Nwid,int Nht);
	
	/// For direct access to the Raster data
	void *getRawData(void) {return data;}
	const void *getRawData(void) const {return data;}
	
	/// Return the number of pixels per image row (always >= width)
	int getRowSize(void) const {return row;}
};

//Thin type layer over FlatRaster
template <class pix>
class FlatRasterT:public FlatRaster {
public:
	//Uninitialized Raster
	FlatRasterT() 
		: FlatRaster() {}
	//Make a Raster of the given size
	FlatRasterT(int Nwid,int Nht)
		: FlatRaster(Nwid,Nht,sizeof(pix)) {}
	//Make a Raster from the given manager
	FlatRasterT(int Nwid,int Nht,PixelBufferMgr *Nmgr)
		: FlatRaster(Nwid,Nht,Nmgr) {}
	//Make a (wid x ht) subimage of the given parent (starting at x,y)
	// Shallow copy only-- this and parent refer to same data
	FlatRasterT(int Nwid,int Nht,FlatRasterT<pix> &parent,int x,int y)
		: FlatRaster(Nwid,Nht,parent,x,y,sizeof(pix)) {}
	//Shallow copy constructor
	FlatRasterT(const FlatRasterT<pix> &parent)
		: FlatRaster(parent) {}
	//Read-from-file constructor
	FlatRasterT(const char *fileName) {readNoThrow(fileName);}
	
	// Assignment operator: makes a *shallow* ref-counted copy!
	FlatRasterT<pix> &operator=(const FlatRasterT<pix> &src) {
		freeMgr(); setMgr(src.mgr); return *this;
	}
	
	virtual int bytesPerPixel(void) const {return sizeof(pix);}

	/// This is the datatype of one of our pixels.
	typedef pix pixel_t;	

	//Direct Pixel access routines (non-virtual, for speed)
	pix &at(int x,int y) {return getPixels()[y*row+x];}
	const pix &at(int x,int y) const {return getPixels()[y*row+x];}
	pix &operator()(int x,int y) {return at(x,y);}
	const pix &operator()(int x,int y) const {return at(x,y);}	
	
	//For direct access to the Raster data
	pix *getPixels(void) {return (pix *)data;}
	const pix *getPixels(void) const {return (const pix *)data;}
	int getRowSize(void) const {return row;} //Pixels per image row

	//Fill us entirely with this color
	inline void clear(const pix &p) {
		for (int y=0;y<ht;y++)
		for (int x=0;x<wid;x++)
			at(x,y)=p;
	}
};

#define FlatRasterConstructors(me) \
	me() : super() {}\
	me(int Nwid,int Nht) : super(Nwid,Nht) {}\
	me(int Nwid,int Nht,PixelBufferMgr *Nmgr) : super(Nwid,Nht,Nmgr) {}\
	me(int Nwid,int Nht,me &from,int x,int y)\
		: super(Nwid,Nht,from,x,y) {}\
	me(const me &from) : super(from) {}\
	me(const char *fileName) {this->readNoThrow(fileName);}

//FlatRaster for when Pixel type has get/set Color and byte routines
template <class pix>
class FlatRasterPixT : public FlatRasterT<pix> {
	typedef FlatRasterT<pix> super;
public:
	FlatRasterConstructors(FlatRasterPixT<pix>)
	
	//Single-Pixel interface (must be in-bounds)
	virtual Color getColor(int x,int y) const {
		return this->at(x,y);
	}
	virtual void setColor(int x,int y,const Color &c) {
		this->at(x,y)=c;
	}
	// Row must be in-bounds, and is from (x1,y) to (x2-1,y)
#define OSL_FLATRASTER_GET_FOR \
	const pix *src=&this->at(x1,y); \
	for (int x=x1;x<x2;x++) 
#define OSL_FLATRASTER_SET_FOR \
	pix *dest=&this->at(x1,y); \
	for (int x=x1;x<x2;x++) 
	virtual void getRgbRow(int y,int x1,int x2,byte *dest) const {
		OSL_FLATRASTER_GET_FOR {
			src->getRgb(dest); 
			src++;dest+=3;
		}
	}
	virtual void getRgbaRow(int y,int x1,int x2,byte *dest) const {
		OSL_FLATRASTER_GET_FOR {
			src->getRgba(dest); 
			src++;dest+=4;
		}
	}
	virtual void getBgrRow(int y,int x1,int x2,byte *dest) const {
		OSL_FLATRASTER_GET_FOR {
			src->getBgr(dest); 
			src++;dest+=3;
		}
	}
	virtual void getBgraRow(int y,int x1,int x2,byte *dest) const {
		OSL_FLATRASTER_GET_FOR {
			src->getBgra(dest); 
			src++;dest+=4;
		}
	}
	virtual void getGrayRow(int y,int x1,int x2,byte *dest) const {
		OSL_FLATRASTER_GET_FOR {
			src->getGray(dest); 
			src++;dest++;
		}
	}

	virtual void setRgbRow(int y,int x1,int x2,const byte *src) {
		OSL_FLATRASTER_SET_FOR {
			dest->setRgb(src); 
			src+=3;dest++;
		}
	}
	virtual void setRgbaRow(int y,int x1,int x2,const byte *src) {
		OSL_FLATRASTER_SET_FOR {
			dest->setRgba(src); 
			src+=4;dest++;
		}
	}
	virtual void setBgrRow(int y,int x1,int x2,const byte *src) {
		OSL_FLATRASTER_SET_FOR {
			dest->setBgr(src); 
			src+=3;dest++;
		}
	}
	virtual void setBgraRow(int y,int x1,int x2,const byte *src) {
		OSL_FLATRASTER_SET_FOR {
			dest->setBgra(src); 
			src+=4;dest++;
		}
	}
	virtual void setGrayRow(int y,int x1,int x2,const byte *src) {
		OSL_FLATRASTER_SET_FOR {
			dest->setGray(src); 
			src++;dest++;
		}
	}
};


//An array of Color objects (128-bit Color!)
class ColorRaster:public FlatRasterPixT<Color> {
	typedef FlatRasterPixT<Color> super;
public:
	FlatRasterConstructors(ColorRaster)

	virtual unsigned int getProperties(void) const;
};

//Describes an array of RgbaPixel's.  This is the most common kind of Raster.
class RgbaRaster:public FlatRasterPixT<RgbaPixel> {
	typedef FlatRasterPixT<RgbaPixel> super;
public:
	FlatRasterConstructors(RgbaRaster)

	virtual unsigned int getProperties(void) const;
	
	//Fetch a bilinearly-interpolated Sample from x,y
	// Wraps around boundaries if out of bounds.  
	// IMAGE SIZE (wid and ht) MUST BOTH BE POWERS OF TWO
	// Coordinates are .8 fixed-point.
	RgbaPixel fix8BilinearWrap(int x,int y) const;
	
	//Fetch a bilinearly-interpolated Sample from x,y
	// Pins to boundaries if out of bounds.  
	// Coordinates are .8 fixed-point.
	RgbaPixel fix8BilinearPin(int x,int y) const;
	
	virtual Color getBilinearWrap(float x,float y) const;
	virtual Color getBilinearPin(float x,float y) const;
	
	virtual LinearPixelSource *getLinearSource(const GraphicsState &s) const;

#define OSL_SPECIALIZED_RGBA_RASTER 1
#if OSL_SPECIALIZED_RGBA_RASTER
	//Copy this Pixel-aligned Raster onto yourself 
	// in the given Region. src(0,0) -> this(ox,oy)
	virtual void alignedCopy(const GraphicsState &s,
			const osl::ru::Region &where,
			int ox,int oy,const Raster &src);
	
	//Copy this "Pixel source" onto yourself in the given Region
	virtual void copy(const osl::ru::Region &where,
			const LineSource &from);
	
	//Fill this shape with the current Color.
	virtual void fill(const GraphicsState &s,
			const osl::ru::Region &where);
	
	virtual void clear(const Color &c);
#endif
};
//Describes an alpha-free array of RgbaPixel's.
class RgbRaster:public RgbaRaster {
	typedef RgbaRaster super;
public:
	FlatRasterConstructors(RgbRaster)
	
	virtual unsigned int getProperties(void) const;
};

//This class describes an alpha channel
class AlphaRaster:public FlatRasterT<byte> {
	typedef FlatRasterT<byte> super;
public:
	FlatRasterConstructors(AlphaRaster)
	
	virtual unsigned int getProperties(void) const;
	
	//Single-Pixel interface (must be in-bounds)
	virtual Color getColor(int x,int y) const;
	virtual void setColor(int x,int y,const Color &c);

	virtual void getGrayRow(int y,int x1,int x2,byte *dest) const;
	virtual void setGrayRow(int y,int x1,int x2,const byte *src);
	
	//Fetch a bilinearly-interpolated Sample from x,y
	// Wraps around boundaries if out of bounds.  
	// IMAGE SIZE (wid and ht) MUST BOTH BE POWERS OF TWO
	// Coordinates are .8 fixed-point.
	byte fix8BilinearWrap(int x,int y) const;
	
	//Fetch a bilinearly-interpolated Sample from x,y
	// Pins to boundaries if out of bounds.  
	// Coordinates are .8 fixed-point.
	byte fix8BilinearPin(int x,int y) const;
	
	virtual Color getBilinearWrap(float x,float y) const;
	virtual Color getBilinearPin(float x,float y) const;
	
	virtual LinearPixelSource *getLinearSource(const GraphicsState &s) const;
};

//This class describes a greyscale floating-Point image
class FloatRaster:public FlatRasterT<float> {
	typedef FlatRasterT<float> super;
	class params {
	public:
		double min,max; //Displayed range
		double float2color_s,float2color_o;
		double color2float_s,color2float_o;
		bool doClip;
		void setMinMax(double min_,double max_);
		params() {
			setMinMax(0.0,1.0);
			doClip=true;
		}
	};
	params p;
public:
	FlatRasterConstructors(FloatRaster)
	
	void setMin(double minVal) {
		p.setMinMax(minVal,p.max);
	}
	void setMax(double maxVal) {
		p.setMinMax(p.min,maxVal);
	}
	void setMinMax(double minVal,double maxVal) {
		p.setMinMax(minVal,maxVal);
	}
	void setClip(bool withClip) {p.doClip=withClip;}
	
	virtual unsigned int getProperties(void) const;
	
	//Single-Pixel interface (must be in-bounds)
	virtual Color getColor(int x,int y) const;
	virtual void setColor(int x,int y,const Color &c);
};

//Has a single bit/pixel-- a black and white image
// 0 is white; 1 is black (the Mac/Postscript convention)
class BitmapRaster : public Raster {
public:
	typedef unsigned int block_t; //A portion of a row of Pixels
	enum {
		blackBlock=(block_t)(~0), //A whole block of black Pixels
		whiteBlock=(block_t)(0), //A whole block of white Pixels
		invertBlock=(block_t)(~0), //A whole block of 1 bits
		bitsPer=sizeof(block_t)*8,
		bitsMask=bitsPer-1,
		blockMask=((block_t)(~0))^bitsMask,
		blockShift=5 //Log2 of bitsPer. Valid only for 32-bit systems.
	};
protected:
	block_t *data;
public:
	int rowBlocks;//Blocks in a row
	BitmapRaster() :Raster() {data=NULL;}
	BitmapRaster(int Nwid,int Nht) :Raster() {data=NULL;reallocate(Nwid,Nht);}
	virtual ~BitmapRaster();
	virtual void reallocate(int Nwid,int Nht);
	
	virtual unsigned int getProperties(void) const;
	
	//Single-Pixel interface (must be in-bounds)
	virtual Color getColor(int x,int y) const;
	virtual void setColor(int x,int y,const Color &c);
	
	//Set the entire image to block_t 
	void set(block_t to);
	//Xor the entire image by block_t
	void blockXor(block_t by);
	
	//Pixel-block interface
	block_t &blockAt(int x,int y) 
		{return data[y*rowBlocks+(x>>blockShift)];}
	const block_t &blockAt(int x,int y) const 
		{return data[y*rowBlocks+(x>>blockShift)];}
	block_t &block(int b,int y) 
		{return data[y*rowBlocks+b];}
	const block_t &block(int b,int y) const 
		{return data[y*rowBlocks+b];}
};

//A shrinking image filter
class MiniRaster : public Raster 
{
	Raster &dest;
	int fac;//Minification factor
	double facInv;//1.0/(fac*fac)
	int accumWid;
	Color *accum;//Accumulator for a line of data [accumWid=wid*fac]
	Progress *prog;
	void zeroAccum(void);
	void writeAccum(int unscaledY);
public:
	MiniRaster(int shrinkFactor,Raster &dest);
	virtual ~MiniRaster();
	virtual void reallocate(int Nwid,int Nht);
	virtual void setColor(int x,int y,const Color &c);
	virtual Color getColor(int x,int y) const;
	virtual void setRgbRow(int y,int x1,int x2,const byte *src);
	virtual void setRgbaRow(int y,int x1,int x2,const byte *src);
	virtual void setGrayRow(int y,int x1,int x2,const byte *src);
};

}; }; //end namespace osl::Graphics
#endif //__OSL_RASTER_H

