/*
  Classes for representing USGS Digital Raster Graphs (DRG),
  which are scanned georeferenced images of USGS topo maps.
  
  Orion Sky Lawlor, olawlor@acm.org, 2007/09/28 (Public Domain)
*/
#ifndef __OSL_DRG_IMAGE_H
#define __OSL_DRG_IMAGE_H

#include "osl/geo.h"
#include "osl/io.h"
#include "osl/color.h"
#include "osl/raster.h"

using osl::graphics2d::Color;

class proj_parameters {
public:
	/* UTM zone number (for example, zone 6 for interior Alaska) */
	int utm_zone; 
	/* Hemisphere (default 'N' for northern hemisphere) */
	char hem;
	/* Eccentricity of earth */
	double ecc;
	/* Equatorial, polar earth radius */
	double re_major, re_minor;

	proj_parameters();
};


/* One DRG topographic image. */
class drg_image {
public:
	drg_image(const char *imgName,double lat,double lon);
	~drg_image();
	
	// Return the color of our image at this location
	Color get_color(double lat,double lon);
	
	// Return our coordinate system
	const osl::GeoImage &get_geo(void) {return geo;}
	/* Our UTM output projection coordinates */
	proj_parameters proj;
	
	// Return our filename
	const char *get_name(void) const {return imgFile.c_str();}
private:
	/* This is our UTM coordinate system */
	osl::GeoImage geo;
	/* This is our raster image's filename. */
	std::string imgFile;
	/* If non-NULL, this is our raster image data. */
	osl::graphics2d::RgbaRaster *img;
	void read_img(void);
};


/* A 2D array of DRG images. Grid is regular, but sparse--only a random set of images are present. */
class drg_grid {
public:
	class drg_index {
	public:
		int x,y;
		bool operator==(const drg_index &b) const {
			return x==b.x && y==b.y;
		}
		/* Need a less-than to use drg_index in a std::map. */
		bool operator<(const drg_index &b) const {
			if (y<b.y) return true;
			if (y>b.y) return false;
			return x<b.x;	
		}
	};
	
	/* Add this image at this lat/lon */
	void add_image(drg_image *img,double lat,double lon);
	
	/* Return the image that occupies this lat/lon, or NULL if none exists. */
	drg_image *get_image(double lat,double lon);

	drg_grid(double latscale_,double lonscale_) 
		:latscale(latscale_), lonscale(lonscale_) {}

private:
	/* These scale factors convert (lat,lon), in degrees,
	  into an integer DRG grid index. */
	double latscale, lonscale;
	/* This map converts DRG grid indexes into DRG images.
	  If there's nothing in this map, there is no grid location there. */
	typedef std::map<drg_index,drg_image *> map_t;
	map_t map;
	
	/* Return the graph index that would be used by this lat,lon in degrees. */
	drg_index make_index(double lat,double lon);
};

/* A set of drg grid levels */
class drg_gridset {
public:
	drg_gridset(void);
	// Add this image to our set
	void add(const char *fileName);
	// Return the color of our image at this location
	Color get_color(double lat,double lon,double resolution);
	
	// Return an image for this map projection.
	osl::graphics2d::RgbaRaster render(const osl::GeoImage &geo);
	
	// Return our map-projection-coordinates bounding box
	const osl::Bbox2d &get_box(void) const {return box;}
	
private:
	enum  {nGrids=6};
	drg_grid *grids[nGrids]; 
	osl::Bbox2d box;
};

#endif
