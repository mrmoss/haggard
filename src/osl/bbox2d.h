/*
Orion's Standard Library
Orion Sky Lawlor, 2005/1/27
NAME:		osl/vector2d.h

2D bounding box class.
*/

#ifndef __OSL_BBOX2D_H
#define __OSL_BBOX2D_H

#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif

#ifndef __OSL_VECTOR1D_H
#  include "osl/vector1d.h"
#endif

#ifndef __OSL_SERIALIZER_H
#  include "osl/serializer.h"
#endif

namespace osl {

/// A 2D bounding box.
template <class T,class VectorNd>
class bbox2dT {
	enum {N=2};
	typedef bbox2dT<T,VectorNd> bboxNd;
	typedef seg1dT<T> rSeg1d;
	/// Spans for x (0) and y (1)
	seg1dT<T> segs[N];
public:
	bbox2dT() {}
	bbox2dT(const rSeg1d &x,const rSeg1d &y)
		{segs[0]=x; segs[1]=y;}
	explicit bbox2dT(const VectorNd &a) {
		segs[0].init(a[0]); segs[1].init(a[1]);
	}
	bbox2dT(const VectorNd &a,const VectorNd &b)
	{
		segs[0].init(a[0],b[0]);
		segs[1].init(a[1],b[1]);
	}
	inline void shift(const Vector2d &by) {
		segs[0].shift(by.x); segs[1].shift(by.y);
	}
	/// Expand this bounding box by this distance on all sides.
	inline void expand(const Vector2d &dist) {
		add(getMin()-dist); add(getMax()+dist);
	}
	
	void print(const char *desc=NULL) const;

	rSeg1d &axis(int i) {return segs[i];}
	const rSeg1d &axis(int i) const {return segs[i];}
	inline const rSeg1d & operator[] (int i) const {return segs[i];}
	VectorNd getCenter(void) const {
		return 0.5*(getMin()+getMax());
	}
	/// Return the minimum vector
	VectorNd getMin(void) const 
	{
		return VectorNd(segs[0].getMin(),
			segs[1].getMin());
	}
	/// Return the maximum vector
	VectorNd getMax(void) const 
	{
		return VectorNd(segs[0].getMax(),
			segs[1].getMax());
	}
	/// Return the width of us
	T getWidth(void) const {return segs[0].getLength();}
	/// Return the height of us
	T getHeight(void) const {return segs[1].getLength();}
	
	enum {nHalfspaces=4};
	/// Get the i'th halfspace of this box.
	inline Halfspace2d getHalfspace(int i) const {
		Halfspace2d h;
		switch (i) {
		case 0: h.n=Vector2d( 1, 0); h.d=-axis(0).getMin(); break;
		case 1: h.n=Vector2d(-1, 0); h.d= axis(0).getMax(); break;
		case 2: h.n=Vector2d( 0, 1); h.d=-axis(1).getMin(); break;
		case 3: h.n=Vector2d( 0,-1); h.d= axis(1).getMax(); break;
		};
		return h;
	}
	enum {nCorners=4};
	/// Return the i'th corner.  0=xy, 1=Xy, 2=xY, 3=XY
	inline VectorNd getCorner(int i) const {
		return VectorNd(
			(i&1)?segs[0].getMax():segs[0].getMin(),
			(i&2)?segs[1].getMax():segs[1].getMin());
	}
	
	void add(const VectorNd &b) {
		segs[0].add(b[0]); segs[1].add(b[1]);
	}
	void add(const bboxNd &b) {
		segs[0].add(b.segs[0]); segs[1].add(b.segs[1]);
	}
	bboxNd getUnion(const bboxNd &b) {
		return bboxNd(segs[0].getUnion(b.segs[0]),
			segs[1].getUnion(b.segs[1]));
	}
	bboxNd getIntersection(const bboxNd &b) {
		return bboxNd(segs[0].getIntersection(b.segs[0]),
			segs[1].getIntersection(b.segs[1]));
	}
	//Interior or boundary (closed interval)
	bool intersects(const bboxNd &b) const {
		return segs[0].intersects(b.segs[0])&&
		       segs[1].intersects(b.segs[1]);
	}
	//Interior only (open interval)
	bool intersectsOpen(const bboxNd &b) const {
		return segs[0].intersectsOpen(b.segs[0])&&
		       segs[1].intersectsOpen(b.segs[1]);
	}
	//Interior or boundary (closed interval)
	bool contains(const VectorNd &b) const {
		return segs[0].contains(b[0])&&
		       segs[1].contains(b[1]);
	}
	//Interior or boundary (closed interval)
	bool contains(const bboxNd &b) const {
		return segs[0].contains(b.segs[0])&&
		       segs[1].contains(b.segs[1]);
	}
	//Interior only (open interval)
	bool containsOpen(const VectorNd &b) const {
		return segs[0].containsOpen(b[0])&&
		       segs[1].containsOpen(b[1]);
	}
	//Interior or left endpoint (half-open interval)
	bool containsHalf(const VectorNd &b) const {
		return segs[0].containsHalf(b[0])&&
		       segs[1].containsHalf(b[1]);
	}
	void empty(void) {
		for (int i=0;i<N;i++) segs[i].empty();
	}
	void infinity(void) {
		for (int i=0;i<N;i++) segs[i].infinity();
	}
	bool isEmpty(void) const {
		for (int i=0;i<N;i++) if (segs[i].isEmpty()) return true;
		return false;
	}
	
	void io(osl::io::Serializer &s) {
		VectorNd min=getMin(), max=getMax();
		IO(min); IO(max);
		for (int i=0;i<N;i++) segs[i].setMinMax(min[i],max[i]);
	}
};

typedef bbox2dT<int,Point> Bbox2i;
typedef bbox2dT<double,osl::Vector2d> Bbox2d;

};

IO_CLASS_ALIAS(osl::Bbox2d,Bbox2d,0);



#endif
