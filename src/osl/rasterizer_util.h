/*
Orion's Standard Library
Orion Sky Lawlor, 12/22/2000
NAME:		osl/raster.h

DESCRIPTION:	C++ Rasterization utilities--
classes for a collection of scanline segments.

*/
#ifndef __OSL_RASTER_UTIL_H
#define __OSL_RASTER_UTIL_H
#include "osl/path.h"
#include <vector>
#include <map>

namespace osl { 

////////////////////////////////////////////////////////////////
/*
For Rasterizing polygons, copying Regions, etc, we maintain a 
list of object edges for each Y-line during scan conversion.
Sorting this list gives the fill pairs, or "scanlineSegment"s.
*/
namespace ru {
	
using osl::graphics2d::Rect;
typedef unsigned int alpha_t;

//Represents the data for a scanline-shape intersection.
class ScanHit {
public:
	typedef unsigned short coord_t; //X coordinates have this type
	
	coord_t x;//X coordinate of intersection
	alpha_t alpha; //Alpha (or other marker data) starting at intersection
	static double alpha2double, double2alpha;
	
	ScanHit() { x=0; alpha=0; }
	ScanHit(int x_,alpha_t alpha_) {
		x=(coord_t)x_;
		alpha=alpha_;
	}
	ScanHit(int x_,double real_alpha) {
		x=(coord_t)x_;
		alpha=(alpha_t)(real_alpha*double2alpha);
	}
};

/// A set of rasterized pixels--
///  Returns a sequence of HitLists.
class Region {
public:
	virtual ~Region();
	
	/**
	 Get an array of hits for this scanline. The call sequence is
	 contorted for speed, so use the convenience class "ScanLine",
	 below, instead of calling this directly.
	 
	  @param dest Possible storage for the output hits.
	  @param len On input, the length of dest.  On output, the 
	             number of hits on this scanline.
	  @param atLeastY On input, this is minimum scanline to return.
	        On output, gives the scanline actually returned, which
		may be greater than the input requested.
	  
	  @return If dest, or a statically allocated array internal to  
	     the region are returned, this scanline is complete.
	     If NULL is returned, the region requires you to allocate
	     at least len ScanHits and call getLine again.
	*/
	virtual const ScanHit *getLine(ScanHit *dest,int &len,int &atLeastY) const =0;
};

/**
 A set of ScanHits on a scanline.  This class wraps
 an efficient, sensible caller interface around Region::getLine.
 The curious-looking but canonical usage of a ScanLine is:
	 <code>
	 for (int y=minHeight;;y++)
	 {
	   ScanLine row(region,y);
	   if (y>=maxHeight) break;
	   
	   for (int i=0;i<row.spans();i++) 
	   if (row[i].alpha!=0) {
	   	int xmin=row[i].x, xmax=row[i+1].x;
		for (int x=xmin;x<xmax;x++) 
		  ... do something with (x,y) and alpha ...
	   }
	 }
	 </code>
*/
class ScanLine {
	/// Number of ScanHits on our line.
	int len;
	/// ScanHits on our line.
	const ScanHit *hits;
	
	/// Number of ScanHits to preallocate.
	enum {shortLen=32};
	/// Preallocated ScanHits
	ScanHit shortHits[shortLen];
	
	/// Dynamically allocated ScanHits, or 0 if none.
	ScanHit *allocHits;
	/// Non-inlined allocation for ScanLine:
	void outOfLineAlloc(const Region &region,int &y); 
	
public:
	inline ScanLine(const Region &region,int &y) {
		len=shortLen;
		allocHits=NULL;
		hits=region.getLine(shortHits,len,y);
		if (hits==NULL)
			outOfLineAlloc(region,y);
	}
	~ScanLine() {
		if (allocHits) delete[] allocHits;
	}
	
	/// Return the number of spans on our line, which 
	///  is one less than the number of ScanHits.
	///  For example, a simple ScanLine consisting of the 
	///  pixels from 100 to 102 (inclusive) would have 1 span,
	///  the first at x==100, the second at x==103.
	inline int spans(void) const {return len-1;}
	
	/// Return the i'th ScanHit on our line.  Each ScanHit
	///  divides the preceeding span from the following span.
	inline const ScanHit &operator[](int i) const {return hits[i];}
};
	   

/// A Rectangular Region
class RectRegion : public Region {
	Rect r;
	ScanHit hits[2];
public:
	RectRegion(const Rect &r_);
	~RectRegion();
	
	virtual const ScanHit *getLine(ScanHit *dest,int &len,int &atLeastY) const;
};

/// Implementation utility for Regions
class ClippingRegion : public Region {
protected:
	Rect r;//Clip boundaries
public:
	/// Create a buffer for this Region
	ClippingRegion(const Rect &r_);
	/// Better call setSize if you use this
	ClippingRegion(void) {}
	
	/// Resize to be the given size
	virtual void setSize(const Rect &r_);
	
//Clip utility functions
	inline int clipX(int v) const {return r.clipX(v);}
	inline int clipY(int v) const {return r.clipY(v);}
	inline bool oobX(int v) const {return r.oobX(v);}
	inline bool oobY(int v) const {return r.oobX(v);}
};



/// Stores a scan conversion of a shape-- hit points are 
/// added in, and spans are read out.
/// In implementation, each y has an x-sorted list of ScanHits.
class ScanConverted : public ClippingRegion {
private:
	typedef ClippingRegion super;
	class sHit;
	mutable sHit *cachedHit;
	
	//Add state:
	
	/// This is the list of hits for one scanline--
	typedef std::map<ScanHit::coord_t,ScanHit> ScanLineList;
	
	/// Y-indexed list of hits
	std::vector<ScanLineList> yLink;
	enum {BIG=0x70000000, LIL=-0x70000000};
	int minY,maxY;//Smallest and largest non-empty links
	
	//Get state:
	int curY; //Next returned Y
	
	ScanConverted(const ScanConverted&); //Don't use these
	void operator=(const ScanConverted&);
public:
	ScanConverted(const Rect &r_);
	virtual ~ScanConverted();
	
	/// Resize this ScanConverted to be the given size
	virtual void setSize(const Rect &r_);
	
	/// Clear stored hit Points.  Call this before rendering.
	void resetAdd(void); 
	
	/// Add a hit Point.  Alpha values must be special "differential" values.
	void add(int x_,int y_,alpha_t alpha_);
	
	/// Sum differential alphas to real alphas.  Must be called before output.
	///  @param fillNZ Use the nonzero winding number fill rule (not even-odd).
	virtual void prepareEnterExit(int fillNZ); 
	
	/// Extract our list of hit points.
	virtual const ScanHit *getLine(ScanHit *dest,int &len,int &atLeastY) const;
};


//--------------------- Rasterization routines -------------------

/// Aliased, 0-or-255 edge rasterizer:
class PolyEdgeSimple : public osl::graphics2d::ShatterDest {
	ScanConverted &sc;
public:
	PolyEdgeSimple(ScanConverted &Nsc) :sc(Nsc) {}
	
	//Add an edge between these vertices
	virtual void line(const Vector2d &start,const Vector2d &end);
};

/// Antialiased edge rasterizer:
class PolyEdgeSmooth : public osl::graphics2d::ShatterDest {
	ScanConverted &sc;
public:
	PolyEdgeSmooth(ScanConverted &Nsc) :sc(Nsc) {}
	
	//Add an edge between these vertices
	virtual void line(const Vector2d &start,const Vector2d &end);
};


}; //end namespace osl::ru
}; //end namespace osl
#endif //__OSL_IMAGE_H

