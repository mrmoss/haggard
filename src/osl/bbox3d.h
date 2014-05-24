/*
Orion's Standard Library
Orion Sky Lawlor, 11/3/1999
NAME:		osl/vector3d.h

3D Bounding Box routines.  Exists so that the segment
utilities (vector1d) don't have to be included everywhere.
*/
#ifndef __OSL_BBOX3D_H
#define __OSL_BBOX3D_H

#ifndef __OSL_VECTOR1D_H
#  include "osl/vector1d.h"
#endif
#ifndef __OSL_VECTOR3D_H
#  include "osl/vector3d.h"
#endif

namespace osl {

template <class T,class VectorNd>
class bbox3dT {
	enum {N=3};
	typedef bbox3dT<T,VectorNd> bboxNd;
	typedef seg1dT<T> rSeg1d;
	/// Spans for x (0), y (1), and z (2)
	seg1dT<T> segs[N];
public:   
	bbox3dT() {}
	bbox3dT(const rSeg1d &x,const rSeg1d &y,const rSeg1d &z)
		{segs[0]=x; segs[1]=y; segs[2]=z;}
	bbox3dT(const VectorNd &a,const VectorNd &b,const VectorNd &c)
	{
		segs[0].init(a[0],b[0],c[0]);
		segs[1].init(a[1],b[1],c[1]);
		segs[2].init(a[2],b[2],c[2]);
	}
	bbox3dT(const VectorNd &a,const VectorNd &b)
	{
		segs[0].init(a[0],b[0]);
		segs[1].init(a[1],b[1]);
		segs[2].init(a[2],b[2]);
	}
	
	void print(const char *desc=0) const;
	
	inline void shift(const VectorNd &by) {
		segs[0].shift(by.x);
		segs[1].shift(by.y);
		segs[2].shift(by.z);
	}
	
	rSeg1d &axis(int i) {return segs[i];}
	const rSeg1d &axis(int i) const {return segs[i];}
	VectorNd getCenter(void) const {
		return 0.5*(getMin()+getMax());
	}
	double getVolume(void) const {
		return  segs[0].getLength()*
			segs[1].getLength()*
			segs[2].getLength();
	}
	
	enum {nHalfspaces=6};
	/// Get the i'th halfspace of this box.
	inline Halfspace3d getHalfspace(int i) const {
		Halfspace3d h;
		switch (i) {
		case 0: h.n=Vector3d( 1, 0, 0); h.d=-axis(0).getMin(); break;
		case 1: h.n=Vector3d(-1, 0, 0); h.d= axis(0).getMax(); break;
		case 2: h.n=Vector3d( 0, 1, 0); h.d=-axis(1).getMin(); break;
		case 3: h.n=Vector3d( 0,-1, 0); h.d= axis(1).getMax(); break;
		case 4: h.n=Vector3d( 0, 0, 1); h.d=-axis(2).getMin(); break;
		case 5: h.n=Vector3d( 0, 0,-1); h.d= axis(2).getMax(); break;
		};
		return h;
	}
	
	enum {nCorners=8};
	/// Return the i'th corner.  0=xyz, 1=Xyz, 2=xYz, ... 7=XYZ
	inline VectorNd getCorner(int i) const {
		return VectorNd(
			(i&1)?segs[0].getMax():segs[0].getMin(),
			(i&2)?segs[1].getMax():segs[1].getMin(),
			(i&4)?segs[2].getMax():segs[2].getMin());
	}
	
	void add(const VectorNd &b) {
		segs[0].add(b[0]); segs[1].add(b[1]); segs[2].add(b[2]);
	}
	void add(const bboxNd &b) {
		segs[0].add(b.segs[0]); segs[1].add(b.segs[1]); segs[2].add(b.segs[2]);
	}
	bboxNd getUnion(const bboxNd &b) {
		return bboxNd(segs[0].getUnion(b.segs[0]),
			segs[1].getUnion(b.segs[1]),
			segs[2].getUnion(b.segs[2]));
	}
	bboxNd getIntersection(const bboxNd &b) {
		return bboxNd(segs[0].getIntersection(b.segs[0]),
			segs[1].getIntersection(b.segs[1]),
			segs[2].getIntersection(b.segs[2]));
	}
	//Interior or boundary (closed interval)
	bool intersects(const bboxNd &b) const {
		return segs[0].intersects(b.segs[0])&&
		       segs[1].intersects(b.segs[1])&&
		       segs[2].intersects(b.segs[2]);
	}
	//Interior only (open interval)
	bool intersectsOpen(const bboxNd &b) const {
		return segs[0].intersectsOpen(b.segs[0])&&
		       segs[1].intersectsOpen(b.segs[1])&&
		       segs[2].intersectsOpen(b.segs[2]);
	}
	//Interior or boundary (closed interval)
	bool contains(const VectorNd &b) const {
		return segs[0].contains(b[0])&&
		       segs[1].contains(b[1])&&
		       segs[2].contains(b[2]);
	}
	//Interior only (open interval)
	bool containsOpen(const VectorNd &b) const {
		return segs[0].containsOpen(b[0])&&
		       segs[1].containsOpen(b[1])&&
		       segs[2].containsOpen(b[2]);
	}
	//Interior or left endpoint (half-open interval)
	bool containsHalf(const VectorNd &b) const {
		return segs[0].containsHalf(b[0])&&
		       segs[1].containsHalf(b[1])&&
		       segs[2].containsHalf(b[2]);
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
	/// Return the minimum vector
	VectorNd getMin(void) const 
	{
		return VectorNd(segs[0].getMin(),
			segs[1].getMin(),
			segs[2].getMin());
	}
	/// Return the maximum vector
	VectorNd getMax(void) const 
	{
		return VectorNd(segs[0].getMax(),
			segs[1].getMax(),
			segs[2].getMax());
	}
	
#ifdef __CK_PUP_H
	void pup(PUP::er &p) {p|segs[0]; p|segs[1]; p|segs[2];}
#endif
};

typedef bbox3dT<double,osl::Vector3d> Bbox3d;

};

#endif
