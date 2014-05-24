/**
  2D Voronoi diagram and Delaunay triangulation 
  computation interface routines.
  
  Orion Sky Lawlor, olawlor@acm.org, 2004/10/21
*/
#ifndef __OSL_VORONOI_H
#define __OSL_VORONOI_H

#include "osl/vector2d.h"
#include <vector>

namespace delaunay {
typedef osl::Vector2d Point;

typedef double float_t;

/**
 Structure used both for sites (Vornoi vertices)
  and for Delaunay vertices 
*/
struct Site	{
	/// x,y coordinates of this point.
	Point	coord;
	/// 0-based serial number for this point.
	int		sitenbr;
	/// Reference count (for dynamic allocation)
	int		refcnt;
	
	operator int () const {return sitenbr;}
};

/**
  Represents an edge in the Voronoi diagram.
*/
struct Edge {
	/// Coefficients of line:
	///    a x + b y = c
	float_t		a,b,c;
	/// Endpoints of line.  May be NULL for edges at infinity.
	///  If these are non-NULL, they are Voronoi vertices.
	Site 	*ep[2];
	/// Regulating points (controlling points).
	///   These are always non-NULL Delaunay vertices.
	Site	*reg[2];
	/// 0-based serial number of this edge.
	int		edgenbr;
};

/**
 Accepts Delaunay and Voronoi components as they
 are created during a computation below.
 
 The Delaunay triangulation is a good triangulation 
 of the original point set.  The Voronoi diagram consists
 of one region per original point, containing the parts of
 the plane that are closer to that original points than any
 other original point.  The Voronoi diagram is represented 
 as a set of vertices and edges, some of which may be infinite.
*/
class Consumer {
public:
	/// An line equidistant between two source points.
	///   Endpoints are not yet set.
	virtual void voronoi_line(delaunay::Edge *v);
	
	/// A line segment dividing the region controlled by 
	///  two source points.  Both endpoints are set (if they exist)
	virtual void voronoi_edge(delaunay::Edge *v);
	
	/// A point equidistant from three or more source points.
	///   s->sitenbr is *not* an input vertex number;
	///   voronoi vertex numbers start from 0.
	virtual void voronoi_vertex(delaunay::Site *s);
	
	/// One of the input points.  pts[s->sitenbr]==s->coord
	virtual void delaunay_vertex(delaunay::Site *s);
	
	/// A triangle in the Delaunay triangulation.
	///  Three zero-based delaunay vertex indices.
	virtual void delaunay_triangle(int v1,int v2,int v3);
	
	virtual ~Consumer();
};

void compute(const std::vector<Point> &pts,Consumer &dest);

};

#endif
