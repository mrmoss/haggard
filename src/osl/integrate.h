/*
Orion's Standard Library
Orion Sky Lawlor, 8/30/2002
NAME:		osl/integrate.h

DESCRIPTION:	C++ numerical integration library 

This file provides routines for integrating functions,
not over 1D regions, but over 2D polygons.

This is a heavily templated implementation file, for when
you want to integrate some new function over a poly.
*/
#ifndef __OSL_INTEGRATE_H
#define __OSL_INTEGRATE_H

namespace osl { namespace integrate {

/*
Integrate this function over a polygon via a trapezoid method.

The basic idea is to decompose the integration region into
a set of *overlapping* trapezoids, which sum up to our integral.


The specific decomposition used is:
<pre>
  / /                      / R_i  / m_i*x+b_i
  | | f(x,y) dx dy = sum_i |      | f(x,y) dy dx
  / /                      / L_i  / 0
  triangle
</pre>

The templates here have gotten completely out of control; 
I'm trying to optimize for both speed and flexibility.
*/

/*
Call line() for each line segment in this polygon:
	void line(const Vector2d &start,const Vector2d &end);
*/
template <class line_t>
void outlinePoly(const Polygon &p,line_t c) {
	int n=p.size();
	for (int i=0;i<n-1;i++)
		c.line(p[i],p[i+1]);
	c.line(p[n-1],p[0]);
}

/*
Convert each line segment to a call to trapezoid.
The trapezoids' top edge is the line segment, its
bottom edge lies along the x axis, and the left and right 
edges are parallel to the y axis.

void trapezoid(double Sx,double Ex, //Limits of integration in X
		double m, double b); //Linear function for upper limit of y=m*x+b
Note that S may be greater than E, e.g., when moving back around
the polygon.  S and E will never be equal.
*/
template <class trap_t>
class line2trapezoid_t {
	trap_t t;
public:
	line2trapezoid_t(trap_t t_) :t(t_) {}
	void line(const Vector2d &start,const Vector2d &end) {
		if (start.x!=end.x) {
			//Solve y=m*x+b
			double m=(end.y-start.y)/(end.x-start.x);
			double b=start.y-m*start.x;
			t.trapezoid(start.x,end.x,m,b);
		}
	}
};
template <class trap_t> line2trapezoid_t<trap_t> 
line2trapezoid(trap_t t_) {return line2trapezoid_t<trap_t>(t_);}

/*
Convert trapezoid calls into two evaluations of an indefinite
integral-accumulator function.  Note that this is a fairly roundoff-
intensive way to calculate an integral-- a difference is smaller,
and has better roundoff behaviour, than this "sum signed" approach.

void integral(double x, //X to evaluate indefinite integral at
	double my,double by, //Upper limit on y
	double sign); //-1.0 for S, 1.0 for E (for integral(end)-integral(start))
*/
template <class integralAccum_t>
class trapezoid2integral_t {
	integralAccum_t &t;
public:
	trapezoid2integral_t(integralAccum_t &t_) :t(t_) {}
	inline void trapezoid(double Sx,double Ex, //Limits of integration in X
		double m, double b)
	{
		t.integral(Sx,m,b,-1.0);
		t.integral(Ex,m,b,1.0);
	}
};
template <class integralAccum_t> trapezoid2integral_t<integralAccum_t> 
trapezoid2integral(integralAccum_t &t_) {return trapezoid2integral_t<integralAccum_t>(t_);}

//Apply this polygon to this integral
template <class integralAccum_t> void 
integrateAccum(const Polygon &p,integralAccum_t &a) {
	outlinePoly(p,line2trapezoid(trapezoid2integral(a)));
}

/*
Sum up basicIntegral calls to evaluate an entire integral.

A basicIntegral_t evaluates, at x, the indefinite 
integral of some function over a trapezoid bounded by m*x+b.

return_t (basicIntegral_t)(double x,double my,double by);
*/
template <class basicIntegral_t,class return_t>
class integralSum_t {
	basicIntegral_t &t;
	return_t sum;
public:
	integralSum_t(basicIntegral_t &t_) :t(t_), sum(0.0) {}
	void integral(double x, //X to evaluate indefinite integral at
		double my,double by, //Upper limit on y
		double sign) //-1.0 for S, 1.0 for E (for integral(end)-integral(start))
	{
		sum+=sign*t(x,my,by);
	}
	return_t getIntegral(void) const {return sum;}
};

//Evaluate a single definite integral over a polygon:
template <class return_t,class basicIntegral_t> return_t 
basicIntegrate(const Polygon &p,basicIntegral_t bi) {
	integralSum_t<basicIntegral_t,return_t> is(bi);
	integrateAccum(p,is);
	return is.getIntegral();
}
template <class basicIntegral_t> double
integrateDouble(const Polygon &p,basicIntegral_t bi) {
	return basicIntegrate<double,basicIntegral_t>(p,bi);
}

//An integralAccum_t, for calculating the 2d center-of-mass
class centerOfMass_t {
	double area_sum;
	Vector2d com_sum;//Center of mass times area
public:
	centerOfMass_t(void) :area_sum(0),com_sum(0,0) {}
	void integral(double x, //X to evaluate indefinite integral at
		double m,double b, //Upper limit on y
		double sign)
	{
		const double oneHalf=1.0/2.0;
		const double oneThird=1.0/3.0;
		const double oneSixth=1.0/6.0;
		
//Integral2[1] == Integral[x, (m x+b) ] == m x^2/2+b x
		area_sum+=(oneHalf*(m*x)+b)*(x*sign);
		
// x component: Integral2[x] == Integral[x, x(m x+b) ] == m x^3/3+b x^2/2
// y component: Integral2[y] == Integral[x, Integral[y,y]] == 
//     Integral[x, (m x+b)^2/2 ] == Integral[x, (m^2 x^2 + 2 b m x + b^2)/2 ] == 
//     m^2 x^3/6 + b m x^2/2 + b^2 x/2
		com_sum.x+=(oneThird*(m*x)+oneHalf*b)*x*(x*sign);
		com_sum.y+=((oneSixth*m*(m*x)+oneHalf*b*m)*x+oneHalf*b*b)*(x*sign);
	}
	double getArea(void) const {return area_sum;}
	double getIx(void) const {return com_sum.x;}
	double getIy(void) const {return com_sum.y;}
	Vector2d getCOM(void) const {return com_sum*(1.0/area_sum);}
};

//Another integralAccum_t, for computing 2nd-order moments of inertia
class momentsOfInertia_t {
	double Ixx; //Integral2[x^2]
	double Ixy; //Integral2[x y]
	double Iyy; //Integral2[y^2]
public:
	momentsOfInertia_t(void) :Ixx(0),Ixy(0),Iyy(0) {}
	void integral(double x, //X to evaluate indefinite integral at
		double m,double b, //Upper limit on y
		double sign)
	{
		const double oneThird=1.0/3.0;
		const double oneFourth=1.0/4.0;
		const double oneEigth=1.0/8.0;
//Integral2[x^2] == Integral[x,x^2 (m x+b)] == Integral[x,m x^3 + b x^2] ==
		Ixx+=(oneFourth*(m*x)+oneThird*b)*x*(x*(x*sign));
		
//Integral2[x y] == Integral[x,x (m x+b)^2/2] == 
//     Integral[x,(m^2 x^3+2 m b x^2+b^2 x)/2] == m^2 x^4/8+m b x^3/3+b^2 x^2/4
		Ixy+=((oneEigth*m*(m*x)+oneThird*m*b)*x+oneFourth*b*b)*(x*(x*sign));
		
//Integral2[y^2] == Integral[x,(m x+b)^3/3] == (m x+b)^4/(12 m)
// (division is much simpler and possibly faster than horrible polynomial)
		if (m==0) Iyy+=oneThird*b*b*b*(x*sign);
		else /*m!=0, so division possible*/ {
			double sum=m*x+b;
			sum=sum*sum; sum=sum*sum;
			Iyy+=sum/(12*m)*sign;
		}
	}
	double getIxx(void) const {return Ixx;}
	double getIxy(void) const {return Ixy;}
	double getIyy(void) const {return Iyy;}
};



}; }; //end namespace osl::integrate
#endif //__OSL_INTEGRATE_T_H

