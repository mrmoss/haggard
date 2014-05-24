/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 1/1/2003

Graphics utility class: Stored Path class


*/
#ifndef __OSL_STOREDPATH_H
#define __OSL_STOREDPATH_H

#ifndef __OSL_PATH_H
#  include "osl/path.h"
#endif

namespace osl { namespace graphics2d {

/**
 * A Path stored as a sequence of operations.  This can be used
 *   to pass paths around, and is used inside the Postscript interpreter
 *   to carry path state.
 *
 * This class is one of the few instances of both a Path (a 
 * consumer of line and arc segments) as well as a Shape (a 
 * source of line and arc segments).
 *
 * The internal representation is two vectors-- one of operator codes,
 *   the other of Vector2d operator parameters.
 */
class StoredPath : public Path, public Shape {
	typedef enum {Invalid=0, Move, Line, Arc, Cubic, Close} operator_type;
	//The following should be a vector<operator_type>, but 
	// enums are usually stored as ints, which are too big.
	typedef std::vector<unsigned char> ops_t;
	ops_t ops;
	
	typedef std::vector<Vector2d> pts_t;
	pts_t pts;
	
	Vector2d lastPt; ///< Only used during construction.
public:
	StoredPath(void);
	
	inline const pts_t &getPoints(void) const { return pts; }
	inline pts_t &setPoints(void) { return pts; }
	inline const ops_t &getOps(void) const { return ops; }
	inline ops_t &setOps(void) { return ops; }
	
	/// Clear the stored path
	void clear(void);
	
	/// These only append the segment to the stored path:
	virtual void move(const Vector2d &p);
	virtual void line(const Vector2d &p);
	virtual void close(void);
	virtual void cubic(const Vector2d &c1,const Vector2d &c2,
		const Vector2d &final);
	virtual void arc(const Ellipse2d &e,double start,double end);
	
	virtual Vector2d getLastPoint(void) const;
	
	/**
	 * Play back the current path to this destination.
	 * This will result in the same sequence of move, line, 
	 * arc, cubic, and close calls to dest that were given 
	 * to this Path.
	 *
	 * This call can be repeated to play the same stored path
	 * back several times.
	 */
	void draw(Path &dest) const;
};


}; };

#endif
