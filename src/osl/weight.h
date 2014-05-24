/*
Return a weight between 0 and 1 for any input.  
Function used is:
	0    x<a || x>d
	1    b<=x && x<=c
	smoothly in between
*/
#ifndef __OSL_SMOOTHWEIGHT_H
#define __OSL_SMOOTHWEIGHT_H

namespace osl {

class SmoothWeight {
	//Blend output smoothly between 0 and 1 for input between 0 and 1
	double blend(double r) const {
		return 0.5-0.5*cos(r*M_PI);
	} 
	double a,b,c,d;
	double lSlope,rSlope;
public:
	SmoothWeight(double a_,double b_,double c_,double d_)
		:a(a_), b(b_), c(c_), d(d_) 
	{
		lSlope=1.0/(b-a);
		rSlope=1.0/(d-c);
	}
	double weight(double x) const {
		if (x<=a || x>=d) return 0.0;
		if (b<=x && x<=c) return 1.0;
		if (x<b) return blend((x-a)*lSlope);
		else return blend((d-x)*rSlope);
	}
	double operator()(double x) const {return weight(x);}
};

}; /*end namespace osl*/

#endif /*def(thisHeader)*/

