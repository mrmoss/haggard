#ifndef __OSL_MATRIXPOWER_H
#define __OSL_MATRIXPOWER_H

#include "osl/matrix2d.h"

namespace osl {
class MatrixPower2dImpl;

/**
Allows one to take fractional powers of a 2d translation matrix.

Uses eigenvalue decomposition of matrix M, so
   M = P D P^-1
where D is a diagonal matrix (the eigenvalues) and P is
a matrix of the eigenvectors.

Then matrix algebra tells us pow(M,exp)=P pow(D,exp) P^-1.
*/
class MatrixPower2d {
	MatrixPower2dImpl *impl;
public:
	/// Factorize this matrix.
	MatrixPower2d(const Matrix2d &src);
	~MatrixPower2d();
	
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

#endif /* __OSL_MATRIXPOWER_H */
