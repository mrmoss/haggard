/**
Georeferenced coordinate manipulation.
  Orion Sky Lawlor, olawlor@acm.org, 2003/4/21
*/
#ifndef __OSL_GEO_H
#define __OSL_GEO_H
#include "osl/bbox2d.h"
#include "osl/matrix2d.h"
#include "osl/graphics_util.h" /* for Rect */
#include "osl/serializer.h"
#include "osl/vector2d_io.h" /* because geo program read/write vector2d's */
#include "osl/fast_math.h"

namespace osl {

typedef osl::graphics2d::Rect Rect;

/**
Describes the coordinates of a georeferenced raster image.
*/
class GeoImage {
public:
	Vector2d pixInv; ///< Pixels per map unit; == 1.0/pixSize [pixel/m]
	inline void setupPixInv(void) { pixInv=Vector2d(1.0/pixelSize.x,1.0/pixelSize.y);}
	
	Vector2d origin; ///< Map coordinates of pixel (0,0) [m]
	Vector2d pixelSize; ///< Map coordinates per pixel [m/pixel]
	int width,height; ///<  Dimensions of described image [pixels]
	
	/// Create an uninitialized GeoImage
	GeoImage() {}
	
	/// Create a georeferenced image with this origin, pixel size, and dimensions
	GeoImage(const Vector2d &origin_,const Vector2d &pixelSize_,int w,int h) 
		:origin(origin_),pixelSize(pixelSize_),width(w),height(h)
	{ setupPixInv(); }
	
	/// Create a georeferenced image with this bounding box and pixel size.
	///  Boundary is the number of pixels to add around the box's boundary.
	GeoImage(const Bbox2d &box,double pixelSize,double boundary=0.0,bool doFlip=true);
	
	/// Read a .geo file to get our projection properties.
	///  The .geo format is:
	///    origin=Vector2d(<origin x,y>)
	///    pixelSize=Vector2d(<pixel size x,y>)
	///    width=<width> 
	///    height=<height>
	GeoImage(const char *baseName);
	
	/// Save our projection properties to this baseName's .geo file.
	void write(const char *baseName);
	
	void io(osl::io::Serializer &s);

	/// Flip this image along the Y axis.
	void flip(void) {
		origin=mapFmPixel(0,height);
		pixelSize.y=-pixelSize.y;
		setupPixInv();
	}

	inline Point getSize(void) const { return Point(width,height); }
	inline Bbox2d getBox(void) const { 
		return Bbox2d(mapFmPixel(0,0),mapFmPixel(width,height));
	}
	
	/// Return the map coordinates of pixel (x,y).
	///  E.g., pixel (0.0,0.0) is considered to be the origin.
	inline Vector2d mapFmPixel(double x,double y) const {
		return origin+Vector2d(pixelSize.x*x,pixelSize.y*y);
	}
	inline Vector2d mapFmPixel(const Vector2d &v) const {
		return mapFmPixel(v.x,v.y);
	}
	inline Vector2d mapFmPixel(const Point &p) const {
		return mapFmPixel(p.x,p.y);
	}
	
	/// Return the map coordinates of the *center* of this pixel.
	inline Vector2d mapFmPixelCenter(const Point &p) const {
		return mapFmPixel(p.x+0.5,p.y+0.5);
	}
	
	
	/// Return the pixel coordinates corresponding to these map coordinates:
	inline Point pixelFmMap(const Vector2d &map) const {
		return Point(
			fastFloor((map.x-origin.x)*pixInv.x),
			fastFloor((map.y-origin.y)*pixInv.y)
		);
	}
	inline Vector2d pixelFmMapd(const Vector2d &map) const {
		return Vector2d(
			(map.x-origin.x)*pixInv.x,
			(map.y-origin.y)*pixInv.y
		);
	}
	inline double pixelFmMap(double v) const {return v*pixInv.x;}
	inline Point clipPixel(Point p) const {
		if (p.x<0) p.x=0;
		if (p.y<0) p.y=0;
		if (p.x>=width) p.x=width-1;
		if (p.y>=height) p.y=height-1;
		return p;
	}
	inline bool contains(const Vector2d &map) const {
		Vector2d p=pixelFmMapd(map);
		return (p.x>=0) && (p.y>=0) && (p.x<width-1) && (p.y<height-1);
	}
	
	/**
	  Return the smallest pixel Rect that contains both 
	  these map locations, clipped to our bounds.
	 */
	Rect pixelRectFmMap(const Vector2d &a,const Vector2d &b) const;
	Rect pixelRectFmMap(const Bbox2d &b) const { return pixelRectFmMap(b.getMin(),b.getMax()); }
	Rect pixelRectFmMapNoclip(const Vector2d &a,const Vector2d &b) const;
	Rect pixelRectFmMapNoclip(const Bbox2d &b) const { return pixelRectFmMapNoclip(b.getMin(),b.getMax()); }
};


/// A 2D georeferenced height image.
class GeoHeight : public GeoImage {
	typedef GeoImage super;
	float *data;
	void allocate(void) {
		data=new float[width*height];
	}
public:
	/// Make a new blank image with these coordinates.
	explicit GeoHeight(const GeoImage &geo_) :super(geo_) { 
		allocate(); 
	}
	
	/// Read in a height image from this file & .geo
	GeoHeight(const char *fName);
	~GeoHeight() {delete[] data;}
	
	/// Write this height image to this file & .geo
	///  This is a native-format binary float file.
	void write(const char *fName);
	
	/// Return the elevation of pixel (x,y)
	inline float &at(int x,int y) {return data[y*width+x];}
	inline float &at(const Point &p) {return at(p.x,p.y);}
	inline float at(int x,int y) const {return data[y*width+x];}
	inline float at(const Point &p) const {return at(p.x,p.y);}
	
	/// Return the elevation of this map location
	float atMap(const Vector2d &map) const {
		return at(pixelFmMap(map));
	}
};


/**
  Converts between various 2D coordinate systems, like map projections.
 */
class CoordMap2d {
public:
	virtual ~CoordMap2d();
	
	/// Project this location into our coordinate system.
	virtual Vector2d map(const Vector2d &v) const =0;
	/// Project this size into our coordinate system.
	virtual double map(double v) const =0;
	
	/** 
	  Map this (absolute radian) orientation angle. 
	*/
	virtual double map_ang(const Vector2d &cen,double ang) {
		return Polar2d(
			map(Vector2d(Polar2d(1.0,ang))+cen)-map(cen)
		).theta;
	}
};

/// Convert incoming map projection coordinates to pixels:
class GeoToPixelsMap2d : public CoordMap2d {
	GeoImage g;
public:
	GeoToPixelsMap2d(const GeoImage &g_) :g(g_) {}
	Vector2d map(const Vector2d &v) const;
	double map(double v) const;
};

/// Perform coordinate conversion based on a matrix.
class MatrixMap2d : public CoordMap2d {
	Matrix2d m;
	double scale;
public:
	MatrixMap2d(void) {setMatrix(Matrix2d(1.0));}
	MatrixMap2d(const Matrix2d &m_) {setMatrix(m_);}
	/// Linearize this map about this point.
	MatrixMap2d(const CoordMap2d &map,Vector2d cen=Vector2d(0,0));
	void setMatrix(const Matrix2d &m_);
	
	/// Apply this matrix before mapping:
	void applyBefore(const Matrix2d &pre);
	/// Apply this matrix after mapping:
	void applyAfter(const Matrix2d &post);
	
	Vector2d map(const Vector2d &v) const;
	double map(double v) const;
};

/**
  Allocate a new coordinate conversion object to perform
  this coordinate conversion.
*/
CoordMap2d *makeCoordMap(const char *convDesc);

/// Copy src into dest, respecting map coordinates.
///  Copies zero where there is no src.
void copy(const GeoHeight &src,GeoHeight &dest);

/**
Read a bounding box from this file.  The format is:
   <min x> <min y> <extent x> <extent y>
All are projection coordinates, in meters.
*/
Bbox2d readRegion(const char *regionFileName);

};


IO_CLASS_ALIAS(osl::GeoImage,GeoImage,0)

#endif
