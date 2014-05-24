/**
Solve a cubic polynomial.

Orion Sky Lawlor, olawlor@acm.org, 2004/10/29
*/
#ifndef __OSL_CUBIC_H
#define __OSL_CUBIC_H
#include <complex>

namespace osl {

/**
Solve linear equation
	ax + b = 0
Returns number of roots: 1 or 0.
Returns 0 if there are infinitely many roots.
*/
inline int linear(double  a,         /* coefficient of x   */
	double  b,         /* constant term      */
	
	double *x)         /* pointer to output solution      */
{
	if (a==0) return 0; /* zero (or infinitely many) solutions */
	x[0]=-b/a;
	return 1;
}

/**
Solve quadratic equation.  Returns number of real roots: 0 to 2.
*/
inline int quadratic(double  a,         /* coefficient of x^2 */
	double  b,         /* coefficient of x   */
	double  c,         /* constant term      */
	
	double *x)         /* array of output solutions  (up to 2) */
{
	if (a==0) return linear(b,c, x);
	double a_inv=1.0/a;
	double b_2a=-0.5*b*a_inv;
	double det=b_2a*b_2a - c*a_inv;
	if (det<0) return 0; /* no real roots */
	det=sqrt(det);
	x[0]=b_2a-det;
	x[1]=b_2a+det;
	return 2;
}


/**
Compute the real roots of this cubic equation.

Modified from the original at Glenn Rhoads' page:
	http://remus.rutgers.edu/~rhoads/Code/
*/
inline int cubic(double  a,         /* coefficient of x^3 */
                double  b,         /* coefficient of x^2 */
                double  c,         /* coefficient of x   */
                double  d,         /* constant term      */
		
                double *x)         /* array of output solutions      */
{
	if (a==0) return quadratic(b,c,d, x);
	double a_inv=1.0/a;
	double    a1 = b*a_inv, a2 = c*a_inv, a3 = d*a_inv; /* normalize leading term */
	double    Q = (a1*a1 - 3.0*a2)*(1.0/9.0);
	double    sqrt_Q = sqrt(Q);
	double R = (2.0*a1*a1*a1 - 9.0*a1*a2 + 27.0*a3)*(1.0/54.0);
	double    R2_Q3 = R*R - Q*Q*Q;
	const double oneThird=1.0/3.0;
	
	if (R2_Q3 <= 0) {
		double theta = acos(R/(sqrt_Q*Q));
		x[0] = -2.0 * sqrt_Q * cos((theta       )*oneThird) - a1*oneThird;
		x[1] = -2.0 * sqrt_Q * cos((theta+2.0*PI)*oneThird) - a1*oneThird;
		x[2] = -2.0 * sqrt_Q * cos((theta+4.0*PI)*oneThird) - a1*oneThird;
		return 3;
	}
	else {
		double t=pow( sqrt(R2_Q3) + fabs(R), oneThird);
		x[0] = (t+Q/t)*((R < 0.0) ? 1 : -1) - a1*oneThird;
		return 1;
	}
}

/************* Complex versions of above **********/

typedef std::complex<double> Complex;

/**
Solve linear equation
	ax + b = 0
Returns number of roots: 1 or 0.
Returns 0 if there are infinitely many roots.
*/
inline int linear(double  a,         /* coefficient of x   */
	double  b,         /* constant term      */
	
	Complex *x)         /* pointer to output solution      */
{
	if (a==0) return 0; /* zero (or infinitely many) solutions */
	x[0]=-b/a;
	return 1;
}

/**
Solve quadratic equation.  Returns number of roots: 0 to 2.
Roots are returned in ascending order.
*/
inline int quadratic(double  a,         /* coefficient of x^2 */
	double  b,         /* coefficient of x   */
	double  c,         /* constant term      */
	
	Complex *x)         /* array of output solutions  (up to 2) */
{
	if (a==0) return linear(b,c, x);
	double a_inv=1.0/a;
	double b_2a=-0.5*b*a_inv;
	double det=b_2a*b_2a - c*a_inv;
	if (det>=0) {
		det=sqrt(det);
		x[0]=b_2a-det;
		x[1]=b_2a+det;
	} else /* det < 0, both roots imaginary */ {
		det=sqrt(-det);
		x[0]=Complex(b_2a,-det);
		x[1]=Complex(b_2a,+det);
	}
	return 2;
}


/**
Compute the real roots of this cubic equation.

See
	http://mathworld.wolfram.com/CubicFormula.html
for a complete derivation.
*/
inline int cubic(double  a,         /* coefficient of x^3 */
                double  b,         /* coefficient of x^2 */
                double  c,         /* coefficient of x   */
                double  d,         /* constant term      */
		
                Complex *x)         /* array of output solutions      */
{
	if (a==0) return quadratic(b,c,d, x);
	double a_inv=1.0/a;
	double    a2 = b*a_inv, a1 = c*a_inv, a0 = d*a_inv; /* normalize leading term */
	const double halfRootThree=0.5*sqrt(3.0);
	const double oneThird=1.0/3.0;
	
	double lambda = oneThird*a2;
	double    Q = (3.0*a1 - a2*a2)*(1.0/9.0);
	double R = (9.0*a2*a1 - 27*a0 - 2.0*a2*(a2*a2))*(1.0/54.0);
	Complex S,T;
	if (Q==0) 
	{ /* work around a bug in gcc 3.2.3 pow(complex(0),1/3) */
		S=pow(2*R,oneThird);
		T=0;
	} else {
		double    D = Q*Q*Q + R*R;
		/* could simplify this if D is nonnegative... */
		Complex sqrt_D=sqrt(Complex(D));
		S=pow(R+sqrt_D,oneThird);
		T=pow(R-sqrt_D,oneThird);
	}
	
	x[0]=-lambda+     (S+T);
	x[1]=-lambda -0.5*(S+T) +Complex(0,halfRootThree)*(S-T);
	x[2]=-lambda -0.5*(S+T) -Complex(0,halfRootThree)*(S-T);
	
	return 3;
}

};

#endif
