/*
Parses a trivially simple ASCII drawing format, ".daf".

  Orion Sky Lawlor, olawlor@acm.org, 2003/4/21
*/
#ifndef __OSL_DAF_H
#define __OSL_DAF_H

#include <stdio.h>
#include "osl/vector2d.h"
#include "osl/bbox2d.h"
#include "osl/geo.h"
#include "osl/graphics.h"
#include "osl/storedpath.h"

namespace osl { namespace daf {

using osl::graphics2d::Color;

/// Accepts the graphical components of a .daf file.
///  This is the central interface of .daf processing.
class DrawingDest {
public:
	/// A many-segmented line.  Often pts[0]==pts[n-1].
	/// Pts may be trashed by this routine.
	virtual void polyline(Vector2d *pts,int n) =0;
	
	/// A line segment, from start to end.
	virtual void line(const Vector2d &start,const Vector2d &end) =0;
	
	/// An arc of a circle, with this center, radius, and radian angles.
	virtual void arc(const Vector2d &cen,double r,double startAng,double endAng) =0;
	
	/// A text string, with this bottom-left and character height.
	virtual void text(const Vector2d &bottomLeft,double height,const char *str) =0;	
	
	/// A predefined image macro, with this center and orientation (radians).
	virtual void insert(const Vector2d &center,double orientAng,const char *name) =0;
	
	/// Set the color to be used for future images.
	virtual void color(const Color &c) =0;		

	// for whining compilers
	virtual ~DrawingDest();
};

/// Applies a CoordMap2d to these objects before passing on to dest.
class MapDest : public DrawingDest {
	CoordMap2d &m;
	bool mapInverts;
	DrawingDest &dest;
public:
	MapDest(CoordMap2d &m_,DrawingDest &dest_);
	
	virtual void polyline(Vector2d *pts,int n);
	virtual void line(const Vector2d &start,const Vector2d &end);
	virtual void arc(const Vector2d &cen,double r,double startAng,double endAng);
	virtual void text(const Vector2d &bottomLeft,double height,const char *str);
	virtual void insert(const Vector2d &center,double orientAng,const char *name);
	virtual void color(const Color &c);	
};

/// Ignores all incoming input.  Useful for inheriting a filter class.
class IgnoreDest : public DrawingDest {
public:
	virtual void polyline(Vector2d *pts,int n);
	virtual void line(const Vector2d &start,const Vector2d &end);
	virtual void arc(const Vector2d &cen,double r,double startAng,double endAng);
	virtual void text(const Vector2d &bottomLeft,double height,const char *str);
	virtual void insert(const Vector2d &center,double orientAng,const char *name);
	virtual void color(const Color &c);	
};

/**
  Use a StoredPath to accumulate intermediate polylines into
  batches.
  
  This is needed to properly render closed shapes drawn with polylines.
  The polylines arrive separately, but must be rendered together.
  
  Ignores everything but closed polylines.
*/
class StoredPathDest : public IgnoreDest 
{
public:
	/// The path segments seen so far.
	osl::graphics2d::StoredPath path;
	
	StoredPathDest();
	virtual ~StoredPathDest();
	
	/// Add this segment to our stored path.
	virtual void polyline(Vector2d *pts,int n);
};

/**
  Draws incoming components to a Graphics.
  Uses a StoredPath to accumulate intermediate polylines;
  this is needed to properly poke holes in the polylines,
  which arrive separately but must be rendered together.
*/
class GraphicsDest : public StoredPathDest 
{
	osl::graphics2d::Graphics &dest;
	osl::graphics2d::GraphicsState gs;
	bool m_doFill;
public:
	GraphicsDest(const osl::graphics2d::GraphicsState &gs_,
		osl::graphics2d::Graphics &dest_,bool doFill=true);
	virtual ~GraphicsDest();
	
	/// Write all stored output to the Graphics device.
	void flush(void);
	
	virtual void polyline(Vector2d *pts,int n);
	virtual void line(const Vector2d &start,const Vector2d &end);
	virtual void arc(const Vector2d &cen,double r,double startAng,double endAng);
	virtual void text(const Vector2d &bottomLeft,double height,const char *str);
	virtual void insert(const Vector2d &center,double orientAng,const char *name);
	virtual void color(const Color &c);	
};

// Return the side this point lies from this box.
//  0-- interior
//  1, 2 -- horiz. sides
//  3, 4 -- vert. sides
inline int getSide(const Vector2d &v,const Bbox2d &b) {
	if (v[0]<b.axis(0).getMin()) return 1;
	if (v[0]>b.axis(0).getMax()) return 2;
	if (v[1]<b.axis(1).getMin()) return 3;
	if (v[1]>b.axis(1).getMax()) return 4;
	return 0;
}

/**
  Splits up the .daf components to a regular array
  (indexed by "tile") of sub-objects (in dests).
*/
class TileDaf : public osl::daf::DrawingDest {
	osl::GeoImage &tile; ///< returns tile number given map coords
	Color srcColor; ///< current source color 
	std::vector<Color> destColors;///< last written color for each tile
	osl::daf::DrawingDest **dests; ///< drawing destinations for each tile
	osl::Vector2d enlarge; ///< Distance to enlarge each side of box by
	
	/// Prepare to write to this tile.
	osl::daf::DrawingDest &destTile(int tx,int ty) {
		int tileNo=tx+ty*tile.width; 
		if (destColors[tileNo]!=srcColor) {
			destColors[tileNo]=srcColor;
			dests[tileNo]->color(srcColor);
		}
		return *dests[tileNo]; 
	}
	
	void clipPolyline(osl::daf::DrawingDest &d,int tx,int ty,Vector2d *pts,int n);
public:
	/**
	  toTile converts map coordinates into tile coordinates.
	  dests is a toTile.width * toTile.height raster-order
	  array of drawing consumers.  Incoming objects are divided
	  by location and scattered to these consumers.  Out-of-bounds
	  objects are discarded.
	*/
	TileDaf(osl::GeoImage &toTile,osl::daf::DrawingDest **dests);
	virtual ~TileDaf();
	
	virtual void polyline(Vector2d *pts,int n);
	virtual void line(const Vector2d &start,const Vector2d &end);
	virtual void arc(const Vector2d &cen,double r,double startAng,double endAng);
	virtual void text(const Vector2d &bottomLeft,double height,const char *str);
	virtual void insert(const Vector2d &center,double orientAng,const char *name);
	virtual void color(const Color &c);
};


/// Writes components to a FILE.
class FileDest : public DrawingDest 
{
	FILE *f;
	void o(const char *str) {fprintf(f,"%s",str);}
	void o(double d) {fprintf(f,"%.4f",d);}
	void o(const Vector2d &v) {fprintf(f,"%.3f %.3f",v.x,v.y);}
	void o(int n) {fprintf(f,"%d",n);}
public:
	FileDest(FILE *f_) :f(f_) {}
	
	virtual void polyline(Vector2d *pts,int n);
	virtual void line(const Vector2d &start,const Vector2d &end);
	virtual void arc(const Vector2d &cen,double r,double startAng,double endAng);
	virtual void text(const Vector2d &bottomLeft,double height,const char *str);
	virtual void insert(const Vector2d &center,double orientAng,const char *name);
	virtual void color(const Color &c);	
};

/// Compute a bounding box of contents
class BboxDest : public DrawingDest 
{
	DrawingDest *sub;
	Bbox2d box;
	void o(const Vector2d &v) {box.add(v);}
public:
	BboxDest(DrawingDest *sub_=NULL) :sub(sub_) {box.empty();}
	inline const Bbox2d &getBox(void) const {return box;}
	
	virtual void polyline(Vector2d *pts,int n);
	virtual void line(const Vector2d &start,const Vector2d &end);
	virtual void arc(const Vector2d &cen,double r,double startAng,double endAng);
	virtual void text(const Vector2d &bottomLeft,double height,const char *str);
	virtual void insert(const Vector2d &center,double orientAng,const char *name);
	virtual void color(const Color &c);	
};

/// Read this .daf file into this destination.
void read(const char *fName,DrawingDest &dest);


}; };

#endif
