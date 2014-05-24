/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 12/26/2002

Graphics utility class: Path

 * A Path is the central abstraction for Postscript-style graphics.
 *  It is an ordered sequence of pen motions ("segments"), which
 *  includes line segments, curves, and arcs.  Paths can describe the 
 *  boundary of virtually arbitrary shapes, including shapes with holes
 *  and disconnected regions.
 */
#ifndef __OSL_PATH_H
#define __OSL_PATH_H

#include <vector>

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#ifndef __OSL_BBOX2D_H
#  include "osl/bbox2d.h"
#endif
#ifndef __OSL_MATRIX2D_H
#  include "osl/matrix2d.h"
#endif
#ifndef __OSL_GRAPHICS_UTIL_H
#  include "osl/graphics_util.h"
#endif


namespace osl { namespace graphics2d {

/**
 * Represents a 2d ellipse, with a polar coordinate frame.
 *
 * Points at an angle theta on the ellipse always satisfy 
 *    p(theta) = center + x * cos(theta) + y * sin(theta)
 * This representation is chosen because it is closed under
 * affine transformations: just transform center, x, and y.
 */
class Ellipse2d {
	Vector2d center; //Center location
	Vector2d x,y; //Directions of x and y axes (not perp.)
public:
	/**
	 * Build a circle with given center, with this radius.
	 */
	Ellipse2d(const Vector2d &center_,double radius)
		:center(center_), x(radius,0), y(0,radius) {}
	/**
	 * Build an actual ellipse with the given center of symmetry,
	 *  and x and y axes.
	 */
	Ellipse2d(const Vector2d &center_, 
		const Vector2d &x_, const Vector2d &y_)
		:center(center_), x(x_), y(y_) {}
	
	inline const Vector2d &getCenter(void) const {return center;}
	inline const Vector2d &getX(void) const {return x;}
	inline const Vector2d &getY(void) const {return y;}
	
	/**
	 * Return the location at angle ang (in radians) on the ellipse.
	 */
	Vector2d polar(double ang) const 
		{return center+x*cos(ang)+y*sin(ang);}
	
	/**
	 * Transform this ellipse by this matrix.
	 * This is the big advantage of this ellipse representation--
	 * it's easy to transform.
	 */
	void transform(const Matrix2d &m) {
		center=m.apply(center);
		x=m.applyDirection(x);
		y=m.applyDirection(y);
	}
};

/** 
 * A Path is the central abstraction for Postscript-style graphics.
 *  It is an ordered sequence of pen motions ("segments"), which
 *  includes line segments, curves, and arcs.  Paths can describe the 
 *  boundary of virtually arbitrary shapes, including shapes with holes
 *  and disconnected regions.  This is actually a class that accepts
 *  the segments that describe a Path; a class that produces path
 *  segments is a Shape, below.
 *
 * Each call to move begins a new "subpath", which is a continous
 *   piece of the shape's boundary.  A closed subpath is one which 
 *   ends in a close call, which connects the subpath back to its 
 *   starting point.
 *
 * Subpaths may intersect, or be contained in other subpaths.
 *
 * This is the abstract superclass of all classes that can 
 *  accept Paths.
 */
class Path {
public:
	typedef const Vector2d cv2;
	
	virtual ~Path();
	
	///Set the current point to p.  This begins a new subpath.
	virtual void move(cv2 &p) =0;
	
	/**
	 * Draw a straight line from the current point to p.
	 * If there is no current point, this is equivalent to move.
	 */
	virtual void line(cv2 &p) =0;
	
	///Connect the current point and the first point of the subpath.
	virtual void close(void) =0;
	
	/**
	 * Draw a cubic bezier curve between the current point,
	 * the two given control points, and the destination.
	 *
	 * There must be a current point to call cubic().
	 *
	 * Always leaves the current point at final.
	 *
	 * The default implementation splits the cubic into 
	 * line segments, using the flatness error tolerance.
	 */
	virtual void cubic(cv2 &ctrl1,cv2 &ctrl2,cv2 &final);
	
	/** 
	 * Draw an arc of this ellipse from the angles at start 
	 * to end radians.  There's no need for start to be 
	 * less than end, and neither need be less than two pi.
	 *
	 * Begins by calling line(start of arc), so the arc will
	 * be connected by a line segment to the rest of the path.
	 * Leaves the current point at the end of the arc.
	 *
	 * The default implementation splits the arc into cubic bezier
	 * sections, using the flatness error tolerance.
	 */
	virtual void arc(const Ellipse2d &e,double start,double end);
	
	/**
	 * Return the current point. 
	 */
	virtual Vector2d getLastPoint(void) const =0;
	
	/// Render a clockwise-wound (under math axes) ellipse at e.
	inline void ellipse(const Ellipse2d &e) {arc(e,2*M_PI,0); close(); }
	inline void circle(const Vector2d &cen,double r) {ellipse(Ellipse2d(cen,r));}
	
	/**
	 * Add a Postscript-style arcto segment, creating a rounded 
	 *  (with radius r) corner circle between the lines AB and BC.  
	 *  Returns the tangent points along the lines, tAB and tBC.
	 */
	void arcto(double r,
		const Vector2d &A,const Vector2d &B,const Vector2d &C,
		Vector2d *tAB=NULL,Vector2d *tBC=NULL);
	
	///Aliases for above operations, with Postscript names
	inline void moveto(cv2 &p) {move(p);}
	inline void moveto(double x,double y) {move(Vector2d(x,y));}
	inline void rmoveto(cv2 &del) {move(getLastPoint()+del);}
	inline void rmoveto(double dx,double dy) {rmoveto(Vector2d(dx,dy));}
	
	inline void lineto(cv2 &p) {line(p);}
	inline void lineto(double x,double y) {line(Vector2d(x,y));}
	inline void rlineto(cv2 &del) {line(getLastPoint()+del);}
	inline void rlineto(double dx,double dy) {rlineto(Vector2d(dx,dy));}
	
	inline void curveto(cv2 &ctrl1,cv2 &ctrl2,cv2 &final) 
		{cubic(ctrl1,ctrl2,final);}
	inline void rcurveto(cv2 &delCtrl1,cv2 &delCtrl2,cv2 &delFinal) 
	{
		Vector2d loc=getLastPoint();
		cubic(loc+delCtrl1, loc+delCtrl2, loc+delFinal);
	}
	
	
	/**
	 * Return our curve error tolerance, which is used by the default
	 * arc and cubic routines for subdivision.  By default this returns
	 * 0.25 (one quarter-pixel, if this is device coordinates), which should
	 * be fine for most purposes.
	 */
	virtual double getFlatness(void) const;
};


/**
 * A Shape is a source for Paths--something that makes Path
 * calls.  A better name for this interface might be PathSource.
 *
 * For example, circles, rectangles, polygons, etc. 
 * are all Shapes.
 */
class Shape {
public:
	virtual ~Shape();
	
	/**
	 * Play back the our path segments to this destination.
	 * This will result in a sequence of move, line, close, 
	 * cubic, and arc calls to dest.
	 *
	 * This call can be repeated to play the same stored path
	 * back several times.
	 */
	virtual void draw(Path &dest) const =0;
};

/**
 * Apply this affine transform to the incoming path, passing the transformed
 * result to this destination.
 */
class TransformPath : public Path {
	const Matrix2d *m; //Matrix to transform by (can change)
	Path *dest; //Destination for transformed Paths
public:
	/// Transform by this matrix and pass to this destination.
	TransformPath(const Matrix2d &m_,Path &dest_) 
		:m(&m_), dest(&dest_) {}
	
	/// These transform the segment and pass straight to dest:
	virtual void move(const Vector2d &p);
	virtual void line(const Vector2d &p);
	virtual void close(void);
	virtual void cubic(const Vector2d &c1,const Vector2d &c2,
		const Vector2d &final);
	virtual void arc(const Ellipse2d &e,double start,double end);
	
	virtual Vector2d getLastPoint(void) const;
	
	virtual double getFlatness(void) const;
};

/**
 * Apply an affine transformation to this Shape.
 */ 
class TransformShape : public Shape {
	const Shape &src;
	Matrix2d m;
public:
	TransformShape(const Matrix2d &m_,const Shape &src_)
		:src(src_), m(m_) {}
	
	virtual void draw(Path &dest) const;
};

/// Draw a line between two points.  Not closed.
class LineShape : public Shape {
	Vector2d start, end;
public:
	LineShape(const Vector2d &s,const Vector2d &e)
		:start(s), end(e) {}
	
	virtual void draw(Path &dest) const;
};

/// A polygon, which can be closed or non-closed.
class PolyShape : public Shape {
	const Vector2d *pts;
	int nPts;
	bool doClose;
public:
	PolyShape(const Vector2d *pts_,int nPts_,bool doClose_=true)
		:pts(pts_), nPts(nPts_), doClose(doClose_) {}
	
	virtual void draw(Path &dest) const;
};

/// An arc between two angles.
class ArcShape : public Shape {
	Ellipse2d e;
	double start,end;
public:
        ArcShape(const Ellipse2d &e_,double s,double e)
                :e(e_), start(s), end(e) {}
        
        virtual void draw(Path &dest) const;
};

/// A clockwise-wound, closed circular shape.
class CircleShape : public Shape {
	Vector2d center; double r;
public:
	CircleShape(const Vector2d &center_,double r_)
		:center(center_), r(r_) {}
	
	virtual void draw(Path &dest) const;
};

/// A clockwise-wound, axis-aligned closed rectangular shape.
class BoxShape : public Shape {
	Vector2d tl; //Small-coordinates points
	Vector2d br; //Large-coordinates points
public:
	BoxShape(double l,double t, double r,double b)
		:tl(l,t), br(r,b) {}
	BoxShape(const Vector2d &tl_,const Vector2d &br_)
		:tl(tl_), br(br_) {}
	BoxShape(const Rect &r)
		:tl(r.left,r.top), br(r.right,r.bottom) {}
	
	virtual void draw(Path &dest) const;
};
typedef BoxShape RectShape;
typedef BoxShape RectangleShape;


/********** Silly Path utilities *************/
/// Appends the path segments for one shape after another.
class AppendShape : public Shape {
	const Shape &first, &second;
public:
	AppendShape(const Shape &first_, const Shape &second_)
		:first(first_), second(second_) {}
	
	virtual void draw(Path &dest) const;
};

/// Do-nothing path
class NullPath : public Path {
	Vector2d lastPt;
public:
	virtual void move(const Vector2d &p);
	virtual void line(const Vector2d &p);
	virtual void close(void);
	virtual void cubic(const Vector2d &c1,const Vector2d &c2,
		const Vector2d &final);
	virtual void arc(const Ellipse2d &e,double start,double end);
	virtual Vector2d getLastPoint(void) const;
};

/// Skim off a bounding box as path segments go by.
/// Sadly, because of getLastPoint, there *must* be a dest path.
class BboxPath : public Path {
	Path &dest;
	Bbox2d box;
public:
	BboxPath(Path &dest_) :dest(dest_) {box.empty();}
	
	virtual void move(const Vector2d &p);
	virtual void line(const Vector2d &p);
	virtual void close(void);
	virtual void cubic(const Vector2d &c1,const Vector2d &c2,
		const Vector2d &final);
	virtual void arc(const Ellipse2d &e,double start,double end);
	virtual Vector2d getLastPoint(void) const;
	
	inline const Bbox2d &getBox(void) const { return box; }
	inline Bbox2d getBox(const Matrix2d &m) const { 
		Bbox2d ret(m.apply(box.getMin()), m.apply(box.getMax()));
		return ret; 
	}
};


/// Accept a sequence of directed line segments from ShatterPath
class ShatterDest {
public:
	virtual ~ShatterDest();
	virtual void line(const Vector2d &start,const Vector2d &end) =0;
};

/// Convert a path to a sequence of directed line segments
class ShatterPath : public Path {
	ShatterDest &dest;
protected:
	Vector2d firstPt; //First point on subpath
	Vector2d lastPt; //Last point on subpath we've hit
	bool hasLast; //True unless we're between subpaths
public:
	ShatterPath(ShatterDest &dest);
	
	virtual void move(const Vector2d &p);
	virtual void line(const Vector2d &p);
	// arc and curve are defaults
	virtual void close(void);
	virtual Vector2d getLastPoint(void) const;
};


}; };

#endif
