/*  
Matrix blending:

Orion Sky Lawlor, olawlor@acm.org, 2003/5/24
*/
#include <stdio.h>
#include <stdlib.h>
#include "osl/osl.h"
#include "osl/fn1d.h"
#include "osl/matrixpower.h"
#include <math.h>
#include <complex>

/** I'll need complex numbers to represent matrix eigenvectors/
eigenvalues. */
typedef std::complex<double> Complex;
typedef osl::MatrixT<Complex,3,3> Matrix3x3;
double complex2double(const Complex &c) { return c.real(); }

namespace std { /* silly: I just want "fabs(cpx)" to work inside matrixT template... */
inline double fabs(const Complex &c) {
	return abs(c);
}
};

namespace osl {
  class MatrixPower2dImpl {
	/**
	D[0] and D[1] are the paired complex eigenvalues.
	D[2] is the single pure real eigenvalue.
	*/
	Complex D[3];
	
	/// P is a complex matrix with the eigenvectors in its columns.
	Matrix3x3 P;
	/// P_i is the inverse of P;
	Matrix3x3 P_i;
	
	bool init(const Matrix2d &src);
  public:
	/// Factorize this matrix.
	MatrixPower2dImpl(const Matrix2d &src);
	
	/// Raise this matrix to this power, returning the new matrix.
	void power(double exp,Matrix2d &ret) const;
	
	/// Evaluate the location of this point under this power of this matrix.
	Vector2d power(double exp,const Vector2d &src) const;
	
	/**
	 * Find the possible axis-aligned extrema of the curve
	 * swept out by the map of this point as exp varies from 0 to 1.
	 */
	void extrema(const Vector2d &v,VirtualConsumer<Vector2d> &dest) const;
  };
};
using namespace osl;

MatrixPower2d::MatrixPower2d(const Matrix2d &src) {
	impl=new MatrixPower2dImpl(src);
}
MatrixPower2d::~MatrixPower2d() { 
	delete impl;
}
void MatrixPower2d::power(double exp,Matrix2d &ret) const {
	impl->power(exp,ret);
}
Vector2d MatrixPower2d::power(double exp,const Vector2d &src) const {
	return impl->power(exp,src);
}
void MatrixPower2d::extrema(const Vector2d &v,VirtualConsumer<Vector2d> &dest) const {
	impl->extrema(v,dest);
}

MatrixPower2dImpl::MatrixPower2dImpl(const Matrix2d &src) {
	Matrix2d m(src);
	while (!init(m))
	{ //That try didn't work-- perturb the matrix and try again:
		double mag=0;
		for (int r=0;r<2;r++) for (int c=0;c<2;c++) {
			double val=fabs(m(r,c));
			if (mag<val) mag=val;
		}
		m(0,0)+=mag*1.0e-5;
		m(0,1)+=mag*0.5e-5;
		m(1,0)+=mag*0.25e-5;
		m(1,1)+=mag*0.125e-5;
		// printf("Perturbed matrix\n");
	}
}

/*
   Solve the 2x2 system:
	[ a b ] [ dest[0] ] = [ c ]
	[ d e ] [ dest[1] ] = [ f ]
 */
bool solve2x2(
	float a, float b, float c,
	float d, float e, float f,
	float *dest)
{
	MatrixT<float,2,3> m;
	m(0,0)=a; m(0,1)=b; m(0,2)=c;
	m(1,0)=d; m(1,1)=e; m(1,2)=f;
	if (!m.solve()) return false;
	dest[0]=m(0,2);
	dest[1]=m(1,2);
	return true;
}


/* Find the eigenvalues and eigenvectors for this matrix.
   The eigenvalues D are solutions to the equation:
       [ a b c ] [ x ]     [ x ]
       [ d e f ] [ y ] = D [ y ]
       [ 0 0 1 ] [ 1 ]     [ 1 ]
   or
       [ a-D b  c  ] [ x ]   [ 0 ]
       [ d  e-D f  ] [ y ] = [ 0 ]
       [ 0   0 1-D ] [ 1 ] = [ 0 ]
   This means the determinant must be zero, so D satisfies
   the cubic
       0 = ((a-D)*(e-D)-db)*(1-D)
   
   The first two solutions are the paired complex roots 
   of (a-D)*(e-D)-db = 0.
   
   The final solution is D==1, which is the translation portion.
   
   WARNING: assumes this is a homogenous matrix-- the bottom
    row must read "0 0 1".
 */
bool MatrixPower2dImpl::init(const Matrix2d &src) {
	double a=src(0,0), b=src(0,1), c=src(0,2);
	double d=src(1,0), e=src(1,1), f=src(1,2);
	Complex eigenvector[3];
	
	double b__2=(a+e)*0.5; 
	double detSqr=b__2*b__2-(a*e-d*b);
	Complex det;
	if (detSqr<0)
		det=Complex(0,sqrt(-detSqr));
	else
		det=Complex(sqrt(detSqr),0);
	
	// Paired roots:
	D[0]=b__2+det;
	D[1]=b__2-det;
	
	for (int c=0;c<2;c++) {
		if (a!=0 && b!=0) { // Eigenvector uniquely determined by first row:
			eigenvector[0]=-b;
			eigenvector[1]=a-D[c];
		} else if (d!=0 && e!=0) { // Uniquely determined by second row:
			eigenvector[0]=e-D[c];
			eigenvector[1]=-d;
		} else { // Eigenvectors not uniquely determined-- pick one
			eigenvector[0]=!c;
			eigenvector[1]=c;
		}
		
		eigenvector[2]=0; //Direction vector
		P.setColumn(c,eigenvector);
	}
	
	D[2]=1; //Last eigenvector: translation portion (always real)
	float soln[2];
	if (!solve2x2(a-1,b,-c, d,e-1,-f, soln)) {
		// Eigenvector not uniquely determined-- 
		//  just take zero.
		soln[0]=0.0;
		soln[1]=0.0;
	}
	eigenvector[0]=soln[0];
	eigenvector[1]=soln[1];
	eigenvector[2]=Complex(1,0);
	P.setColumn(2,eigenvector);
	
	while (!P.invert(P_i)) {
		//Eigenvectors not invertible-- this should never happen!
		return false;
	}
	
	return true;
}

/**
 * Raise this 2d translation matrix to a (fractional) power "exp".
 */
void MatrixPower2dImpl::power(double exp,Matrix2d &ret) const
{
	Matrix3x3 powD(1.0); //Diagonal matrix
	for (int i=0;i<3;i++) powD(i,i)=pow(D[i],exp);
	
	Matrix3x3 PD;
	P.product(powD,PD);
	Matrix3x3 PDP_i;
	PD.product(P_i,PDP_i);
	copy(complex2double,PDP_i,ret);
}

inline void v2d_copy(const Vector2d &src, Complex *dest) {
	dest[0]=src.x; dest[1]=src.y; dest[2]=1.0;
}

/// Evaluate the location of this point under this power of this matrix.
Vector2d MatrixPower2dImpl::power(double exp,const Vector2d &src) const
{
	Complex V1[3], V2[3], V3[3];
	v2d_copy(src,V1);
	P_i.apply(V1,V2);
	for (int i=0;i<3;i++) V2[i]=V2[i]*pow(D[i],exp);
	P.apply(V2,V3);
	return Vector2d(real(V3[0]),real(V3[1]));
}

/**
 * Evaluates the 1D derivative of the exponential spiral:
 *   x(n) = C1*L1^n + C2*L2^n + C3*L3^n
 * C1-3 and L1-3 are all complex constants.
 *
 *   partial x / partial n 
 *      = \sum_i C_i*ln(L_i)*L_i^n
 *      = \sum_i (C_i*ln(L_i)) * e^(n*ln(L_i))
 */
class SpiralDerivative {
	Complex ClnL[3]; // C_i*ln(L_i)
	Complex lnL[3]; // ln(L_i)
public:
	SpiralDerivative(
		const Complex *C,
		const Complex *L)
	{
		for (int i=0;i<3;i++) {
			lnL[i]=log(L[i]);
			ClnL[i]=C[i]*lnL[i];
		}
	}
	
	double operator() (double n) const {
		Complex sum(0,0);
		for (int i=0;i<3;i++)
			sum+=ClnL[i]*exp(n*lnL[i]);
		return real(sum);
	}
};


void MatrixPower2dImpl::extrema(const Vector2d &v,
	VirtualConsumer<Vector2d> &dest) const
{
	// Add endpoints to extrema:
	dest.consume(v);
	dest.consume(power(1.0,v));
	// Add zeros of derivative along each axis:
	for (int axis=0;axis<2;axis++) {
		Complex V[3], P_iV[3];
		v2d_copy(v,V);
		P_i.apply(V,P_iV);
		
		Complex C[3];
		for (int i=0;i<3;i++) {
			C[i]=P(axis,i)*P_iV[i];
		}
		SpiralDerivative s(C,D);
		double l=0.0, r=1.0;
		double sl=s(l), sr=s(r);
		if (sl*sr<0) { // There's a zero-crossing in there:
			double n=zero(secant,s,l,r,sl,sr,0.02);
			dest.consume(power(n,v));
		}
	}
}


