/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 1/1/2003

Graphics utility class: Stroke

*/
#ifndef __OSL_STROKE_H
#define __OSL_STROKE_H

#ifndef __OSL_PATH_H
#  include "osl/path.h"
#endif
#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif

namespace osl { namespace graphics2d {
	
/**
 * A StrokeStyle draws the line width, join style, and end caps
 * applied to each line in a path.  Virtually all user-defined 
 * StrokeStyles are OnePassStrokeStyles.  In fact, it's not clear
 * that this class should even exist, other than as documentation.
 */
class StrokeStyle {
public:
	virtual ~StrokeStyle();
	
	/**
	 * Draw a "dot cap" for a single-point path.
	 * The default dot cap is empty.
	 *
	 * @param dest The Path to send the dot cap to.
	 * @param dot The location of the only point in the path.
	 */
	virtual void dotcap(Path &dest, const Vector2d &dot) const;
};


/**
 * A OnePassStrokeStyle strokes a path using separate entities for 
 * each line, join, and cap.  This is the most general StrokeStyle,
 * and should be easy to extend.
 */
class OnePassStroke : public StrokeStyle { 
public:
	/**
	 * Draw this line segment in the middle of the path.
	 *
	 * @param dest The Path to send the line to.
	 * @param A The previous point along the path.
	 * @param B The next point along the path.
	 */
	virtual void line(Path &dest,const Vector2d &A,const Vector2d &B) const =0;
	
	/**
	 * Draw a "line join" for this corner. The join should add segments
	 * to dest to draw the corner between the line segments AB and BC.
	 *
	 * @param dest The Path to send the corner join to.
	 * @param A The previous point along the path.
	 * @param B The location of the corner to join.
	 * @param C The next point along the path.
	 */
	virtual void cornerjoin(Path &dest, const Vector2d &A,const Vector2d &B,const Vector2d &C) const =0;
	
	/**
	 * Draw an "end cap" for the tip of this (non-closed) path.
	 *
	 * @param dest The Path to send the end cap to.
	 * @param end The endpoint of the path.
	 * @param inner The previous point on the path.
	 * @param isStart True if this is the first point on the path, false if the last.
	 */
	virtual void endcap(Path &dest, const Vector2d &end, const Vector2d &inner, bool isStart) const =0;	
};

/**
 * A TwoPassStrokeStyle strokes a path by making two passes: first up one side
 * of the path, then down the other.  This is, suprisingly, trickier and less general 
 * than a OnePassStrokeStyle, but can result in much smaller, more efficient output paths.
 */
class TwoPassStroke : public StrokeStyle {
public:
	/**
	 * Draw a "line join" for this corner.  The join should add segments
	 * to dest covering the left-handed (viewed from A looking toward
	 * B) corner of the line segments AB and BC.
	 *
	 * @param dest The Path to send the corner join to.
	 * @param A The previous point along the path.
	 * @param B The location of the corner to join.
	 * @param C The next point along the path.
	 */
	virtual void cornerjoin(Path &dest, const Vector2d &A,const Vector2d &B,const Vector2d &C) const =0;
	
	/**
	 * Draw an "end cap" for the tip of this (non-closed) path.
	 * The cap should be drawn clockwise around the end point,
	 * oriented away from the inner point.
	 *
	 * @param dest The Path to send the end cap to.
	 * @param end The endpoint of the path.
	 * @param inner The previous point on the path.
	 * @param isStart True if this is the first point on the path, false if the last.
	 */
	virtual void endcap(Path &dest, const Vector2d &end, const Vector2d &inner, bool isStart) const =0;
};

/**
 * A helper class for implementing "standard" stroke styles--
 * a fixed linewidth, small selection of join and cap types, and
 * a miter limit.
 */
class StandardStroke {
public:
	double halfWidth; ///< Half the linewidth (everything's centered, so this is what's needed).
	double miterMaxSq; ///< Maximum actual distance to extend mitered joints from center, squared
	Stroke::join_t join;
	Stroke::cap_t cap;
	StandardStroke(const Stroke &s);
	
	/// "normalize" this vector to be half the linewidth long.
	Vector2d normalize(const Vector2d &v) const; 
};

/**
 * One-pass implementation of standard stroke style.  These draw clockwise-oriented
 * closed segments for each of a line, cornerjoin, and endcap.
 */
class OnePassStandardStroke : public OnePassStroke {
protected:
	StandardStroke s;
public:
	OnePassStandardStroke(const Stroke &s_) :s(s_) {}
	virtual void line(Path &dest,const Vector2d &A,const Vector2d &B) const;
	virtual void cornerjoin(Path &dest, const Vector2d &A,const Vector2d &B,const Vector2d &C) const;
	virtual void endcap(Path &dest, const Vector2d &end, const Vector2d &inner, bool isStart) const;
	virtual void dotcap(Path &dest, const Vector2d &dot) const;
};


/**
 * Two-pass implementation of standard stroke style.  Again, these draw
 * clockwise-oriented closed segments.
 */
class TwoPassStandardStroke : public TwoPassStroke {
protected:
	StandardStroke s;
public:
	TwoPassStandardStroke(const Stroke &s_) :s(s_) {}
	virtual void cornerjoin(Path &dest, const Vector2d &A,const Vector2d &B,const Vector2d &C) const;
	virtual void endcap(Path &dest, const Vector2d &end, const Vector2d &inner, bool isStart) const;
	virtual void dotcap(Path &dest, const Vector2d &dot) const;
};


/**
 * Stroke this source Shape using this StrokeStyle.
 */
class StrokeShape : public Shape {
	const OnePassStroke *onePass; //One of these two must be NULL
	const TwoPassStroke *twoPass; 
	const Shape &src;
public:
	StrokeShape(const OnePassStroke &s,const Shape &src_)
		:onePass(&s),twoPass(0),src(src_) {}
	StrokeShape(const TwoPassStroke &s,const Shape &src_)
		:onePass(0),twoPass(&s),src(src_) {}
	
	virtual void draw(Path &dest) const;
};



}; };

#endif
