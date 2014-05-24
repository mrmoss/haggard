/* 
2D fluid dynamics using the explicit, but stable and 
pressure-free formulation of Jos Stam.

Orion Sky Lawlor, olawlor@acm.org, 2005/12/15 (Public Domain)
*/
#include "osl/fluid.h"

using namespace osl;
using namespace osl::graphics2d;

namespace osl {
/* Add the divergence-eliminating velocity correction for "vel" to "corr" */
void divergenceCorrection(const VelocityRaster &vel,VelocityRaster &corr) {
	int x,y;
#if 0 /* Wider kernel */
	int maskX=vel.wid-1, maskY=vel.ht-1;
	/* Apply correction wherever there's divergence */
	for (y=1;y<vel.ht-1;y++) /* Interior */
	for (x=1;x<vel.wid-1;x++) {
		const Vector2d &v=vel.at(x,y);
		double vl=vel.at(maskX&(x-1),y).x, vr=vel.at(maskX&(x+1),y).x;
		double vt=vel.at(x,maskY&(y-1)).y, vb=vel.at(x,maskY&(y+1)).y;
		double div=(1/4.0)*((vr-vl)+(vb-vt));
		// FIXME: in addition to just destroying divergence,
		//  add that energy back into the velocity (somehow).
		corr.at(x-1,y).x+=div;
		corr.at(x+1,y).x-=div;
		corr.at(x,y-1).y+=div;
		corr.at(x,y+1).y-=div;
		// corr.at(x,y)-=0.5*div*vel.at(x,y); // .dir(); // ?
	}
#elif 1 /* Tiny kernel, wraparound-boundaries version */
	int maskX=vel.wid-1, maskY=vel.ht-1;
	/* Apply correction wherever there's divergence */
	for (y=0;y<vel.ht;y++) /* Interior */
	for (x=0;x<vel.wid;x++) {
		const Vector2d &v=vel.at(x,y);
		double vl=vel.at(maskX&(x-1),y).x, vr=v.x;
		double vt=vel.at(x,maskY&(y-1)).y, vb=v.y;
		double div=(1/4.0)*((vr-vl)+(vb-vt));
		// FIXME: in addition to just destroying divergence,
		//  add that energy back into the velocity (somehow).
		corr.at(maskX&(x-1),y).x+=div;
		corr.at(x,maskY&(y-1)).y+=div;
		corr.at(x,y).x-=div;
		corr.at(x,y).y-=div;
		// corr.at(x,y)-=0.5*div*vel.at(x,y); // .dir(); // ?
	}
#else /* fixed-boundaries version */
	/* Apply correction wherever there's divergence */
	for (y=1;y<vel.ht-1;y++) /* Interior */
	for (x=1;x<vel.wid-1;x++) {
		const Vector2d &v=vel.at(x,y);
		double vl=vel.at(x-1,y).x, vr=v.x;
		double vt=vel.at(x,y-1).y, vb=v.y;
		double div=(1/4.0)*((vr-vl)+(vb-vt));
		// FIXME: in addition to just destroying divergence,
		//  add that energy back into the velocity (somehow).
		corr.at(x-1,y).x+=div;
		corr.at(x,y-1).y+=div;
		corr.at(x,y).x-=div;
		corr.at(x,y).y-=div;
		// corr.at(x,y)-=0.5*div*vel.at(x,y); // .dir(); // ?
	}
#endif
}

/// Multigrid divergence correction class.  Represents one
///  level of the grid.
class MultigridDivergence {
public:
	MultigridDivergence(int w,int h,int maxLevel_,int level_=0) 
		:myLevel(level_), maxLevel(maxLevel_)
	{
		vel.reallocate(w,h);
		corr.reallocate(w,h);
		if (myLevel+1<maxLevel) {
			coarser=new MultigridDivergence(w/2,h/2,maxLevel,myLevel+1);
		} else coarser=0;
	}
	/* Called from outside: add the correction for this velocity image */
	void correct(VelocityRaster &v) {
		int x,y;
		for (y=0;y<v.ht;y++)
		for (x=0;x<v.wid;x++) vel.at(x,y)=v.at(x,y);
		corrFmVel(corr);
		for (y=0;y<v.ht;y++)
		for (x=0;x<v.wid;x++) v.at(x,y)+=corr.at(x,y);
	}
	
private:
	/* Called only from inside: fill "corr" with the correction for our 
	   velocity field (member "vel"). */
	void corrFmVel(VelocityRaster &corr) 
	{
		int x,y;
		if (coarser) { /* Start corr with the correction from the higher level */
		/* Copy vel into coarser->vel  [ fine -> coarse ] */
			VelocityRaster &cvel=coarser->vel;
			for (y=0;y<cvel.ht;y++)
			for (x=0;x<cvel.wid;x++) {
				cvel.at(x,y)=0.25*(
					vel.at(2*x  ,2*y  )+
					vel.at(2*x+1,2*y  )+
					vel.at(2*x  ,2*y+1)+
					vel.at(2*x+1,2*y+1)
				);
			}
		/* Compute coarser correction */
			coarser->corrFmVel(coarser->corr);
		/* Copy coarse correction back to fine grid  [ coarse -> fine ] 
		  FIXME: use smooth interpolation.
		*/
			for (y=0;y<cvel.ht;y++)
			for (x=0;x<cvel.wid;x++) {
				Vector2d c=coarser->corr.at(x,y);
				corr.at(2*x  ,2*y  )=c;
				corr.at(2*x+1,2*y  )=c;
				corr.at(2*x  ,2*y+1)=c;
				corr.at(2*x+1,2*y+1)=c;
			}
		/* Apply coarse correction factors to our velocities */
			for (y=0;y<corr.ht;y++)
			for (x=0;x<corr.wid;x++) {
				vel.at(x,y)+=corr.at(x,y);
			}
		} 
		else /* no coarser grid exists-- start with corr==0 */ {
			corr.set(Vector2d(0,0));
		}
		
		/* Add in correction from our grid level */
		divergenceCorrection(vel,corr);
	}
	
	/* Level is the bit-right-shift to apply to the image size */
	int myLevel,maxLevel;
	VelocityRaster vel,corr; /* Velocity field, and correction field */
	MultigridDivergence *coarser; /* Coarser grid level (if one exists!) */
	/* Return grid level n times coarser than us */
	MultigridDivergence *getLevel(int l) {
		if (l==0) return this;
		else return coarser->getLevel(l-1);
	}
};




/* Create a new steady simulation. */
FluidSimulation::FluidSimulation(int w,int h) 
	:tracer1(w,h), tracer2(w,h), tracerDest(&tracer2),
	 vel1(w,h), vel2(w,h), velDest(&vel2),
	 md(new MultigridDivergence(w,h,8)), /* hardcoded: # of multigrid levels */
	 tracerRast(tracer1), velocityRast(vel1),
	 tracer(&tracer1), velocity(&vel1)
{
	
	tracer1.clear(Color::black);
	tracer2.clear(Color::black);
	vel1.set(Vector2d(0,0));
	vel2.set(Vector2d(0,0));
	srcDestChanged();
}
FluidSimulation::~FluidSimulation()
{
	delete md;
}

/** Take one step of this length. */
void FluidSimulation::step(double dt,int flags) {
	velScale=dt;
	if (!(flags&flag_skip_tracer)) {
	/* Advect tracer field */
		advect(*tracer,*tracerDest);
		std::swap(tracer,tracerDest);
	}
	if (!(flags&flag_skip_velocity)) {
	/* Advect velocity field */
		advect(*velocity,*velDest);
		std::swap(velocity,velDest);
	}
	
	if (!(flags&flag_skip_mass)) {
	/* Mass conservation step, Multigrid Correction */
		md->correct(*velocity);
	}
	srcDestChanged();
}

void FluidSimulation::srcDestChanged(void)
{
	tracerRast.setBuffer(*tracer);
	velocityRast.setBuffer(*velocity);
}

/* Advect this source field into dest.
  WARNING: src==dest will give very weird results.
*/
void FluidSimulation::advect(const RgbaRaster &src,RgbaRaster &dest) {
	for (int y=0;y<src.ht;y++)
	for (int x=0;x<src.wid;x++)
	{
		Vector2d del=getVelocity(x,y);
		//dest.at(x,y)=src.getBilinearWrap(0.5+x+del.x,0.5+y+del.y);
		RgbaPixel interp=src.fix8BilinearWrap(
			(int)(256*(0.5+x+del.x)),
			(int)(256*(0.5+y+del.y))
		);
		// interp&=0xfefefefe; /* Turn off low bit of each channel (decays toward zero) */
		dest.at(x,y)=interp;
	}
}

void FluidSimulation::advect(const VelocityRaster &src,VelocityRaster &dest) {
	Vector2d bias=Vector2d(0,0);
#if (0) 
		if ((stepNo%2)==0) bias=Vector2d(0.3,0.3);
		if ((stepNo%2)==1) bias=Vector2d(-0.3,-0.3); /* Adds a little artificial dissipation */
#endif
	
	for (int y=0;y<src.ht;y++)
	for (int x=0;x<src.wid;x++)
	{
		Vector2d del=getVelocity(x,y)+bias;
		Vector2d sv=src.getBilinearWrap2d(x+del.x,y+del.y); /* Source color */
		// RgbaPixel p=tracer->at(x,y);
		// c.g+=(0.01/256.0)*(p.r()-p.b()); /* Red is boyant, blue sinks */
		dest.at(x,y)=sv; // .setColor(x,y,c);
	}
}

};
