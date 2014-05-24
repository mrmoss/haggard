/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 1/4/2003

DESCRIPTION:	C++ ps* Interpreter: External interface

This file defines the bare-minimum external interface to 
the ps* interpreter.  If you want to define your own
ps* operators or otherwise get under the covers of the
ps* interpreter, see the internal headers in osl/psinterp.h.

* PostScript is a registered trademark of Adobe Corporation.
This software can read many files ending in .ps, but because 
it does not pay tribute to Adobe cannot be called a PostScript(TM) 
Interpreter.
*/
#ifndef __OSL_PS_H
#define __OSL_PS_H

#ifndef __OSL_IO_H
#  include "osl/io.h"
#endif
#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif

namespace osl { namespace ps {

using ::osl::graphics2d::Shape;
using ::osl::graphics2d::Raster;
using ::osl::graphics2d::Graphics;
using ::osl::graphics2d::GraphicsState;

/**
 * A Device is a place for graphics output; it receives the image
 * described.
 *
 * This is an abstract superclass-- inherit from this class to get
 * graphics shapes defined from within postscript.
 */
class Device {
public:
	virtual ~Device();
	
	/**
	 * Draw the outline of this path, using the current:
	 *    -color (gs.getColor())
	 *    -linewidth (gs.getLineWidth())
	 *    -join style (gs.getLinejoin())
	 *    -cap style (gs.getLinecap())
	 *    -dash mode (gs.getDash())
	 *
	 * The points along the shape are in Postscript user space,
	 * and gs.getMatrix() is the user-> device mapping.
	 */
	virtual void stroke(const GraphicsState &gs,const Shape &shape) =0;
	
	/**
	 * Fill this path using the current color.
	 *
	 * The points along the shape are already in Postscript device space,
	 * so gs.getMatrix() is the identity matrix.
	 */
	virtual void fill(const GraphicsState &gs,const Shape &shape) =0;
	
	/**
	 * Show this string at p, returning the new string start point.
	 *  @param p the start point of the text
	 *  @param str the characters to display
	 *  @param len the number of characters to display
	 */
	virtual Vector2d show(const GraphicsState &gs,const Vector2d &p,
		const char *str,int len) =0;
	
	/**
	 * Show this raster (pixel-by-pixel) image.
	 * gs.getMatrix() is the matrix to show the image.
	 *
	 *  @param r The image to show.
	 */
	virtual void image(const GraphicsState &gs,const Raster &r) =0;
	
	/**
	 * Output for this page is now complete. 
	 */
	virtual void showpage(void) =0;
	
	/** 
	 * Erase this page.  Used at the start of each page after the first, 
	 * and (very occasionally) for the erasepage operator.
	 */
	virtual void erasepage(void) =0;
};


/// Null Postscript output device: All output operators do nothing.
class NullDevice : public Device {
public:
	virtual ~NullDevice();
	virtual void stroke(const GraphicsState &gs,const Shape &shape) {}
	virtual void fill(const GraphicsState &gs,const Shape &shape) {}
	virtual Vector2d show(const GraphicsState &gs,const Vector2d &p,
		const char *str,int len) { return p; }
	virtual void image(const GraphicsState &gs,const Raster &r) {}
	virtual void erasepage(void) {}
	virtual void showpage(void) {}
};


/**
 * A PsViewMatrix flips the standard PS coordinate system around into
 * the standard Graphics coordinate system.
 *
 *  @param flipYat Mirror the Y axis around this line.  Typically the height,
 *      in pixels, of the destination Graphics.
 *  @param origin Location of the lowerleft corner in PS units (72 dpi pixels).
 *  @param scale Scale factor to apply.  1.0 scale factor means 72 dpi, the default.
 */
class PsViewMatrix {
	Matrix2d m;
	void set(double flipYat,const Vector2d &origin,const Vector2d &scale);
public:
	PsViewMatrix(double flipYat) 
		:m(1.0) { set(flipYat,Vector2d(0,0),Vector2d(1,1)); }
	PsViewMatrix(double flipYat,const Vector2d &origin) 
		:m(1.0) { set(flipYat,origin,Vector2d(1,1)); }
	PsViewMatrix(double flipYat,const Vector2d &origin,const Vector2d &scale) 
		:m(1.0) { set(flipYat,origin,scale); }
	
	operator const Matrix2d & () const { return m; }
};


/// GraphicsDevice draws Postscript to a standard graphics2d::Graphics.
class GraphicsDevice : public Device {
	osl::graphics2d::Graphics &graphics;
	osl::Matrix2d matrix;
	osl::graphics2d::Color pagecolor;
	osl::graphics2d::Font *font;
	osl::graphics2d::GraphicsState getState(const GraphicsState &gs);
	bool hitPage; // Showpage has been executed
	bool dirtyPage; // There are marks on this page
	void doDraw(void);
public:
	GraphicsDevice(osl::graphics2d::Graphics &graphics_,
			osl::Matrix2d matrix_,
			osl::graphics2d::Color pagecolor_=osl::graphics2d::Color::white);
	~GraphicsDevice();
	
	virtual void stroke(const GraphicsState &gs,const Shape &shape);
	virtual void fill(const GraphicsState &gs,const Shape &shape);
	virtual Vector2d show(const GraphicsState &gs,const Vector2d &p,
		const char *str,int len);
	virtual void image(const GraphicsState &gs,const Raster &r);
	
	/// Calls graphics.clear(pagecolor).
	virtual void erasepage(void);
	
	/// Sets hitPage to true
	virtual void showpage(void);
	
	inline bool hitNewPage(void) {
		if (!hitPage) return false;
		hitPage=false;
		return true;
	}
	inline bool pageDirty(void) {return dirtyPage;}
};


/// Exceptions thrown by PostScript:
class PsException : public ::osl::Exception {
public:
	typedef enum {
	invalid,
	cantPush,cantPop, //Thrown by Stack
	quit, //User called "quit"
	error, //Called Interp::error
	handledError //Called Interp::handledError
	} exceptionType;
private:
	exceptionType type;
	static char *type2str[];
public:
	PsException(exceptionType t);
	exceptionType getType(void) const {return type;}
};


/**
 * Read this Postscript (or EPS) file all the way through, making calls
 * to this Device.  This routine may throw IOExceptions or PsExceptions 
 * if there is an error during the read.
 */
void read(::osl::io::InputStream &s,Device &dest);
void read(::osl::io::InputStream &s,::osl::graphics2d::Graphics &dest,double ht);
void read(::osl::io::InputStream &s,::osl::graphics2d::Rasterizer &dest);


class PsFileImpl;
/**
 * Read this Postscript (or EPS) file incrementally 
 */
class PsFile {
	PsFileImpl *impl;
public:
	PsFile(::osl::io::InputStream &s);
	~PsFile();
	
	/// Read the next page of the file.
	/// The first call reads the first page.
	/// Returns false if there are no more pages.
	/// May throw IOException or PsException.
	bool nextPage(GraphicsDevice &dest);
};


}; };

#endif
