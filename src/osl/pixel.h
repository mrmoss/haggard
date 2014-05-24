/*
Orion's Standard Library
Orion Sky Lawlor, 1/24/2002
NAME:		osl/pixel.h

DESCRIPTION:	C++ image pixel library 

This file provides templated implementations of color pixels.
These classes are heavily optimized for extreme speed; so
they're a good deal more confusing than you might expect.

The general contract for a pixel type is very simple-- provide
conversions to and from graphics2d::Color's, and get/set
RGB, BGRA, RGBA, and BGRA bytes.
*/
#ifndef __OSL_PIXEL_H
#define __OSL_PIXEL_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#ifndef __OSL_FAST_MATH_H
#  include "osl/fast_math.h"
#endif

namespace osl { namespace graphics2d {
/**************** Bit Operations ***********************/

/**
 Templated bit shift function, which shifts backwards for negative
 shift count.  This is a really stupid, ugly implementation, but it inlines,
 unlike the obvious implementation involving an "if".
*/
template <int shift> inline unsigned int signedLeftShift(unsigned int val) {return val<<shift;}
template <> inline unsigned int signedLeftShift<-1>(unsigned int val) {return val>>1;}
template <> inline unsigned int signedLeftShift<-2>(unsigned int val) {return val>>2;}
template <> inline unsigned int signedLeftShift<-3>(unsigned int val) {return val>>3;}
template <> inline unsigned int signedLeftShift<-4>(unsigned int val) {return val>>4;}
template <> inline unsigned int signedLeftShift<-5>(unsigned int val) {return val>>5;}
template <> inline unsigned int signedLeftShift<-6>(unsigned int val) {return val>>6;}
template <> inline unsigned int signedLeftShift<-7>(unsigned int val) {return val>>7;}
template <int shift> inline unsigned int signedRightShift(unsigned int val) {return val>>shift;}
template <> inline unsigned int signedRightShift<-1>(unsigned int val) {return val<<1;}
template <> inline unsigned int signedRightShift<-2>(unsigned int val) {return val<<2;}
template <> inline unsigned int signedRightShift<-3>(unsigned int val) {return val<<3;}
template <> inline unsigned int signedRightShift<-4>(unsigned int val) {return val<<4;}
template <> inline unsigned int signedRightShift<-5>(unsigned int val) {return val<<5;}
template <> inline unsigned int signedRightShift<-6>(unsigned int val) {return val<<6;}
template <> inline unsigned int signedRightShift<-7>(unsigned int val) {return val<<7;}


/**
  Extract a field of destBits low bits from the field of width srcBits
  starting at bit srcShift of src.  If destBits>srcBits, the low bits
  of destBits will contain additional bits from src.
  
  This routine is used to extract a single channel (e.g., red) from a pixel.
  It's a class, not a routine, because template partial specialization
  (used for the bitfield expansion cases below) doesn't *work* for functions--
  it's not in the C++ standard!
*/
template <class storage_t, unsigned int srcShift,unsigned int srcBits, unsigned int destBits>
class extractBitfield_c { public:
inline unsigned int extract(storage_t src) { return signedRightShift<srcShift+srcBits-destBits>(src);}
};

/**
  Specialized extraction function for expanding 1-bit wide fields to 8 bits.
*/
template <class storage_t, unsigned int srcShift>
class extractBitfield_c<storage_t,srcShift,1,8> { public:
inline unsigned int extract(storage_t src) { 
	unsigned int ret=0x01&(src>>srcShift);
	/// Copy that 1 bit into all 8 places:
	ret|=ret<<1; ret|=ret<<2; ret|=ret<<4;
	return ret;
}
};

/**
  Specialized extraction function for expanding 4-bit wide fields to 8 bits.
*/
template <class storage_t, unsigned int srcShift>
class extractBitfield_c<storage_t,srcShift,4,8> { public:
inline unsigned int extract(storage_t src) { 
	unsigned int ret=0x0f&(src>>srcShift);
	/// Use the high 4 bits to fill in the missing low bits:
	return (ret<<4)|ret;
}
};

/**
  Specialized extraction function for expanding 5-bit wide fields to 8 bits.
*/
template <class storage_t, unsigned int srcShift>
class extractBitfield_c<storage_t,srcShift,5,8> { public:
inline unsigned int extract(storage_t src) { 
	unsigned int ret=0x1f&(src>>srcShift);
	/// Use the high 3 bits to fill in the missing low bits:
	return (ret<<3)|(ret>>2);
}
};

/// Gets rid of extractBitfield_c class silliness; adds function interface.
template <class storage_t, unsigned int srcShift,unsigned int srcBits, unsigned int destBits>
inline unsigned int extractBitfield(storage_t src) {
	extractBitfield_c<storage_t,srcShift,srcBits,destBits> b;
	return b.extract(src);
}


/// Return a bitmask for the low n bits.
template <class storage_t,int n>
inline storage_t bitmask(void) {return (((storage_t)(1u))<<n)-1u;}

/// Silly class used in insertBitfield, to allow partial specialization.
///   This is the default case when the source and destination are different sizes.
template <class storage_t, unsigned int destBits, unsigned int srcBits>
class maskBitfield_c {public:
storage_t mask(storage_t val) { return val&(bitmask<storage_t,destBits>()<<(srcBits-destBits)); }
};

/// No need to mask if source and dest bitfields are same size:
template <class storage_t, unsigned int commonBits>
class maskBitfield_c<storage_t,commonBits,commonBits> {public:
storage_t mask(storage_t val) { return val; }
};

/// Shift val, which has srcBits bits, over to be a bitfield of destBits starting at destShift.
template <class storage_t, unsigned int destShift,unsigned int destBits, unsigned int srcBits>
inline storage_t insertBitfield(storage_t src) {
	maskBitfield_c<storage_t,destBits,srcBits> b;
	return signedLeftShift<destShift+destBits-srcBits>(b.mask(src));
}



/**************************** Pixel Type ************************/
/// Usual Pixel type: a single value stores the r,g,b, and a fields
///  in different bit fields.  This allows various SIMD-like operations easily.
/// Storage layout depends on host integer width and endian-ness:
///   see ARGB_Pixel (aka RgbaPixel) for discussion of typical layout.
template <
	class storage_t, //Storage class for Pixel
	unsigned int rShift, unsigned int rBits, //(s>>rShift)&((1<<rBits)-1) is red channel
	unsigned int gShift, unsigned int gBits, // green channel
	unsigned int bShift, unsigned int bBits, // blue channel
	unsigned int aShift, unsigned int aBits  // alpha channel (or none if aBits==0)
> class  ColorPixelT {
	storage_t s;

	//Compile-time constants.  Expressed as inline functions because
	// in-class constants are horrific in C++.  Function should optimize away completely.
	inline unsigned int rmask(void) const { return (1u<<rBits)-1u; }
	inline unsigned int gmask(void) const { return (1u<<gBits)-1u; }
	inline unsigned int bmask(void) const { return (1u<<bBits)-1u; }
	inline unsigned int amask(void) const { return (1u<<aBits)-1u; }
	
	inline unsigned int rfield(void) const {return (s>>rShift)&rmask();}
	inline unsigned int gfield(void) const {return (s>>gShift)&gmask();}
	inline unsigned int bfield(void) const {return (s>>bShift)&bmask();}
	inline unsigned int afield(void) const {return (s>>aShift)&amask();}

	//Convert the given field to an 8-bit byte
	//  Fields shorter than 8 bits are padded with garbage
	inline byte rtoByte(void) const {return byte(extractBitfield<storage_t,rShift,rBits,8>(s));}
	inline byte gtoByte(void) const {return byte(extractBitfield<storage_t,gShift,gBits,8>(s));}
	inline byte btoByte(void) const {return byte(extractBitfield<storage_t,bShift,bBits,8>(s));}
	inline byte atoByte(void) const {return byte(extractBitfield<storage_t,aShift,aBits,8>(s));}

	//Convert the given byte value into a Color field.
	inline storage_t rfromByte(byte b) const {return insertBitfield<storage_t,rShift,rBits,8>(b);}
	inline storage_t gfromByte(byte b) const {return insertBitfield<storage_t,gShift,gBits,8>(b);}
	inline storage_t bfromByte(byte b) const {return insertBitfield<storage_t,bShift,bBits,8>(b);}
	inline storage_t afromByte(byte b) const {return insertBitfield<storage_t,aShift,aBits,8>(b);}

public:
	enum {CHANNEL_MAX=(1<<rBits)-1};
	
	inline storage_t rm(void) const {return rmask()<<rShift;}
	inline storage_t gm(void) const {return gmask()<<gShift;}
	inline storage_t bm(void) const {return bmask()<<bShift;}
	inline storage_t am(void) const {return amask()<<aShift;}
	inline unsigned int rf(void) const {return rfield();}
	inline unsigned int gf(void) const {return gfield();}
	inline unsigned int bf(void) const {return bfield();}
	inline unsigned int af(void) const {return afield();}
	inline byte r(void) const {return rtoByte();}
	inline byte g(void) const {return gtoByte();}
	inline byte b(void) const {return btoByte();}
	inline byte a(void) const {return atoByte();}
	inline byte redByte(void) const {return rtoByte();}
	inline byte greenByte(void) const {return gtoByte();}
	inline byte blueByte(void) const {return btoByte();}
	inline byte alphaByte(void) const {
		if (aBits!=0)
			return atoByte();
		else
			return (byte)255;
	}
	
//Default copy constructor, assignment operator are fine
	ColorPixelT() {}//Default constructor
	ColorPixelT(storage_t v) :s(v) {}
	ColorPixelT(const Color &c) {
		s=  ((fastFloor(c.r*(rmask()+0.99)))<<rShift)+
			((fastFloor(c.g*(gmask()+0.99)))<<gShift)+
			((fastFloor(c.b*(bmask()+0.99)))<<bShift);
		if (aBits!=0)
			s+=(fastFloor(c.a*(amask()+0.99)))<<aShift;
	}
	ColorPixelT(byte v) {
		setRgb(v,v,v);
	}
	ColorPixelT(byte r,byte g,byte b) {
		setRgb(r,g,b);
	}
	ColorPixelT(byte r,byte g,byte b,byte a) {
		setRgb(r,g,b,a);
	}
	operator Color() const { return getColor(); }
	Color getColor(void) const {
		const float rConv=1.0f/rmask();
		const float gConv=1.0f/gmask();
		const float bConv=1.0f/bmask();
		float alpha=1.0f;
		if (aBits!=0) {
			const float aConv=1.0f/amask();
			alpha=aConv*afield();
		}
		return Color(rConv*rfield(),gConv*gfield(),bConv*bfield(),Color::premultiplied(alpha));
	}
	operator storage_t() const { return s; }
	storage_t val(void) const { return s; }
	operator storage_t&() { return s; }
	inline byte operator[](int i) const {
		if (i==0) return r();
		if (i==1) return g();
		if (i==2) return b();
		return a();
	}

//Get this Pixel's value into the given (8-bit) byte buffer
	void getBytes(byte *dest,int dR,int dG,int dB) const
		{dest[dR]=redByte();dest[dG]=greenByte();dest[dB]=blueByte();}
	void getRgb(byte *dest) const {getBytes(dest,0,1,2);}
	void getBgr(byte *dest) const {getBytes(dest,2,1,0);}
	void getRgba(byte *dest) const {getRgb(dest);dest[3]=alphaByte();}
	void getBgra(byte *dest) const {getBgr(dest);dest[3]=alphaByte();}
	
	void getGray(byte *d) const {
		d[0]=byte((rtoByte()+
		           gtoByte()+
		           btoByte()
			)/3u);
	}
//Set this Pixel to the given (8-bit) byte buffer
	inline void setRgb(byte r,byte g,byte b,byte a=255u) {
		if (aBits!=0)
		  s=rfromByte(r)+
		    gfromByte(g)+
		    bfromByte(b)+
		    afromByte(a);
		else /*No alpha channel*/
		  s=rfromByte(r)+
		    gfromByte(g)+
		    bfromByte(b);
	}
	void setRgb(const byte *s) {
		setRgb(s[0],s[1],s[2]);
	}
	void setRgba(const byte *s) {
		setRgb(s[0],s[1],s[2],s[3]);
	}
	void setBgr(const byte *s) {
		setRgb(s[2],s[1],s[0]);
	}
	void setBgra(const byte *s) {
		setRgb(s[2],s[1],s[0],s[3]);
	}
	void setGray(const byte *s) {
		setRgb(s[0],s[0],s[0]);
	}
};

template <class inPix_t,class outPix_t> 
void convertPixel(inPix_t in,outPix_t &out) {
	out.setRgb(in.r(),in.g(),in.b(),in.a());
}

//-- the remaining Pixel types are rarely used --
//Pixel type for individually-addressible Color channels
template <
	class channel,//Data type of one Color channel, e.g., byte
	unsigned int nChan //Number of (Color) channels, including alpha (e.g. 4)
> class ChannelPixelT {
protected:
	channel data[nChan];
public:
	//Default copy constructor, assignment operator are fine
	ChannelPixelT() {}//Default constructor
	ChannelPixelT(const channel *a) 
		{for (unsigned int i=0;i<nChan;i++) data[i]=a[i];} 

	//Bracket operator: lets you extract a single channel
	channel &operator[](int i) {return data[i];} 
	const channel &operator[](int i) const {return data[i];} 
	
	//To treat Pixel data as a simple Pointer
	channel *ptr(void) {return data;}
	const channel *ptr(void) const {return data;}
};

/**
  Pixel type for separately-stored r,g,b(,a) color channels.
*/
template <
	class channel,//Data type of one Color channel, e.g., byte
	int chanMax, //Maximum value of one channel (e.g. 255)
	int r_ind, int g_ind, int b_ind, // stored channel index of red,green,blue
	int a_ind // -1 if no alpha, else channel index of alpha channel
> class ChannelColorPixelT : public ChannelPixelT<channel,3+(a_ind!=-1)> {
	typedef ChannelPixelT<channel,3+(a_ind!=-1)> super;
public:
	enum {CHANNEL_MAX=chanMax};

	 //Default copy constructor, assignment operator are fine
	ChannelColorPixelT() {}//Default constructor
	ChannelColorPixelT(const channel *a) :super(a) {}
	inline ChannelColorPixelT(const Color &c) {
		set(channel(c.r*chanMax),channel(c.g*chanMax),channel(c.b*chanMax),channel(c.a*chanMax));
	}
	inline ChannelColorPixelT(channel r,channel g,channel b, channel a=0) {
		set(r,g,b,a);
	}
	inline void set(channel r,channel g,channel b, channel a=chanMax) {
		this->data[r_ind]=r; this->data[g_ind]=g; this->data[b_ind]=b; 
		if (a_ind!=-1) this->data[a_ind]=a;
	}
	inline operator Color() const {return toColor();}
	Color toColor(void) const {
		const float conv=1.0f/chanMax;
		return Color(conv*r(),conv*g(),conv*b(),Color::premultiplied(conv*a()));
	}
	
	inline channel r(void) const {return this->data[r_ind];}
	inline channel g(void) const {return this->data[g_ind];}
	inline channel b(void) const {return this->data[b_ind];}
	inline channel a(void) const {
		if (a_ind!=-1)
			return this->data[a_ind];
		else
			return chanMax;
	}
	
	//Get this Pixel into the given byte buffer
	void getRgb(byte *d) const {toColor().getRgb(d);}
	void getRgba(byte *d) const {toColor().getRgba(d);}
	void getBgr(byte *d) const {toColor().getBgr(d);}
	void getBgra(byte *d) const {toColor().getBgra(d);}
	void getGray(byte *d) const {toColor().getGray(d);}
	
	//Set this Pixel from the given byte buffer
	void setRgb(const byte *s) {
		set(
			this->data[0]*channel(chanMax)/255,
			this->data[1]*channel(chanMax)/255,
			this->data[2]*channel(chanMax)/255
		);
	}
	void setRgba(const byte *s) {
		set(
			this->data[0]*channel(chanMax)/255,
			this->data[1]*channel(chanMax)/255,
			this->data[2]*channel(chanMax)/255,
			this->data[3]*channel(chanMax)/255
		);
	}
	void setBgr(const byte *s) {
		set(
			this->data[2]*channel(chanMax)/255,
			this->data[1]*channel(chanMax)/255,
			this->data[0]*channel(chanMax)/255
		);
	}
	void setBgra(const byte *s) {
		set(
			this->data[2]*channel(chanMax)/255,
			this->data[1]*channel(chanMax)/255,
			this->data[0]*channel(chanMax)/255,
			this->data[3]*channel(chanMax)/255
		);
	}
	void setGray(const byte *s) {
		channel v=s[0]*channel(chanMax)/255;
		set(v,v,v);
	}
};

template <
	class channel,//Data type of one Color channel, e.g., byte
	int chanMax //Maximum value of one channel (e.g. 255)
> class GrayPixelT {
	channel val;
public:
	 //Default copy constructor, assignment operator are fine
	GrayPixelT() {}//Default constructor
	GrayPixelT(const channel a) {val=a;}
	GrayPixelT(const Color &c) {
		val=channel(c.asGray()*chanMax);
	}
	operator Color() const {
		return Color(float(val)/chanMax);
	}
	
	//Bracket operator: lets you extract a single channel
	operator channel &() {return val;} 
	operator const channel &() const {return val;} 
	
	//Get this Pixel's value into the given byte buffer
	void getRgb(byte *d) const
		{byte v=byte(val*255/chanMax);for (int i=0;i<3;i++) d[i]=v;}
	void getRgba(byte *d) const
		{getRgb(d); d[0]=byte(255);}
	void getGray(byte *d) const
		{d[0]=byte(val*255/chanMax);}
	//Set this Pixel's value from the given byte buffer
	void setRgb(const byte *s)
		{val=(s[0]+s[1]+s[2])/3*channel(chanMax)/255;}
	void setRgba(const byte *s) 
		{setRgb(s);}
	void setGray(const byte *s) 
		{val=s[0]*channel(chanMax)/255;}
};

/********************* De-templating Pixel typedefs ******************
These can be easier to type (and much less frightening) than
the whole templated definition.
*/

/// Gray is easy: there's only one channel.
typedef GrayPixelT<unsigned char, 255> grayPixel;

/**
  Standard color pixel type used throughout OSL.
  
  Builds pixel via: int pix=(A<<24)+(R<<16)+(G<<8)+B.  
  
  Windows Compatability:
    On a little-endian machine, as bytes this is BGRA,
     the quasi-standard for Windows 32-bit DIBs.  This means
     ARGB_Pixel's can be used directly with SetDIBitsToDevice,
     and they're fast.
  
  Mac Compatability:
    On a big-endian machine, as bytes this is ARGB,
     the standard for Mac 32-bit GWorld images.
  
  OpenGL Compatability:
    GL wants RGBA by default, which is too bad.  However, 
    GL_BGRA/GL_UNSIGNED_BYTE works on little-endian machines;
    and GL_BGRA/GL_UNSIGNED_INT_8_8_8_8_REV works on big-endian 
    machines.  Apple also has a GL_ARGB_EXT for Macs.
  
  Most file formats want something different; but that's OK, 
  because files are slow anyway.
*/
typedef ColorPixelT<uint32, 16,8, 8,8, 0,8, 24,8> ARGB_Pixel;

/// "Standard" color pixel used everywhere:
///   Alpha high, red next, green middle, blue low.
/// Note the "Rgba" in RgbaPixel means it's got 4 channels,
///  it does *not* imply anything about the channel order!
typedef ARGB_Pixel RgbaPixel;

//--the remaining Pixel types are rarely used:
/// 24-bit Pixel, without alpha channel
typedef ColorPixelT<uint32, 16,8,  8,8,  0,8, 24,0> RgbPixel;

/// BGRA (little bytes: ARGB) 32-bit pixel. (swapped version of standard pixel)
///    OpenGL type: GL_BGRA/GL_UNSIGNED_INT_8_8_8_8
typedef ColorPixelT<uint32,  8,8, 16,8, 24,8,  0,8> BGRA_Pixel; 

/// RGBA (little bytes: ABGR) 32-bit pixel.  OpenGL's favorite type.
///    OpenGL type: GL_RGBA/GL_UNSIGNED_INT_8_8_8_8
typedef ColorPixelT<uint32, 24,8,  16,8,  8,8,  0,8> RGBA_Pixel; 
/// ABGR (little bytes: RGBA) 32-bit pixel. 
typedef ColorPixelT<uint32,  0,8,  8,8, 16,8, 24,8> ABGR_Pixel; 

/// Usual 16-bit Pixel with equal-weight Color resolution and a 1-bit alpha channel
///   OpenGL type: GL_BGRA,GL_UNSIGNED_SHORT_1_5_5_5_REV
typedef ColorPixelT<uint16, 10,5, 5,5, 0,5, 15,1> ARGB_Pixel5;

/// Funny OpenGL RGBA 16-bit Pixel:
///   OpenGL type: GL_RGBA/GL_UNSIGNED_SHORT_5_5_5_1
typedef ColorPixelT<uint16, 11,5, 6,5, 1,5,  0,1> RGBA_Pixel5;

/// Alphaless 16-bit Pixel:
///   OpenGL type: GL_RGB/GL_UNSIGNED_SHORT_5_6_5
typedef ColorPixelT<uint16, 11,5, 5,6, 0,5, 16,0> RGB_Pixel565;

//10 bit color channels (accurate, but weird)
typedef ColorPixelT<uint32, 20,10, 10,10, 0,10, 30,2> ARGB_Pixel10; 

//16 bit color channels
typedef ChannelColorPixelT<uint16,65535,1,2,3,0> ARGB_Pixel16;
typedef ChannelColorPixelT<uint16,65535,0,1,2,3> RGBA_Pixel16;

typedef GrayPixelT<float,1> floatPixel;
	
/**************** Pixel Source *******************/
class PixelSource {
public:
	virtual ~PixelSource();
	
	//Get the Color of this line at the current x, and
	// then advance to the next x.
	virtual Color getColor(void)=0;
	//As above, but fetch the RgbaPixel value
	virtual RgbaPixel getPixel(void);
};

class LineSource {
public:
	virtual ~LineSource();
	
	//Get the Pixels for this scanline (like "new HitList")
	//  Multiple lines may be outstanding on the same source.
	virtual PixelSource *startLine(int firstX,int y) const =0;
	
	//Give this line back to the source (like "delete h")
	//  Memory *will* leak if there are startLines 
	//    without matching endLines!
	virtual void endLine(PixelSource *h) const =0;
};

//Superclass of .16 fixed-Point linear sources:
class LinearPixelSource : public PixelSource {
protected:
	int fix_dx,fix_dy;//.16 fixed-Point x increments
	int fix_x, fix_y;//.16 fixed-Point current location
public:
	inline void setDel(int dx,int dy) {fix_dx=dx; fix_dy=dy;}
	inline void setLoc(int x,int y) {fix_x=x; fix_y=y;}
	inline void advance(void) {
		fix_x+=fix_dx; fix_y+=fix_dy;
	}
	//Each of these calls the other-- be sure to overload one!
	virtual Color getColor(void);
	virtual RgbaPixel getPixel(void);
};

//Implementation utility: reads Pixels from a Raster image
class RasterLineSource : public LineSource {
protected:
	const Raster *src;
public:
	RasterLineSource(const Raster *src_) :src(src_) { }
};

//Implementation utilty for LineSources: splits out source creation
// (createSource, private) from source initialization (startLine, public)
class CachingLineSource : public RasterLineSource {
	mutable PixelSource *cache;
protected:
	virtual PixelSource *createSource(void) const=0;
	inline PixelSource *getSource(void) const {
		if (cache==NULL) return createSource();
		else {PixelSource *ret=cache; cache=NULL; return ret;}
	}
public:
	CachingLineSource(const Raster *src_);
	~CachingLineSource();
	void endLine(PixelSource *h) const;
};

//Extract Pixels from a source image according to a Matrix map
class Matrix2dSource : public CachingLineSource {
	const Matrix2d &m;
	int fix_dx, fix_dy;//.16 fixed-Point line increments
	const GraphicsState &s;
	PixelSource *createSource(void) const;
public:
	Matrix2dSource(const Raster &src_,const Matrix2d &m_,const GraphicsState &s_);
	PixelSource *startLine(int firstX,int y) const;
};


}; }; //end namespace osl::Graphics
#endif //__OSL_RASTER_H

