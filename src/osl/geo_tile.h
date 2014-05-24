/**
Georeferenced image tile manipulation.
  Orion Sky Lawlor, olawlor@acm.org, 2003/9/1
*/
#ifndef __OSL_GEO_TILE_H
#define __OSL_GEO_TILE_H
#include "osl/geo.h"
#include "osl/graphics.h"

namespace osl {

/// Describe the location and orientation of a set of image tiles.
///   A rectangular set of image tiles can be seen as a big image,
///   composed of tiles instead of pixels.
///
/// Although the layout of the tiles is raster-like, the tiles 
///   themselves need not be raster.
class TileSet : public GeoImage {
	/// The directory storing our tile data.
	String dirName;

	/// Return the name of the directory containing row Y.
	String tileYDir(int y) const;
public:
	/// Build a new set of tiles for this region and pixel size.
	TileSet(const Bbox2d &box,double pixelSize,String dirName_="tiles") 
		:GeoImage(box,pixelSize,0,false), dirName(dirName_)
	{}
	/// Build a new set of tiles for this region.
	TileSet(const GeoImage &gi,String dirName_="tiles") 
		:GeoImage(gi), dirName(dirName_)
	{}
	/// Read the tileset from this directory.
	TileSet(String dirName_="tiles") 
		:GeoImage((dirName_+".geo").c_str()), dirName(dirName_)
	{}

	/// Return the name of the tile directory.
	const String &baseName(void) const {return dirName;}
	
	/// Return the name of the directory containing tile p.
	String directory(const Point &p) const;
	
	/// Return true if tile p exists.
	bool exists(const Point &p) const;
	
	/// Return the name of the directory for tile p, which will be created.
	String createDirectory(const Point &p) const;
	
	/// Look up this filename for the p'th tile.
	osl::io::File file(const Point &p,const char *fileName);
	
	/// Write out our tileSet geometry to the our directory.
	void write(void) const;
};

class TilesImageList;

/// Renders a tileSet.
class TileSetRenderer : public TileSet 
{
	TilesImageList *img; //Names of image tiles, in raster order (width*height of them)
	bool tileYdown; // if true, tile image Y axes point down
public:
	TileSetRenderer(String dirName_="tiles");
	~TileSetRenderer();
	
	/// Draw this tileset into the given graphics, where
	/// screen is the coordinate frame for the display.
	void paint(osl::graphics2d::Graphics &dest,const osl::GeoImage &screen);
};

};

#endif
