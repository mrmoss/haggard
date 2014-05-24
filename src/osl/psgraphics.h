/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 4/2/2002

DESCRIPTION:	C++ Postscript Interpreter: Graphics Library

This file defines the graphics-related routines used inside the
Postscript Interpreter.  The fact that there's anything
*but* this header is a sad commentary on the bloated state of
the Postscript language.
*/
#ifndef __OSL_PSGRAPHICS_H
#define __OSL_PSGRAPHICS_H

#include <vector>

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif
#ifndef __OSL_RASTER_H
#  include "osl/raster.h"
#endif
#ifndef __OSL_COLORS_H
#  include "osl/colors.h"
#endif
#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif
#ifndef __OSL_PATH_H
#  include "osl/path.h"
#endif
#ifndef __OSL_STOREDPATH_H
#  include "osl/storedpath.h"
#endif
#ifndef __OSL_PS_H
#  include "osl/ps.h"
#endif
#ifndef __OSL_PSOBJ_H
#  include "osl/psobj.h"
#endif

namespace osl { namespace ps {

class Interp;
class FontDictionary;
using osl::graphics2d::Raster;

/// A Postscript dictionary representing a font
class FontDictionary : public Dictionary {
public:
	FontDictionary() {}
	
	/// Convert this object to a font dict, or throw typecheck
	FontDictionary(Interp *interp,const Object &obj);
	
	/// Make ourselves a new FontID:
	void makeFontID(Interp *interp,NameID name);
	
	/// Lookup our FontID:
	FontID *getFontID(Interp *interp);
	
	/// Copy this font, applying the given matrix
	FontDictionary copy(Interp *interp,const Matrix2d &scl);
	
	void getMatrix(Interp *interp,Matrix2d &dest) const;
	void setMatrix(Interp *interp,const Matrix2d &m);
};

/**
 * Stored information about a font
 * FIXME: add actual font details here, as well as accessor methods.
 */
class FontID {
	NameID name;
	
	Matrix2d cacheMatrix;
	FontDictionary cacheFont;
	friend class FontDictionary;
public:
	NameID getName(void) const {return name;}
	
	void init(FontDictionary *dict,NameID name_); 
};

using osl::graphics2d::Shape;
using osl::graphics2d::Path;
using osl::graphics2d::TransformPath;
using osl::graphics2d::Ellipse2d;
using osl::graphics2d::Bezier;
using osl::graphics2d::StoredPath;

/**
 * The stored path used inside the Postscript interpreter.
 * It's a thin wrapper around a bare "StoredPath"-- all it adds
 * is a single "current point" location.
 */
class PsPath : public StoredPath {
	bool noCurrentPoint;
	typedef StoredPath super;
public:
	
	PsPath() { clear(); }
	
	//This is used for things like relative moves, etc.
	// throws "noCurrentpoint" if there's no current point;
	bool hasCurrentPoint(void) const {return !noCurrentPoint;}
	
	//Check if we have a current point.  If not, throw "nocurrentpoint"
	void checkCurrentPoint(Interp *interp);
	
	inline void setCurrentPoint(const Vector2d &p) {
		noCurrentPoint=false;
	}
	
	/// These update the current point as they go:
	virtual void move(const Vector2d &p);
	virtual void line(const Vector2d &p);
	virtual void close(void);
	virtual void cubic(const Vector2d &c1,const Vector2d &c2,
		const Vector2d &final);
	virtual void arc(const Ellipse2d &e,double start,double end);
	
	/// Clear the stored path
	void clear(void) {
		noCurrentPoint=true;
		super::clear();
	}
};


class Interp;
class Dictionary;
class Object;

using ::osl::graphics2d::GraphicsState;
using ::osl::graphics2d::CmykColor;
using ::osl::graphics2d::Color;

/**
 * Postscript dash pattern.  This is an array of dash/space lengths.
 * For example, the dash pattern {1.0,0.2} means 1.0 unit of dash,
 * then 0.2 units of space.
 */
class PsDashPat {
public:
	enum {dashMax=11};
private:
	float off, dashes[dashMax];
	int len;
public:
	void init(int len_,float off_,const float *src_);
	PsDashPat(int len_,float off_,const float *src_) 
		{init(len_,off_,src_);}
	
	int getLen(void) const {return len;}
	float getStart(void) const {return off;}
	float getIndex(int i) const {return dashes[i];}
};

/**
 * Postscript halftone screen representation.  This is print-specific,
 * and largely ignored by the interpreter.
 */
class PsScreen {
	float freq; //Cells per inch on paper (!)
	float ang; //Rotation angle in device coords
	Object proc; //Spot function
public:
	PsScreen();
	
	void set(float f,float a,Object p) 
		{freq=f; ang=a; proc=p;}
	float getFreq(void) const {return freq;}
	float getAng(void) const {return ang;}
	Object getProc(void) const {return proc;}
	
	void popFrom(Interp *interp);
	void pushTo(Interp *interp);
};


/**
 * All the graphics-related state maintained by the Postscript interpreter.
 * This includes the current color, linewidth, path, output device, etc.
 */
class PsGraphicsState : public GraphicsState {
public:
	typedef enum {SaveSource=0,GsaveSource} StateSource;
	typedef enum {
		FlagStrokeadjust=1<<0,
		FlagOverprint=1<<2
	} Flag_t;
private:
	typedef GraphicsState super;
	StateSource source;
	byte flags;
	float flatness;
	CmykColor cmyk;
	
	const PsDashPat *pat;
	FontDictionary font; ///< Current output font
	static NullDevice nulldevice;
	Device *device; ///< Current output device
	Matrix2d m_inv; ///< Inverse of user->device transform matrix
	PsPath path; ///< Current path for reading
	TransformPath destPath; ///< Storage for current path for writing
public:
	PsScreen colorscreen[4]; //Grey, red, green, blue
	Object colortransfer[4]; //Grey, red, green, blue
	inline PsPath &getPath(void) { return path; }
	inline Path &setPath(void) { 
		// It's a pain to keep destPath's pointers updated when we
		// get copied, so we just rebuild destPath each time:
		destPath=TransformPath(getMatrix(),path);
		return destPath; 
	}
	
	PsGraphicsState(void);
	
	void setDevice(Device *d) {device=d;}
	Device &getDevice(void) {return *device;}
	inline static Device *getNullDevice(void) {return &nulldevice;}
	void setFont(FontDictionary f) {font=f;}
	FontDictionary getFont(Interp *interp) const;
	void setSource(StateSource s) {source=s;}
	StateSource getSource(void) const {return source;}
	void setFlatness(float s) {flatness=s;}
	float getFlatness(void) const {return flatness;}
	void setStrokeadjust(bool b) {flags|=FlagStrokeadjust;}
	bool getStrokeadjust(void) const {return (bool)(flags&FlagStrokeadjust);}
	void setOverprint(bool b) {flags|=FlagOverprint;}
	bool getOverprint(void) const {return (bool)(flags&FlagOverprint);}
	void setDash(const PsDashPat *p) {pat=p;}
	const PsDashPat *getDash(void) const {return pat;}
	
	void setCMYK(const CmykColor &c) {
		cmyk=c;
		GraphicsState::setColor(c);
	}
	const CmykColor &getCMYK(void) {
		return cmyk;
	}
	void setColor(const Color &c) {
		GraphicsState::setColor(c);
		cmyk=c;
	}
	
	const Matrix2d &getMatrix(void) const {return super::getMatrix();}
	/// Set the user-to-device coordinate transform matrix:
	void setMatrix(const Matrix2d &m) {
		super::setMatrix(m);
		m.invert(m_inv);
	}
	
	/// Convert user coordinates to device coordinates
	inline Vector2d user2device(const Vector2d &src) const 
		{return getMatrix().apply(src);}
	/// Convert a user coordinates direction to device coordinates
	inline Vector2d user2deviceDirection(const Vector2d &src) const 
		{return getMatrix().applyDirection(src);}
	
	/// Convert device coordinates to user coordinates
	inline Vector2d device2user(const Vector2d &src) const 
		{ return m_inv.apply(src); }
	/// Convert a device coordinates direction to user coordinates
	inline Vector2d device2userDirection(const Vector2d &src) const 
		{ return m_inv.applyDirection(src); }
};

/**
 * Controls the postscript font cache machinery, which probably shouldn't
 * exist (or at least shouldn't be visible).
 */
class PsFontCache {
public:
	int blimit; //Maximum number of bytes occupied by a single cached glyph
	int mark,size,lower,upper; //"font cache parameters"
	int umark,ublimit; //parameters for user path cache
	PsFontCache() {
		blimit=0; 
		mark=0;size=0;lower=0;upper=0;
		umark=0; ublimit=0;
	}
	
};

Matrix2d pop(Interp *interp,Object *store=NULL);
bool conv(Interp *interp,const Object &o,Matrix2d &ret);
void conv(Interp *interp,const Matrix2d &src,Array &dest);
void push(Interp *interp,const Matrix2d &m,Object *store=NULL);
void push(Interp *interp,const Vector2d &v);

/**
 * (Hideous) Base class for Postscript interpreter-- contains the graphics-related
 * state.  It's a sad commentary on the status of Postscript that there's 
 * anything *other* than this class to a Postscript interpreter.
 */
class InterpGraphics {
protected:
	//Write our graphics definitions into this dictionary
	void graphicsDefinitions(Dictionary &dest,Interp *interp);
	Device *pagedevice;
	FontDictionary defaultFont;
public:
	enum {gsaveMax=31};
	Stack<PsGraphicsState,gsaveMax> gsave;
	PsGraphicsState gs;
	PsFontCache cacheparams;
	inline PsPath &getPath(void) {return gs.getPath();}
	inline Path &setPath(void) {return gs.setPath();}
	Device &getDevice(void) {return gs.getDevice();}
	Device *getPageDevice(void) {return pagedevice;}
	void setPageDevice(Device *d) {
		pagedevice=d; 
		gs.setDevice(pagedevice);
	}
	
	InterpGraphics(void);
	
	//Lookup a font dictionary
	const FontDictionary &lookupFont(NameID fontName) {
		return defaultFont;
	}
	
	Vector2d user2device(const Vector2d &src) const 
		{return gs.user2device(src);}
	Vector2d user2deviceDirection(const Vector2d &src) const 
		{return gs.user2deviceDirection(src);}
	Vector2d device2user(const Vector2d &src) const 
		{return gs.device2user(src);}
	Vector2d device2userDirection(const Vector2d &src) const 
		{return gs.device2userDirection(src);}
	
	/// Get the current user-coordinates path point, or throw.
	Vector2d getCurrentPoint(void);
};

}; };

#endif /* defined(thisHeader) */
