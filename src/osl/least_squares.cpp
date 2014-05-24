/*
Orion's Standard Library
Orion Sky Lawlor, 10/8/2001

DESCRIPTION:	Linear least-squares library

This file provides routines for performing linear least
squares.  The routines are reasonably efficient, and should
scale to hundreds of unknowns and millions of data Points.
This is useful for, e.g., curve fitting.
*/
#include "osl/least_squares.h"
#include <vector>
using namespace osl;

/*
Find the vector x such that the Matrix Wt transpose times x is nearest to y.
Such an x is the solution of   Wt Wt (transpose) x = Wt y
Returns false if there is no unique nearest y (i.e., on degeneracy).
*/
bool osl::solveLeastSquares(const Matrix &Wt,const matVector &y,matVector &x)
{
	int nr=Wt.rows;
/*
	vassert(nc>=nr,"Least squares Matrix (transpose) should have more columns than rows!");
	vassert(nc==y.getSize(),"Target y vector should match least squares Matrix!");
	vassert(nr==x.getSize(),"Output x vector must be allocated with corRect size!");
*/
	
	//Compute A=Wt times Wt transpose as an augmented Matrix
	Matrix A(nr,nr+1);
	int r,c;
	for (r=0;r<nr;r++)
	for (c=r;c<nr;c++) {
		double e=Wt.getRow(r).dot(Wt.getRow(c));
		A(r,c)=A(c,r)=e;
	}
	
	//Compute t=Wt y
	allocVector t(nr);
	Wt.apply(y,t);
	A.setCol(nr,t);
	
	//Solve the system A x = t
	if (!A.solve()) return false;
	
	//Extract the solution x
	A.getCol(nr,x);
	
	return true;
}

/*
Build a 2D homogenous Matrix to approximate the mapping
from src to dest.
*/
bool osl::fitMatrix2d(int nPts,const Vector2d *src,const Vector2d *dest,Matrix2d &destFromSrc)
{
	const int nCoeff=3;//Number of output coefficients per axis
	Matrix Wt(nCoeff,nPts);
	allocVector y(nPts);
	double xSto[nCoeff];
	matVector x(xSto,nCoeff);
	for (int axis=0;axis<2;axis++) {
		for (int i=0;i<nPts;i++)
		{
			Wt(0,i)=src[i].x;
			Wt(1,i)=src[i].y;
			Wt(2,i)=1.0;
			y(i)=dest[i][axis];
		}
		if (!solveLeastSquares(Wt,y,x)) return false;
		destFromSrc(axis,0)=x(0);
		destFromSrc(axis,1)=x(1);
		destFromSrc(axis,2)=x(2);
	}
	destFromSrc(2,0)=0.0;
	destFromSrc(2,1)=0.0;
	destFromSrc(2,2)=1.0;
	return true;
}


/*Count the Points that would be trimmed at this threshold*/
static int countTrim(double trimThresh,int nPts,const Vector2d *s,const Vector2d *d,
	const Matrix2d &destFmSrc)
{
	int nTrim=0;
	double trimThreshSqr=trimThresh*trimThresh;
	for (int i=0;i<nPts;i++) {
		double curErrSqr=(destFmSrc.apply(s[i])-d[i]).magSqr();
		if (curErrSqr>trimThreshSqr) 
			nTrim++;
	}
	return nTrim;
}

/*Fit a Matrix2d, then trim and re-fit*/
FitResult osl::fitMatrix2dTrim(double trimThresh,int nPts,const Vector2d *src,const Vector2d *dest,Matrix2d &destFmSrc)
{
	std::vector<osl::Vector2d> s(nPts);
	std::vector<osl::Vector2d> d(nPts);
	int i;
	for (i=0;i<nPts;i++) {s[i]=src[i]; d[i]=dest[i];}
	bool status=false;
	int nTrimTot=0;
	double maxErrSqr=0.0;
	
	//Loop until we've trimmed the proper number of Points
	while (1) {
		if (nPts<3) //Trimmed too much!
			break;
		
		//Fit a Matrix to the remaining Points
		if (!fitMatrix2d(nPts,&s[0],&d[0],destFmSrc)) 
			break;
		
		//Home in on an acceptable tolerance-- never trim
		// too many Points at once (would be inaccurate)
		double curThresh=trimThresh;
		double threshGrowth=2.0;
		int nTrim=0;
		int trimAtMost=1+(int)(0.02*nPts);
		while(1) { //Count the Points that would be trimmed
			nTrim=countTrim(curThresh,nPts,&s[0],&d[0],destFmSrc);
			if (nTrim>trimAtMost)
			//Would cut too many Points-- relax threshold
				curThresh*=threshGrowth;
			else
				break;
		};
		if (nTrim==0 && curThresh>trimThresh)
			curThresh/=threshGrowth; //Went too far-- go back a bit
		
		//Trim those Points outside the current tolerance
		nTrim=0;
		double curThreshSqr=curThresh*curThresh;
		for (i=0;i<nPts;i++) {
			double curErrSqr=(destFmSrc.apply(s[i])-d[i]).magSqr();
			if (curErrSqr>maxErrSqr) maxErrSqr=curErrSqr;
			if (curErrSqr>curThreshSqr) 
			{ /*Swap out this bad Point*/
				nTrim++;
				--nPts;
				s[i]=s[nPts];
				d[i]=d[nPts];
				i--;
			}
		}
		nTrimTot+=nTrim;
		if (nTrim==0) 
		{ //We've finally trimmed enough Points
			status=true;
			break;
		}
	}
	return FitResult(!status,sqrt(maxErrSqr),nTrimTot);
}
