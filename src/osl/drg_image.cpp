/**
  Read a set of UTM-projected USGS Digital Raster Graphs,
and combine them into a single image.

Orion Sky Lawlor, olawlor@acm.org, 2007/08/23 (Public Domain)
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "drg_image.h"

#include <vector>
#include <map>
#include <deque>

using osl::Vector2d;
using osl::graphics2d::Color;
using osl::graphics2d::RgbaRaster;

void drg_die(const char  *err){ 
	fprintf(stderr,err);
	exit(1);
}

/******************** Projection Info ******************
 The projection code is from the  Alaska Satellite Facility  user tools (src_lib/asf_meta/jpl_proj.c)
*/

#define SQR(A)	((A)*(A))
#define RE (proj->re_major) /*Extract semimajor axis of earth (equatorial)*/
#define RP (proj->re_minor) /*Extract semiminor axis of earth (polar)*/
#define ecc2 (proj->ecc*proj->ecc) /*Extract first eccentricity, squared*/
#define D2R (M_PI/180.0)
proj_parameters::proj_parameters(void)
{
	/* Alaska zone by default */
	utm_zone=6;
	hem='N';
	
	/*Default: use the GEM-06 (Goddard Earth Model 6) Ellipsoid*/
	re_major=6378144.0;
	re_minor=6356754.9;
	
	proj_parameters *proj=this; /* for RE/RP macros above */
	ecc=sqrt(1.0-RP*RP/(RE*RE));
}

int UTM_zone(double lon) {return (int)(((180.0+lon)/6.0+1.0));} 
/*
  Return UTM projection coordinates (*x,*y), in meters,
  given a latitude/longitude, in degrees.
*/
void ll_utm(proj_parameters *proj,double tlat, double tlon, double *x_, double *y_)
{
	double   k0 = 0.9996;       /* Center meridian scale factor */
	double   epsq, lat, /*lon,*/ lon0, a1, a2, a3, rn;
	double   t, b1, b2, rm, rm0;
	double   tanlat, c;
	double   yadj, xadj = 500000.0;
	double   e4, e6, c1, c2, c3, c4;
	double   x,y;
	
	epsq = ecc2/(1.0 - ecc2);
	lon0 = (double)(proj->utm_zone-1)*6.0 + 3.0 - 180.0;   /* zone from proj constants */
	if (proj->hem=='N') yadj = 0.0; else yadj = 1.0E7; /* Ref pt Southern Hemisphere */
	
	lat = tlat * D2R; /*lon = tlon * D2R;*/
	
	rn =  RE / sqrt(1.0 - ecc2*SQR(sin(lat)));
	tanlat = tan(lat);
	t = SQR(tanlat);
	c = epsq * SQR(cos(lat));
	a1 = cos(lat) * ((tlon-lon0)*D2R);
	a2 = (1.0 - t + c) * (a1*a1*a1) / 6.0;
	a3 = (5.0 - 18.0*t + t*t + 72.0*c - 58.0*epsq) * ((a1*a1*a1*a1*a1) / 120.0);
	
	x = k0 * rn * (a1+a2+a3) + xadj;
	
	e4 = ecc2 * ecc2;
	e6 = e4 * ecc2;
	c1 = 1.0 - ecc2/4.0 - 3.0*e4/64.0 - 5.0*e6/256.0;
	c2 = 3.0*ecc2/8.0 + 3.0*e4/32.0 + 45.0*e6/1024.0;
	c3 = 15.0*e4/256.0 + 45.0*e6/1024.0;
	c4 = 35.0*e6/3072.0;
	rm = RE*(c1*lat-c2*sin(2.0*lat)+c3*sin(4.0*lat)- c4*sin(6.0*lat));
	rm0 = 0.0;
	b1 = (SQR(a1))/2.0 + (5.0 - t + 9.0*c + 4.0 *SQR(c)) * (a1*a1*a1*a1) / 24.0;
	b2 = (61.0-58.0*t+SQR(t)+600.0*c+330.0*epsq) * (a1*a1*a1*a1*a1*a1) / 720.0;
	
	y = k0 * (rm - rm0 + rn*tanlat*(b1 + b2)) + yadj;
	
	*x_ = x;
	*y_ = y;
}

/*
  Return a latitude/longitude, in degrees, 
  given UTM projection coordinates (x,y), in meters.
*/
void utm_ll(proj_parameters *proj,double x,double y,double *lat_d,double *lon)
{
	double esq = ecc2;        /* Earth eccentricity squared */
	double k0  = 0.9996;      /* Center meridian scale factor */
	double u1, u2, u3, lat1, esqsin2, lat1d, long0;
	double rm, e1, u, epsq, t1, c1, rn1, r1, rm0;
	double tanlat1, d;
	double xadj = 500000.0, yadj;    /* Northing origin:(5.d5,0.d0) */
	                               /* Southing origin:(5.d5,1.d7) */
	rm0 = 0.0;
	if (proj->hem=='N') 
		yadj=0.0; 
	else 
		yadj=1.0E7; /* Ref pt Southern Hemisphere */
	long0 = (double)(proj->utm_zone)*6.0-183.0;     /* zone from proj_const.inc   */
	
	rm = (y - yadj)/k0 + rm0;
	e1 = (1.0 - sqrt(1.0 - esq))/(1.0 + sqrt(1.0 - esq));
	u = rm/(RE*(1.0-esq/4.0-(3.0*esq*esq/64.0) - (5.0*esq*esq*esq/256.0)));
	u1 = (3.0 * e1 / 2.0 - (27.0 * e1*e1*e1)/32.0) * sin(2.0*u);
	u2 = (21.0 * (e1*e1)/16.0 - (55.0 * e1*e1*e1*e1)/32.0) * sin(4.0*u);
	u3 = (151.0 * (e1*e1*e1) / 96.0) * sin(6.0*u);
	lat1 = u + u1 + u2 + u3;
	lat1d = lat1/D2R;
	
	esqsin2 = 1.0 - esq*SQR(sin(lat1));
	epsq = esq/(1.0 - esq);
	c1 = epsq * SQR(cos(lat1));
	tanlat1 = sin(lat1)/cos(lat1);
	t1 = tanlat1 * tanlat1;
	rn1 = RE/sqrt(esqsin2);
	r1 = RE*(1.0 - esq)/sqrt(esqsin2 * esqsin2 * esqsin2);
	d = (x - xadj)/(rn1 * k0);
	
	*lat_d = lat1d - ((rn1 * tanlat1/r1) * (d*d*0.50
	              - (5.0 + 3.0*t1 - 10.0*c1 + 4.0*c1*c1
	                      - 9.0*epsq) * (d*d*d*d)/24.0
	              + (61.0 + 90.0*t1 + 298.0*c1 + 45.0*t1*t1
	                       - 252.0*epsq - 3.0*c1*c1)
	                  *(d*d*d*d*d*d)/720.0) )/D2R;
	
	*lon = long0 + ((1.0/cos(lat1)) * (d
	      - (1.0 + 2.0*t1 + c1) * (d*d*d)/6.0
	      + (5.0 - 2.0*c1 + 28.0*t1 - 3.0*c1*c1 + 8.0*epsq + 24.0*t1*t1)
	             *(d*d*d*d*d)/120.0) )/D2R;
}




/****************** Digital Raster Graph (DRG) Handling ************/
/* Projection parameters for output image's projection */
proj_parameters output_proj;

drg_image::drg_image(const char *imgName,double lat,double lon) 
	:geo(imgName), imgFile(imgName), img(NULL) 
{
	proj.utm_zone=UTM_zone(lon);
}
drg_image::~drg_image() {
	delete img;
}

void drg_image::read_img(void) {
	if (img!=NULL) drg_die("Calling read_img, but image already exists!\n");
	/* WARNING: NOT threadsafe!  You'd need a lock on each image... */
	static std::deque<drg_image *> allocd;
	const unsigned int max_images_in_ram=30; /* 15-25 MB/image */
	if (allocd.size()>=max_images_in_ram) 
	{
		/* Avoid runaway memory usage: throw out older images. */
		drg_image *victim=allocd.front(); allocd.pop_front();
		if (victim->img==NULL) drg_die("Unallocated image in allocd pool!");
		printf("Cleaning out cached image '%s', of %dx%d pixels\n",
			victim->imgFile.c_str(),
			victim->img->wid,victim->img->ht);
		delete victim->img;
		victim->img=NULL;
	}
	img=new osl::graphics2d::RgbaRaster(imgFile.c_str());
	if (img->wid != geo.width || img->ht != geo.height)
		drg_die("Image file and geo don't match!");
	allocd.push_back(this);
}

// Return the color of our image at this location
Color drg_image::get_color(double lat,double lon)
{
	if (!img) read_img();
	osl::Vector2d utm;
	ll_utm(&proj,lat,lon,&utm.x,&utm.y);
	if (!geo.contains(utm)) return Color::clear;
	osl::Vector2d pix=geo.pixelFmMapd(utm);
	return img->getBilinearPin(pix.x,pix.y);
}

/******************* drg_grid ****************/

/* Return the graph index that would be used by this lat,lon in degrees. */
drg_grid::drg_index drg_grid::make_index(double lat,double lon) 
{
	drg_index i;
	
	/**
	  Latitudes seem off a bit; looks like 180m+-10m to me.
	  This factor (in degrees) corrects for the error.
	*/
	double meters_per_degree_lat=100.0e3;
	double latbias=+180.0/meters_per_degree_lat;
	
	i.y=(int)floor((lat+latbias)*latscale); /* round to south */
	i.x=(int)ceil(lon*lonscale); /* round to east */
	return i;
}

/* Add this image at this lat/lon */
void drg_grid::add_image(drg_image *img,double lat,double lon)
{
	if (get_image(lat,lon)!=0) {
		printf("WARNING! Added multiple images to same spot of same level  of map!\n");
		return;
	}
	
	drg_index idx=make_index(lat,lon);
	printf("Add at index %d,%d\n",idx.x,idx.y);
	map[idx]=img;
	if (map[idx]!=img) drg_die("WTF  drg_grid::add_image map bad!!!");
	
	// Doublecheck lat/lon projection
	const osl::GeoImage &geo=img->get_geo();
	Vector2d cutm=geo.mapFmPixelCenter(osl::Point(geo.width/2,geo.height/2));
	double clat,clon; utm_ll(&img->proj,cutm.x,cutm.y,&clat,&clon);
	drg_index cidx=make_index(clat,clon);
	if (!(cidx==idx)) 
	{
		printf("WARNING! Image '%s', at filename lat/lon %.2f,%.2f has .geo lat/lon of  %.2f,%.2f!\n",
			img->get_name(), lat,lon, clat,clon);
	}
}

/* Return the image that occupies this lat/lon, or NULL if none exists. */
drg_image *drg_grid::get_image(double lat,double lon) 
{
	drg_index idx=make_index(lat,lon);
	map_t::iterator mi=map.find(idx);
	if (mi==map.end()) {
		//printf("Failing get at index %d,%d\n",idx.x,idx.y);
		return 0;
	}
	else {
		return (*mi).second;
	}
}

/***************************** drg_gridset *********************/

drg_gridset::drg_gridset(void)
{
	box.empty();
	grids[0]=new drg_grid(1.0,1.0/3.0); /* "c" files, 1:250K: 3 deg across, 1 deg high */
	grids[1]=new drg_grid(1.0,1.0/2.0); /* <=58deg "c" files, 1:250K: 2 deg across, 1 deg high */
	grids[2]=new drg_grid(4.0,2.0); /* >=61deg "i" files, 1:63K: 30' across, 15' high */
	grids[3]=new drg_grid(4.0,2.0+2.0/3.0); /* <61deg "i" files, 1:63K: 22.5' across, 15' high */
	grids[4]=new drg_grid(4.0,3.0); /* <=58deg "i" files, 1:63K: 20' across, 15' high */
	grids[5]=new drg_grid(8.0,4.0); /* "l" files, 1:25K: 15' across, 7.5' high */
}


void drg_gridset::add(const char *fileName)
{
	/* remove directory part of filename */
	std::string fName=osl::io::File(fileName).getName();
/*	
For example, a filename is supposed to look like:
 i64149c5
 i meaning 1:25K scale, 15' across, 7.5' high
  64 degrees north latitude
    149 degress west longitude
       c latitude sub-chunk
        5 longitude sub-chunk
*/
	double lat=atoi(std::string(fName,1,2).c_str());
	double lon=atoi(std::string(fName,3,3).c_str());
	int to_grid=-1000000000;
	if (fName[0]=='c') {
		if (lat>58) to_grid=0;
		else to_grid=1;
	}
	else if (fName[0]=='i') {
		if (lat>61) to_grid=2;
		else if (lat>58) to_grid=3;
		else to_grid=4;
	}
	else if (fName[0]=='l') to_grid=5;
	else drg_die("Invalid filename first letter: should be  c, i, or l\n");
	
	/* Sub-chunk */
	char latc=fName[6], lonc=fName[7];
	if (latc>='a' && latc<='h') lat+=(latc-'a')*(7.5/60.0);
	else drg_die("Invalid latitude sub-char!\n");
	if (lonc>='1' && lonc<='8') lon+=(lonc-'1')*(7.5/60.0);
	else drg_die("Invalid longitude sub-char!\n");
	
	lat+=0.001; lon+=0.001; /* correct for roundoff */
	lon=-lon; /* longitudes are all negative in Alaska */
	
	printf("Placing image '%s' at lat/lon  %.3f,%.3f (grid %d)\n",
		fName.c_str(),lat,lon,to_grid);
	
	drg_image *img=new drg_image(fileName,lat,lon);
	grids[to_grid]->add_image(img,lat,lon);
	box=box.getUnion(img->get_geo().getBox());
}

// Return the color of our image at this location
Color drg_gridset::get_color(double lat,double lon,double resolution)
{
	int firstgrid=nGrids-1;
	if (resolution>200) firstgrid=1; /* seriously downsize coarse requests */
	else if (resolution>50) firstgrid=4;
	for (int g=firstgrid;g>=0;g--) {
		drg_image *i=grids[g]->get_image(lat,lon);
		if (i) return i->get_color(lat,lon);
	}
	return Color::clear;
}

// Return an image for this map projection.
osl::graphics2d::RgbaRaster drg_gridset::render(const osl::GeoImage &geo)
{
	RgbaRaster out(geo.width,geo.height);
	for (int y=0;y<out.ht;y++)
	for (int x=0;x<out.wid;x++) {
		double lat,lon;
		Vector2d utm=geo.mapFmPixel(osl::Point(x,y));
		utm_ll(&output_proj,utm.x,utm.y,&lat,&lon);
		out.setColor(x,y,get_color(lat,lon,geo.pixelSize.x));
	}
	return out;
}

#if STANDALONE /* rasterizes "output.geo" from input USGS jpgs */
int main(int argc,char *argv[]) {
	drg_gridset gs;
	for (int argi=1;argi<argc;argi++) gs.add(argv[argi]);
	osl::GeoImage geo("output.geo");
	RgbaRaster out=gs.render(geo);
	out.write("output.jpg");
	return 0;
}
#endif
