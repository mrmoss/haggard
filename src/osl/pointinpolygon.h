/*
Orion's Standard Library
Orion Sky Lawlor, olawlor@acm.org, 2004/11/17
*/
#ifndef __OSL_POINTINPOLYGON_H
#define __OSL_POINTINPOLYGON_H

#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif
#include <vector>

namespace osl { 

/**
 Determine if a point lies inside a 2D polygon.
 Likely has horrible roundoff problems.
*/
class PointInPolygon {
	std::vector<Vector2d> pts; // endpoints of lines
	std::vector<Halfspace2d> halfs; // halfspaces of lines
	Bbox2d box;
public:
	PointInPolygon(const std::vector<Vector2d> &pts_)
		{init(&pts_[0],pts_.size());}
	PointInPolygon(const Vector2d *p,int np)
		{init(p,np);}
	void init(const Vector2d *p,int np)
	{
		box.empty();
		for (int i=0;i<np;i++) {
			pts.push_back(p[i]);
			Vector2d prev=p[i], next=p[(i+1)%np];
			halfs.push_back(Halfspace2d(prev,next,next+Vector2d(1.0e5,0)));
			box.add(prev);
		}
	}
	const Bbox2d &getBox(void) const {return box;}
	const std::vector<Vector2d> &getPoints(void) const {return pts;}
	
	/**
	  Return true if this polygon contains this point.
	  Uses even-odd fill criterion.  Counts boundary as a hit.
	*/
	bool contains(const Vector2d &p) const {
		if (!box.contains(p)) return false; /* misses bounding box */
		
		const static double epsilon=1.0e-10;
		int winding=0;
		/* count crossings of left-facing horizontal ray from point */
		unsigned int i,np=pts.size();
		for (i=0;i<np;i++) {
			Vector2d prev=pts[i], next=pts[(i+1)%np];
			double dy=next.y-prev.y;
			seg1d s(prev.x); s.add(next.x);
			if (dy>0) { /* upward-travelling */
				if (p.y<prev.y || p.y>=next.y) continue;
				double side=halfs[i].side(p);
				if (side>epsilon) winding++; /* increase winding count */
				else if (side>=-epsilon && s.contains(p.x)) return true; /* hits edge */
			} else if (dy<0) { /* downward-travelling */
				if (p.y>prev.y || p.y<=next.y) continue;
				double side=halfs[i].side(p);
				if (side>epsilon) winding--; /* decrease winding count */
				else if (side>=-epsilon && s.contains(p.x)) return true; /* hits edge */
			} else /* dy==0 */ { /* horizontal */
				if (fabs(p.y-prev.y)>epsilon) continue; 
				if (s.contains(p.x)) return true; /* hits edge */
			}
		}
		
		return (winding&1)==1;
	}
};

}; /*end namespace*/

#endif /* def(thisHeader) */
