/* 
2D fluid dynamics using the explicit, but stable and 
pressure-free formulation of Jos Stam.

Orion Sky Lawlor, olawlor@acm.org, 2005/12/15 (Public Domain)
*/
#ifndef __OSL_FLUID_2D_PRESSUREFREE_H
#define __OSL_FLUID_2D_PRESSUREFREE_H

#include "osl/rasterizer.h"
#include "osl/pixel_arithmetic.h"
#include "osl/vector2d.h"

namespace osl { namespace graphics2d {

/// Linear 1D interpolation
inline Vector2d lerp(double f,const Vector2d &a,const Vector2d &b) {
	return a+f*(b-a);
}

/**
  An image consisting purely of velocity vectors.
  Use "at" to access the vectors directly (no scaling).
  For color output, x and y are stored in red and green,
  biased up by 0.5, and scaled according to "setScale".
*/
class VelocityRaster : public osl::graphics2d::FlatRasterT<Vector2d> {
public:
	inline Color getColor(int x,int y) const {
		const Vector2d &v=at(x,y);
		Color c(colorFmVec*v.x+0.5,colorFmVec*v.y+0.5,0);
		c.clip();
		return c;
	}
	inline void setColor(int x,int y,const Color &c) {
		at(x,y)=Vector2d(vecFmColor*(c.r-0.5),vecFmColor*(c.g-0.5));
	}
	
	Vector2d getBilinearWrap2d(double x,double y) const {
		int ix=(int)floor(x), iy=(int)floor(y);
		Vector2d tl=at((ix+0)&(wid-1),(iy+0)&(ht-1));
		Vector2d tr=at((ix+1)&(wid-1),(iy+0)&(ht-1));
		Vector2d bl=at((ix+0)&(wid-1),(iy+1)&(ht-1));
		Vector2d br=at((ix+1)&(wid-1),(iy+1)&(ht-1));
		return lerp(y-iy,
			lerp(x-ix, tl,tr),
			lerp(x-ix, bl,br)
		);
	}

	/* A k-length vector displays as pure white or black */
	void setScale(double k) {
		vecFmColor=k/0.5;  colorFmVec=1.0/vecFmColor;
	}
	
	void set(const Vector2d &v) {
		for (int y=0;y<ht;y++)
		for (int x=0;x<wid;x++)
			at(x,y)=v;
	}
	
	VelocityRaster() {setScale(1.0);}
	VelocityRaster(int w,int h) :FlatRasterT<Vector2d>(w,h) {setScale(1.0);}
private:
	/* Conversion between colors and vectors */
	double vecFmColor, colorFmVec;
};

}; /* end namespace graphics2d */

class MultigridDivergence;

/**
 Fluid flow simulation class.
*/
class FluidSimulation {
	/* Advected tracer field */
	osl::graphics2d::RgbaRaster tracer1, tracer2;
	osl::graphics2d::RgbaRaster *tracerDest;
	
	/* Velocity field */
	osl::graphics2d::VelocityRaster vel1, vel2;
	osl::graphics2d::VelocityRaster *velDest;
	
	/* Divergence correction/mass conservation */
	MultigridDivergence *md;
	
/* Utility routines used within a step */
	double velScale;
	Vector2d getVelocity(int x,int y) {
		return velScale*velocity->at(x,y);
	}
	void srcDestChanged(void);
public:
	/** Rasterizers for current tracer and velocity images.
	  (For applying boundary conditions, forcing, etc.)
	*/
	osl::graphics2d::Rasterizer tracerRast;
	osl::graphics2d::Rasterizer velocityRast;
	/** Pointers to current tracer and velocity images 
	  (For applying boundary conditions, forcing, etc.)
	*/
	osl::graphics2d::RgbaRaster *tracer; 
	osl::graphics2d::VelocityRaster *velocity;
	
	/* Create a new steady (all zeros) simulation. */
	FluidSimulation(int w,int h);
	virtual ~FluidSimulation();
	
	typedef enum {
		flag_skip_tracer=1<<0, ///< Skip tracer advection step
		flag_skip_velocity=1<<1, ///< Skip velocity advection step
		flag_skip_mass=1<<2 ///< Skip mass conservation (velocity divergence) step
	} flag_step_t;
	
	/** Take one step of this length. 
	  dt == Pixels of motion per unit velocity (per step)
	*/
	void step(double dt,int flags=0);
	
	/** Tracer advection */
	virtual void advect(const osl::graphics2d::RgbaRaster &src,osl::graphics2d::RgbaRaster &dest);
	/** Velocity advection (without mass conservation step) */
	virtual void advect(const osl::graphics2d::VelocityRaster &src,osl::graphics2d::VelocityRaster &dest);
};

}; /* end namespace osl */

#endif
