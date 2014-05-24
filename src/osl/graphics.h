/*
Orion's Standard Library
Orion Sky Lawlor, 12/21/2000
NAME:		osl/graphics.h

DESCRIPTION:	C++ Graphics library

This file provides routines for creating, reading in,
writing out, and manipulating 2D graphic images. 

This is the abstract superclass.  Concrete subclasses
are Raster (Pixelated image), post (PostScript vector output),
and GUI-specific Window classes.
*/
#ifndef __OSL_GRAPHICS_H
#define __OSL_GRAPHICS_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#ifndef __OSL_BBOX2D_H
#  include "osl/bbox2d.h"
#endif
#ifndef __OSL_MATRIX2D_H
#  include "osl/matrix2d.h"
#endif
#ifndef __OSL_COLOR_H
#  include "osl/color.h"
#endif
#ifndef __OSL_GRAPHICS_UTIL_H
#  include "osl/graphics_util.h"
#endif
#ifndef __OSL_PATH_H
#  include "osl/path.h"
#endif


namespace osl { namespace graphics2d {

/**
 * A Font is a size and style of type that letters can be drawn in.
 * This is the abstract superclass, which only contains measurement
 * information.  Subclasses contain the actual font data.
 */
class Font : public Noncopyable {
protected:
	double pt_size;
public:
	/**
	 * Build a font with this point size.  This is only used
	 * by concrete Font subclasses.
	 */
	Font(double Npt_size) :pt_size(Npt_size) {}
	virtual ~Font();
	
	/**
	 * Fonts can be only metrics, include rasterized images,
	 * or consist of actual outlines.
	 */
	enum {impl_metrics=1,impl_bitmaps=2,impl_outlines=4};
	virtual int impl_type(void) =0;//Return implementation code
	
//Return various character measurements
	/// Return the distance from the baseline to bottom of the letter 'g'
	virtual double descender(void) const =0;
	/// Return the distance from the baseline to top of the letter 'X'
	virtual double ascender(void) const =0;
	/// Return this Unicode character's width
	virtual double width(int ch) const =0;
	
	/// Return this string's width.
	/// Default implementation loops over and sums up the widths of each
	/// character, without kerning.
	virtual double width(const char *s) const;
	
	/// Return the point size of this font, in pixels.
	/// The default implementation returns pt_size.
	virtual double size(void) const;
};

/**
 * Stroke describes the properties of a line:
 *    - The linewidth
 *    - The way line segments are joined together
 *    - The way line ends are capped
 */
class Stroke {
public:
	inline float getLineWidth(void) const {return width;}
	inline void setLineWidth(float lineWidth) { width=lineWidth; }
	inline void setLineWidth(double lineWidth) { width=(float)lineWidth; }
	
	/// Selects the type of corner join (low numbers are same as Postscript)
	typedef enum {
		MITER_JOIN=0, ///< Extend the two lines until they meet (up to the miter limit)
		ROUND_JOIN=1, ///< Add a rounded segment where they meet
		BEVEL_JOIN=2, ///< Cut off the lines where they end, but fill the joint.
		CRACK_JOIN=3  ///< Leave a gap at the outer angle between the lines
	} join_t;
	inline join_t getJoin(void) const {return joinStyle;}
	inline void setJoin(join_t join) { joinStyle=join; }
	
	/// Selects the type of line-end cap (low numbers same as Postscript)
	typedef enum {
		BUTT_CAP=0, ///< Stroke ends square (the default).
		ROUND_CAP=1, ///< Stroke ends with a circle.
		PROJECTING_CAP=2, ///< Stroke ends with a half-linewidth extra length.
		DIAMOND_CAP=3, ///< Stroke ends in a square diamond shape of half the linewidth.
		DAGGER_CAP=4 ///< Stroke ends in an elongated dagger shape of a full linewidth.
	} cap_t;
	inline cap_t getCap(void) const {return capStyle;}
	inline void setCap(cap_t cap) { capStyle=cap; }
	
	/// For the MITER_JOIN type, this selects the length cutoff beyond which
	///  we switch to a bevelled join.
	inline float getMiterLimit(void) const { return miterLimit; }
	inline void setMiterLimit(float m) { miterLimit=m; }
	inline void setMiterLimit(double m) { miterLimit=(float)m; }
	
	Stroke() 
		:width(1.0), miterLimit(10.0), joinStyle(MITER_JOIN), capStyle(BUTT_CAP) 
		{}
private:
	float width;
	float miterLimit;
	join_t joinStyle;
	cap_t capStyle;
	// FIXME: add dash pattern here
};

/**
 * GraphicsState keeps track of the current Color, pen position, 
 * line width, Font, etc.  GraphicsState is separate from the actual
 * drawing class, Graphics, to make it easy to re-use this graphics state 
 * without having to manually reset everything.
 *
 * The mutating GraphicsState routines (like setColor) return a reference
 * to this GraphicsState object.  This is to make it easy to chain together
 * several mutator calls, like:
 *   gs.setColor(c).setLineWidth(2.0).scale(1.4);
 */
class GraphicsState : public Stroke {
protected:
	typedef       GraphicsState  gs;
	typedef       Vector2d  v2;
	typedef const Vector2d cv2;
	typedef unsigned int property_t;
	
	Color color_;
	bool fill_;
	property_t properties_; ///< Bitvector of properties
	const Font *font_;
	Vector2d point_;
	Matrix2d matrix_; ///< Maps user coordinates to output device coordinates
	
public:
	enum {//Property_t bits-- various mundane Graphics minutia
		EO_FILL=1u<<2,//Use even-odd winding rule for fill (not nonzero)
		
		//These properties are specific to Raster Graphics
		DISABLE_AA=1u<<10,//Turn off shape antialiasing
		NEAREST=1u<<11,//Use nearest-neighbor Raster filtering (instead of bilinear)
		DISABLE_CLIP=1u<<14,//Do not clip-- assume everything is in-bounds
		DISABLE_MATRIX=1u<<15,//Assume coordinates are already Pixels
		//These properties are hints only
		HINT_BEST=1u<<20,//Prefer quality to speed
		
		LAST_PROPERTY=1u<<31
	};
	
	GraphicsState() :color_(0),fill_(false),
		properties_(0),font_(0),point_(0,0),matrix_(1.0) {}
	
	//Inherited:
	gs& setLineWidth(double wid) { Stroke::setLineWidth(wid); return *this;}
	
	/// Fill is now deprecated:
	bool            getFill(void) const {return fill_;}
	gs& setFill(bool doFill) {fill_=doFill; return *this;}
	
	double            getGray(void) const {return color_.asGray();}
	const Color &   getColor(void) const {return color_;}
	property_t      getProperties(void) const {return properties_;}
	const Font *    getFont(void) const {return font_;}
	cv2 &           getPoint(void) const {return point_;}
	const Matrix2d &getMatrix(void) const {return matrix_;}

	gs& setGray(double bright) {color_=Color(bright); return *this;}
	gs& setColor(const Color &c) {color_=c; return *this;}
	gs& setProperties(property_t p) {properties_=p;return *this;}
	gs& setFont(const Font *f) {font_=f; return *this;}
	gs& setPoint(cv2 &cur) {point_=cur; return *this;}
	gs& setMatrix(const Matrix2d &m) {matrix_=m; return *this;}
	
	bool getProperty(property_t p) const {return 0!=(p&properties_);}
	gs& setProperty(property_t p,bool to) 
	{
		if (to) properties_|=p; else properties_&=~p;
		return *this;
	}
	
	gs& move(cv2 &d) {point_+=d; return *this;}
	gs& moveto(cv2 &t) {point_=t; return *this;}

	//Manipulate the input coordinates->drawn coordinates matrices
	gs& resetMatrix(void) {matrix_.identity(); return *this;}
	gs& translate(cv2 &toOrig) {matrix_.translate(mapDir(toOrig)); return *this;}
	gs& scale(cv2 &s);//Scale coordinate axes in x and y
	gs& rotate(double radCCW);//Rotate CCW about origin
	gs& flipY(void);//Mirror about x axis-- y'=-y;
	gs& flipX(void);//Mirror about y axis-- x'=-x;
	Vector2d map(cv2 &v) const {return matrix_.apply(v);}
	Vector2d mapDir(cv2 &v) const {return matrix_.applyDirection(v);}
	double getScale(void) const;
	/// Post-multiply our coordinate transform by this matrix
	gs& product(const Matrix2d &src) {
		Matrix2d tmp;
		matrix_.product(src,tmp); matrix_=tmp;
		return *this;
	}
	/// Pre-multiply our coordinate transform by this matrix
	gs& preduct(const Matrix2d &src) {
		Matrix2d tmp;
		src.product(matrix_,tmp); matrix_=tmp;
		return *this;
	}
	
	gs& setColor(float r,float g,float b,float a=1.0) {return setColor(Color(r,g,b,a));}
	gs& setPoint(double x,double y) {return setPoint(Vector2d(x,y));}
	gs& move(double dx,double dy) {return move(Vector2d(dx,dy));}
	gs& moveto(double x,double y) {return moveto(Vector2d(x,y));}
	gs& scale(double s) {return scale(Vector2d(s,s));}
	gs& scale(double sx,double sy) {return scale(Vector2d(sx,sy));}
	gs& translate(double x,double y) {return translate(Vector2d(x,y));}
	gs& rotateDeg(double degCCW) {return rotate(degCCW*(M_PI/180.0));}
};


}; }; //End namespace osl::Graphics (for include below)
#include "osl/raster.h"
namespace osl{ namespace graphics2d {

/**
 * A Graphics is a place to draw things, such as a Rasterizer, 
 * or Postscript file. It's analogous to a "GC" in X windows or
 * Microsoft Windows; or a "Grafport" on a Macintosh.
 */
class Graphics : public Noncopyable {
protected:
	typedef       GraphicsState  gs;
	typedef const GraphicsState cgs;
	typedef       Vector2d  v2;
	typedef const Vector2d cv2;
public:
	virtual ~Graphics();

// Text processing:
	/**
	 * Create a new font of this type and size.
	 *
	 * The caller is responsible for eventually deleting this Font, which
	 * can only be used with this Graphics.
	 *
	 * @param Name A font name. Should be one of "Times", "Symbol", "Courier", or "Helvetica".
	 */
	virtual Font *newFont(const char *name,double size=14.0) =0;
	
	/**
	 * Render this Unicode character at s.getPoint(), advancing s.getPoint()
	 * by the width of the character.
	 */
	virtual void character(gs &s,int c)=0;//Draw a single (non-control) char
	
	/**
	 * Draw this C-style string at s.getPoint().  This routine
	 * accepts newlines, but does no line wrapping.
	 */
	virtual void text(gs &s,const char *str);//Draw text at current Point

// Raster imaging:
	/**
	 * Copy this source image into our graphics, drawing the image
	 * inside [0,src.getWidth()) x [0,src.getHeight()) of the current
	 * graphics state matrix coordinate system.
	 * 
	 * The GraphicsState matrix should contain a matrix mapping
	 *  to output coordinate pixels from source pixels (screenFmSrc).
	 *
	 * To shift the image, call s.translate() before making this call.
	 */
	virtual void copy(cgs &s,const Raster &src)=0;

// Shapes:
	/**
	 * Fill this shape using the current graphics color.
	 */
	virtual void fill(cgs &gs,const Shape &s) =0;
	
	/**
	 * Outline (stroke) this shape, using the current line width
	 * (caps, joins, and dash pattern).  See osl/stroke.h for more
	 * options.
	 *
	 * The default outlines the shape, then calls fill.
	 */
	virtual void stroke(cgs &gs,const Shape &s);
	
	/**
	 * These defaults just create the corresponding Shape and call
	 * fill or stroke.
	 */
	virtual void poly(cgs &s,cv2 *p,int nPts); //Draw arbitrary, but closed polygon
	virtual void lineSegment(cgs &s,cv2 &start,cv2 &end); //Draw line
	virtual void circle(cgs &s,cv2 &c,double radius); //Draw circle
	virtual void box(cgs &s,double x1,double y1,double x2,double y2); //Draw Rectangle
	
	//Erase the entire image to this color
	virtual void clear(const Color &c);

	//Aliases/shorthand
	void circle(cgs &s,double x,double y,double radius) 
	  {circle(s,Vector2d(x,y),radius);}
	void box(cgs &s,cv2 &o,cv2 &d) 
	  {box(s,o.x,o.y,o.x+d.x,o.y+d.y);}
	void rect(cgs &s,const Rect &r) 
	  {box(s,r.left,r.top,r.right,r.bottom);}
	
	void line(gs &s,cv2 &d)//Draw line from current by d
	  {v2 dest=s.getPoint()+d; lineSegment(s,s.getPoint(),dest); s.setPoint(dest);}
	void lineto(gs &s,cv2 &t)//Draw a line from current to t
	  {lineSegment(s,s.getPoint(),t); s.setPoint(t);}
	void lineto(gs &s,double x,double y) {lineto(s,Vector2d(x,y));}

};


}; }; //end namespace osl::Graphics

#include "osl/rasterizer.h"
#include "osl/postscript.h"

#endif
