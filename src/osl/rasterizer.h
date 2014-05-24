/*
Orion's Standard Library
Orion Sky Lawlor, 12/22/2000
NAME:		osl/rasterizer.h

DESCRIPTION:	Drawable (GC, GrafPort) class.
Implements the osl::Graphics interface for Pixel-based
(Raster) outputs.
*/
#ifndef __OSL_RASTERIZER_H
#define __OSL_RASTERIZER_H

#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif
#ifndef __OSL_RASTERIZER_UTIL_H
#include "osl/rasterizer_util.h"
#endif

namespace osl { namespace graphics2d {

class LineSource;

/**
 * An implementation of the Graphics interface for Raster output--
 * the usual pixel arrays.  This is the most common kind of Graphics.
 */
class Rasterizer : public Graphics {
protected:
	Raster *r;///< Destination image
	bool deleteR; ///< If true, delete r on exit.
	void release(void); ///< Deallocate output buffer.
	
	/**
	 * The scan-converted outline of the Shape we're drawing.  
	 * This buffer gets added to as we rasterize the shape,
	 * then used to fill the shape.
	 */
	ru::ScanConverted sc;
	/// Set up sc with this Shape:
	void scanConvert(cgs &state,const Shape &s);
	
	/// Copy Pixels from src into our image, masking by sc.
	void copyFrom(const GraphicsState &s,
		const Matrix2d &srcFmDest,
		const Raster &src);
public:

	Rasterizer(Raster &dest);///< Write to this image
	Rasterizer(int wid,int ht);///< Allocates new buffer
	~Rasterizer();
	
	//Set the current clip Region (all must be within the Raster buffer)
	// The default is (0,0,wid,ht)
	void setClip(const Rect &r);

	/// Set the raster buffer and clip region to this size
	void reallocate(int w,int h); 
	
	/// Get this Rasterizer's output buffer
	Raster &getBuffer(void) {return *r;}
	const Raster &getBuffer(void) const {return *r;}
	/// Set the output buffer. This version will delete the raster.
	void setBuffer(Raster *r);
	/// Set the output buffer.  This won't delete the raster.
	void setBuffer(Raster &r);
	
	/// Name should be one of "Times", "Symbol", "Courier", or "Helvetica"
	virtual Font *newFont(const char *name,double size=14.0);
	virtual void character(gs &s,int c);//Draw single (non-control) char
	
	virtual void copy(cgs &s,const Raster &src);
	
	virtual void fill(cgs &gs,const Shape &s);

	virtual void clear(const Color &c);
	
	/**
	  Special raster fill routine: copy this LineSource's
	  data into us, inside the given Shape.
	*/
	virtual void copyshape(cgs &gs,const Shape &s,const LineSource &src);
};

}; }; //end namespace osl::Graphics
#endif //__OSL_RASTERIZER_H

