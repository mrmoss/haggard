/**
Coordinate systems, especially for raster images.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-07 (Public Domain)
*/
#ifndef __CYBERALASKA_COORDS_H
#define __CYBERALASKA_COORDS_H

#include "../cyberalaska/vec3.h"
#include "../rasterCV/raster.h" /* for image_size */

namespace cyberalaska {


using rasterCV::image_size;

/**
 A coordinate system, usually used for converting between
 real-world and texture or pixel coordinates.
 
 Supports translation and scaling, but not rotation or skew.
*/
class coords {
	/// This is the world coordinates of our point (0,0,0)
	vec3 origin;
	/// This is the world coordinates size of our image
	vec3 size; 
	/// This is 1.0/size
	vec3 isize;
public:
	coords(vec3 origin_=vec3(0.0),vec3 size_=vec3(1.0))
		:origin(origin_), size(size_) 
	{
		for (int axis=0;axis<3;axis++) isize[axis]=1.0/size[axis];
	}

// To pixel coordinates:
	/** Return the pixel coordinates for this world coordinate */
	vec3 pixel_from_world(const vec3 &w,const image_size &sz) const {
		return pixel_from_texture(texture_from_world(w),sz);
	}
	vec3 pixel_from_texture(const vec3 &t,const image_size &sz) const {
		return vec3(t.x*sz.x_range.hi,t.y*sz.y_range.hi,0.0);
	}

// To texture coordinates:
	/** Return the texture coordinates location of this pixel. */
	vec3 texture_from_pixel(const vec3 &p,const image_size &sz) const {
		return vec3(p.x*(1.0/sz.x_range.hi),p.y*(1.0/sz.y_range.hi),p.z);
	}
	vec3 texture_from_pixel(double x,double y,const image_size &sz) const {
		return texture_from_pixel(vec3(x,y,0.0),sz);
	}
	vec3 texture_from_world(const vec3 &w) const {
		return (w-origin)*isize;
	}

// To world coordinates:
	/** Return the world coordinates location of this pixel. */
	vec3 world_from_pixel(double x,double y,const image_size &sz) const {
		return world_from_texture(texture_from_pixel(x,y,sz));
	}
	/** Return the world coordinates location of this pixel.  z is unchanged. */
	vec3 world_from_pixel(const vec3 &p,const image_size &sz) const {
		return world_from_texture(texture_from_pixel(p,sz));
	}
	/** Return the world coordinates location of this texture coordinates point. */
	vec3 world_from_texture(const vec3 &t) const {
		return origin + size*t;
	}

// Containment:
	/** Return true if this world_coodinates point lies inside our bounds. */
	bool contains_world(const vec3 &w) const {
		return contains_texture(texture_from_world(w));
	}
	/** Return true if this texture coordinates point lies inside our bounds. */
	bool contains_texture(const vec3 &t) const {
		for (int axis=0;axis<3;axis++) 
		{
			if (t[axis]<0 || t[axis]>=1.0)
				return false;
		}
		return true;
	}
	/** Return true if this pixel coordinates point lies inside our bounds. */
	bool contains_pixel(const vec3 &p,const image_size &sz) const {
		return sz.contains(p.x,p.y);
	}
	
	template <class PUP>
	void pup(PUP &p) {
		pup(p,origin,"origin");
		pup(p,size,"size");
		for (int axis=0;axis<3;axis++) isize[axis]=1.0/size[axis];
	}	
};




}; /* end namespace */

#endif /* defined (this header) */
