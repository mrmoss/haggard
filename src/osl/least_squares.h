/*
Orion's Standard Library
Orion Sky Lawlor, 9/23/1999

Provides routines for creating, reading in,
adding, multiplying, and inverting 2d
rotation/translation/scale/skew matrices.
*/
#ifndef __OSL_LEAST_SQUARES_H
#define __OSL_LEAST_SQUARES_H

#ifndef __OSL_MATRIX_H
#include "osl/matrix.h"
#endif
#ifndef __OSL_MATRIX2D_H
#include "osl/matrix2d.h"
#endif

namespace osl {

/**
 Find the vector x such that the Matrix Wt transpose times x is nearest to y
   in a least-squares sense.  Returns false if there is no unique nearest y.

 The matrix Wt should have more columns than rows--as many columns as
 knowns y; as many rows as unknowns x.
*/
bool solveLeastSquares(const Matrix &Wt,const matVector &y,matVector &x);

//Perform a least-squares planar fit
bool fitMatrix2d(int nPts,const Vector2d *src,const Vector2d *dest,
	Matrix2d &destFromSrc);

class FitResult {
public:
	bool failed; //Matrix was singular
	double maxErr; //Worst Point (before trimming)
	int nTrim; //Number of Points removed from fit
	explicit FitResult(bool failed_,double maxErr_,int nTrim_)
		:failed(failed_),maxErr(maxErr_),nTrim(nTrim_) { }
};

/*Fit a Matrix2d, then trim Points farther than trimThresh and re-fit
*/
FitResult fitMatrix2dTrim(double trimThresh,int nPts,const Vector2d *src,const Vector2d *dest,Matrix2d &destFromSrc);


}; //end namespace osl
#endif

