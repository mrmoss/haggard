/*  
Spherical trigonometry routines.

Orion Sky Lawlor, olawlor@acm.org, 2004/7/6
*/
#include "osl/spheretrig.h"

/**
  Return the area (in steradians) of the portion
  of the disk A also covered by the disk B, where 
  the angle D is the distance between the disk centers.
*/
double spheretrig::intersect(double cosA,double cosB,double cosD)
{
/*
Notation:
<pre>
                 Intersection of boundaries  
                          .    
                        / d \                         
     (disk radius) A  /        \    B  (disk radius)  
                    /             \                   
                  /                  \                
      Center    / b                   a \   Center    
     of disk A .--------------------------. of disk B 
                            D                         
              distance between disk centers           
</pre>

Now the law of cosines (on a sphere) gives us:
	cos B = cos A cos D + sin A sin D cos b
so
	cos b = (cos B - cos A cos D) / (sin A sin D)
*/
	double sinA=sinFmCos(cosA);
	double sinB=sinFmCos(cosB);
	double sinD=sinFmCos(cosD);
	double cosa=(cosA - cosB*cosD)/(sinB*sinD);
	if (cosa>1.0 || cosa<-1.0) 
	{ // boundaries don't ever touch
		// double ra=acos(cosA), rb=acos(cosB), rd=acos(cosD);
		double cosAB=cosA*cosB-sinA*sinB;
		double sinAB=sinA*cosB+cosA*sinB;
		if ((sinAB>0) && (cosAB>=cosD)) /* ra+rb<=rd, disks are totally separated */
			return 0.0;
		else { /* disks overlap at least partially */
			double cosAD=cosA*cosD-sinA*sinD;
			double sinAD=sinA*cosD+cosA*sinD;
			if ((sinAD>0) && (cosAD>=cosB))  /* ra+rd<=rb */
				return disk(cosA); /* A totally covered by B */
			int nGT=(cosA<0)+(cosB<0)+(cosD<0); // count of angles over pi/2
			if (nGT>=2) {
				double sinABD=sinAB*cosD+cosAB*sinD;
				if (sinABD>0) // i.e., if (ra+rb+rd > 2*M_PI)
				{/* wraparound-- missing only B's hole */
					return disk(cosA)-(4*M_PI-disk(cosB)); 
				}
			}
			/* B totally covered by A */
			return disk(cosB);
		}
	}
	double cosb=(cosB - cosA*cosD)/(sinA*sinD);
	double cosd=(cosD - cosA*cosB)/(sinA*sinB);
	
	double a=acos(cosa), b=acos(cosb), d=acos(cosd);
	double wA=wedge(cosA,2*b), wB=wedge(cosB,2*a), tT=2*triangle(a,b,d);
	double area=wA+wB-tT;
	return area;
}
