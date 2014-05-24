/**
 Convert lat/lon to map-projected UTM x,y coordinates
 
 Orion Sky Lawlor, olawlor@acm.org, 2007-07-03 
 The projection code is from the  Alaska Satellite Facility  user tools (src_lib/asf_meta/jpl_proj.c)
*/
#ifndef __OSL_UTM_H
#define __OSL_UTM_H
#include <math.h>

#define SQR(A)	((A)*(A))
#define RE (proj->re_major) /*Extract semimajor axis of earth (equatorial)*/
#define RP (proj->re_minor) /*Extract semiminor axis of earth (polar)*/
#define ecc2 (proj->ecc*proj->ecc) /*Extract first eccentricity, squared*/
#define D2R (M_PI/180.0) /* multiply this by degrees to get radians */

struct utm_parameters {
	/* UTM zone number (for example, zone 6 for interior Alaska) */
	int utm_zone; 
	/* Hemisphere (default 'N' for northern hemisphere) */
	char hem;
	/* Eccentricity of earth */
	double ecc;
	/* Equatorial, polar earth radius */
	double re_major, re_minor;
	
	utm_parameters(void) {
		utm_zone=-999;
		hem='N';
		/*Default: use the WGS-84 Ellipsoid*/
		re_major=6378137.0;
		re_minor=6356752.314;
		ecc=sqrt(1.0-SQR(re_minor)/SQR(re_major));
	}
};

/* Return UTM zone given longitude (in degrees) */
inline int UTM_zone(double lon) {return (int)(((180.0+lon)/6.0+1.0));} 

/*
  Return a latitude/longitude, in degrees, 
  given UTM projection coordinates (x,y), in meters.
*/
inline void utm_ll(const utm_parameters *proj,double x,double y,double *lat_d,double *lon)
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

/*
  Return UTM projection coordinates (*p1,*p2), in meters,
  given a latitude/longitude, in degrees.
*/
inline void ll_utm(const utm_parameters *proj,double tlat, double tlon, double *p1, double *p2)
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
	
	*p1 = x;
	*p2 = y;
}

#endif
