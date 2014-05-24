/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 7/8/2000

Mip-Mapping routines. (Multim-In-Parvo, "Many in same place")
*/
#include "osl/graphics.h"

namespace osl { namespace graphics3d {
using namespace osl::graphics2d;
typedef osl::graphics2d::RgbaRaster MipRaster;
typedef osl::graphics2d::RgbaPixel MipPixel;

class EllipseProperties;

class MipMap {
public:
	typedef unsigned int fix8;//24.8 fixed-Point format
	class mipRec {
	public:
		unsigned int xsize,ysize;//2^n, where n is the level of this mipmap
		unsigned int xmask,ymask;//(2^n)-1, same n
		MipRaster img;//a xsize x ysize image
		void set(void) {
			xsize=img.wid; ysize=img.ht;
			xmask=xsize-1; ymask=ysize-1;
		}
	};

public:
	int m;//Master (full size) image is up to 2^m x 2^m Pixels across.
	int masterSize;//2^m
	mipRec r[20];//goes from r[0] (1x1) ... r[m] (full size)
public:
	
	MipMap(const char *imgName) {MipRaster img;img.read(imgName);build(img);}
	MipMap(const MipRaster &img) {build(img);}
	MipMap(void) {m=masterSize=0;}
	
	/// Build mipmaps from the given master image.
	/// Img's size must be a power of two-- 2^n x 2^n.
	void build(const MipRaster &img);
	
	/// Return the mipmap that's at least i pixels across.
	const MipRaster &getImageSized(int minsize) const {
		for (int l=0;l<=m;l++)
			if (r[l].xsize>=(unsigned int)minsize)
				return r[l].img;
		return r[m].img;
	}
	
	/**
	Take a sample from the image of this size.
	The sample is centered at (x,y) in the "master" (top-level) image.
	The sample is sz top-level pixels across.  
	Out-of-bounds points wrap around.
	*/
	MipPixel sample(double x,double y,double sz) const
		{return sample((fix8)(256.0*x),(fix8)(256.0*y),(fix8)(256.0*sz));}
	/// As above, but in 24.8 fixed-Point format
	MipPixel sample(fix8 x,fix8 y,fix8 sz) const;
	
	/**
	  Take an anisotropic sample over this ellipse, in top-level coords.
	  Essentially uses Heckbert's weighted-average algorithm.
	  Will return the filtered color.
	*/
	Color ewa(EllipseProperties p) const;
};

}; };
