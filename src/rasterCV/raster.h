/**
Rasterization utility functions--draws shapes like pie wedges,
using arbitrary per-pixel blending, and arbitrary raster data storage,
both achieved via templates.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-07 (Public Domain)
*/
#ifndef __RASTERCV_RASTER_H
#define __RASTERCV_RASTER_H

#include <algorithm> /* for min/max */
#include <vector>

namespace rasterCV {


/** This represents a contiguous block of pixels in a row.
  Spans are used to efficiently calculate rendered areas of shapes.
*/
class raster_span {
public:
	int lo, hi; ///< first and last+1 pixels
	enum {big=0xffFFffF};

	/// Default constructor makes an empty span
	raster_span() :lo(big),hi(-big) {}

	/// Manually assign endpoints
	raster_span(int lo_,int hi_) :lo(lo_),hi(hi_) {}

	/// Intersect this span with us
	void intersect(const raster_span &o) {
		lo=std::max(lo,o.lo);
		hi=std::min(hi,o.hi);
	}

	/// Return true if we contain no pixels
	bool is_empty() const { return lo>=hi; }

	/// Return the number of pixels we contain
	int length() const {return hi-lo;}

	/// Return true if we contain this pixel number
	template <class coord_t>
	bool contains(coord_t pixel) const { return pixel>=lo && pixel<hi; }
};


/** This is the pixel size of an image. */
class image_size {
public:
	raster_span x_range, y_range; ///< Pixels cover this range.

	image_size(int wid,int ht)
		:x_range(0,wid), y_range(0,ht)
		{}

	inline int width() const {return x_range.length();}
	inline int height() const {return y_range.length();}

	/// Return true if we contain this pixel
	template <class coord_t>
	bool contains(coord_t x,coord_t y) const {
		return x_range.contains(x) && y_range.contains(y);
	}
};

/**
 This template wraps a block of memory as an IMAGE template.
*/
template <class T>
class raster_image : public image_size {
public:
	T *pixels; ///< Actual image data is here.
	int row_shift; ///< Distance, in pixels, between each successive row.

	/** Create an image with this size and pixel data.
	   If row_shift is specified, it gives the distance in pixels between each row. */
	raster_image(int wid,int ht,T *pixels_,int row_shift_=0)
		:image_size(wid,ht), pixels(pixels_), row_shift(row_shift_)
	{
		if (row_shift==0) row_shift=wid;
	}

	/** Return a reference to pixel (x,y).
	    No range checking--MUST be in bounds! */
	T &at(int x,int y) const {
		return pixels[y*row_shift+x];
	}
};

/**
 This template not only wraps, but stores an IMAGE in a block of memory.
*/
template <class T>
class raster_storage : public raster_image<T> {
	std::vector<T> pixel_storage;
public:
	raster_storage(int wid,int ht,T clear=T(0))
		:raster_image<T>(wid,ht,0,0),
		 pixel_storage(wid*ht,clear)
	{
		raster_image<T>::pixels=&pixel_storage[0];
	}
};


/** This class is designed to be used as a pixel type that matches
    OpenCV's, er, "unique" CV_8UC3 BGR format. */
struct pixel_bgr {
public:
	unsigned char b; ///< Blue
	unsigned char g; ///< Green
	unsigned char r; ///< Red
};


#if defined(__OPENCV_CORE_HPP__)
/**
 This template wraps a cv::Mat as an IMAGE template.
 CAUTION: at runtime, the type of the image must match T
 (e.g., unsigned char for CV_8U, pixel_bgr for CV_8UC3)
*/
template <class T>
class openCV_image : public raster_image<T> {
public:
	openCV_image(cv::Mat &m)
		:raster_image<T>(m.cols,m.rows,(T *)m.data,m.step[0]) {}
};
#endif

#if defined(__gl_h_)||defined(__GLUT_H__)||defined(__GLEW_H__)
template <class T> class gl_image_traits {};

template <> class gl_image_traits<unsigned char> {
public: enum { internalFormat=GL_LUMINANCE8, format=GL_LUMINANCE, type=GL_UNSIGNED_BYTE };
};
template <> class gl_image_traits<unsigned short> {
public: enum { internalFormat=GL_LUMINANCE16, format=GL_LUMINANCE, type=GL_UNSIGNED_SHORT };
};
template <> class gl_image_traits<float> {
public: enum { internalFormat=GL_LUMINANCE}; enum { format=GL_LUMINANCE, type=GL_FLOAT };
};
template <> class gl_image_traits<pixel_bgr> {
public: enum { internalFormat=GL_RGB, format=GL_RGB, type=GL_UNSIGNED_BYTE };
};


/** Dump a simple_image into an OpenGL texture.
   OpenGL treats 0,0 as the bottom left corner,
   opposite most other libraries--we doesn't fix this!
*/
template <class T>
void glTexImage2D(raster_image<T> &src,int mip_level=0) {
	gl_image_traits<T> t;
	glPixelStorei(GL_PACK_ALIGNMENT,1);
	glPixelStorei(GL_PACK_ROW_LENGTH,src.row_shift);
	::glTexImage2D(GL_TEXTURE_2D,
		mip_level,
		t.internalFormat,
		src.x_range.length(),src.y_range.length(),
		0, // border
		t.format,
		t.type,
		src.pixels);
	glPixelStorei(GL_PACK_ROW_LENGTH,0);
}

#endif



/**
 A polar-oriented halfspace, passing through the origin
*/
class raster_polar_halfspace {
public:
	float a; // angle, in radians
	float c,s,c_s; // cosine, sine, cos/sin

	raster_polar_halfspace(float ang,float direction=+1.0)
		:a(ang)
	{
		c=cos(a)*direction; s=sin(a)*direction;
		if (s==0.0) c_s=0.0; // invalid
		else c_s=c/s;
	}

	/* Return the X span of this halfspace at this Y value. */
	raster_span x_range(float y) const {
		if (s==0.0) { /* horizontal line--check sign of Y */
			if (y*c>0) return raster_span(); // empty
			else return raster_span(-raster_span::big,raster_span::big); // everything
		}
		else { /* ordinary skew line */
			float x=y*c_s;
			if (s>0) return raster_span(-raster_span::big,x); // left-infinite span
			else return raster_span(x,raster_span::big); // right-infinite span
		}
	}
};

/** Rasterize this sector of a ring, applying PIXELMOD to each pixel */
template <class IMAGE,class PIXELMOD>
void rasterize_sector(IMAGE &img // image to modify (must have x_range and y_range members)
	,PIXELMOD &mod // how to modify it (draw function)
	,float cx,float cy // location of center (pixels)
	,float startAng // angle of start of sector (radians counterclockwise from +x direction)
	,float endAng // angle of end of sector
	,float insideR // radius of inside of ring (pixels)
	,float outsideR // radius of outside of ring (pixels)
	)
{
	raster_span y_range(ceil(cy-outsideR),floor(cy+outsideR));
	y_range.intersect(img.y_range);
	raster_polar_halfspace start(startAng,1.0);
	raster_polar_halfspace end(endAng,-1.0);

	for (int y=y_range.lo;y<y_range.hi;y++) {
		float dy=y-cy; // relative to origin

		/* Start with the halfspaces for the angles */
		raster_span X=start.x_range(dy);
		X.intersect(end.x_range(dy));
		X.lo+=cx; X.hi+=cx; // angles are defined with center at 0,0

		X.intersect(img.x_range);
		if (X.is_empty()) continue; // early exit

		/* Now clip by outside of circle */
		float xr=sqrt(outsideR*outsideR-dy*dy); // x^2 = r^2 - y^2
		X.intersect(raster_span(ceil(cx-xr),floor(cx+xr)));
		if (X.is_empty()) continue; // early exit

		/* Check if we'll hit the inside of the circle */
		float xid=insideR*insideR-dy*dy;
		if (xid>0) { /* this span hits the inside of the circle */
			float xir=sqrt(xid); // x^2 = r^2 - y^2

			// Span before the inside hole:
			raster_span XL(-raster_span::big,floor(cx-xir));
			XL.intersect(X);
			mod.draw(img,y,XL);

			// Span after the inside hole:
			raster_span XR(ceil(cx+xir),raster_span::big);
			XR.intersect(X);
			mod.draw(img,y,XR);
		}
		else
		{ /* we don't hit the inside of the circle */
			mod.draw(img,y,X);
		}

	}
}


/* Assigns a value to pixels. */
template <class T>
class pixel_writer {
public:
	T value;
	pixel_writer(T value_) :value(value_) {}

	template <class IMAGE>
	void draw(IMAGE &img,int y,raster_span span) {
		for (int x=span.lo;x<span.hi;x++)
			img.at(x,y)=value;
	}
};



}; // end namespace



#endif /* defined(this header) */

