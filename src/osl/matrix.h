/*
Orion's Standard Library
Orion Sky Lawlor, 11/3/1999
NAME:		osl/matrix.h

DESCRIPTION:	C++ Matrix library (no templates)

This file provides routines for creating, reading in,
adding, multiplying, and inverting NxM double matrices.
The Matrix data is allocated on the heap, so it's only 
efficient for large matricies.

Define MATRIX_BOUNDS_CHECK to get bounds-checking (default is off).
*/
#ifndef __OSL_MATRIX_H
#define __OSL_MATRIX_H
#include "osl/math_header.h"

namespace osl {

//Arbitrary-length vector of reals: non-allocating version
class matVector {
	double *data; //Alias of return data
	int len;
public:
	matVector(double *dataToPointAt,int len_) 
		{data=dataToPointAt; len=len_;}
	matVector(const double *dataToPointAt,int len_) 
		{data=(double *)dataToPointAt; len=len_;}
	//Default copy constructor, assignment operator

	inline int getLength(void) const {return len;}
	inline int getSize(void) const {return len;} /*synonym*/
	inline double *getData(void) {return data;}
	inline double *getPointer(void) {return data;}

	operator double* () {return data;}
	operator const double* () const {return data;}
	double &operator[](int x) {return data[x];}
	const double &operator[](int x) const {return data[x];}
	double &operator()(int x) {return data[x];}
	const double &operator()(int x) const {return data[x];}
	
	double dot(const matVector &v) const;
};

//As above, but dynamically allocates space using "new"
class allocVector : public matVector {
	//Don't use these:
	allocVector(const allocVector &v);
	allocVector &operator=(const allocVector &v);
public:
	allocVector(int len,double initVal=0.0);
	allocVector(const double *dataToCopy,int len);
	~allocVector();
};

class Matrix {
protected:
	void allocate(int Nrow,int Ncol);//Allocate "data" to contain row x col
	void deallocate(void);//Free "data"
	
	int allocRows,allocCols;//Number of rows and columns allocated
public:
	int rows, cols;//The number of rows and columns below.
	double **data;//The data contained in this Matrix.  
	
	// Format is row first, then column (i.e. data[row][column] or (row,colum)).
	double &operator() (int r,int c) {return data[r][c];}
	const double &operator() (int r,int c) const {return data[r][c];}
	
//Creation routines:
	Matrix();//Create a zero-by-zero Matrix (use reallocate later)
	Matrix(int dim);//Create identity Matrix
	Matrix(int Nrow,int Ncol,const double initTo=0.0);//Create Matrix whose values are all initTo
	Matrix(int Nrow, int Ncol,const double **data);//Create Matrix whose data is given
	Matrix(const Matrix &m);//Copy given Matrix
	~Matrix();//Destructor
	//Set size to given.  If shrinking a Matrix, no
	// reallocation is performed and no values are lost.
	void resize(int Nrow,int Ncol);

//Get a column as a vector (an expensive operation)
	void getCol(int col,double *dest) const;
//Set the values in this column to these values
	void setCol(int col,const double *toWhat);
//Row operations:
	//Get a row as a vector (a cheap operation)
	matVector getRow(int row) const {return matVector(data[row],cols);}
	void getRow(int row,double *dest) const;
	void setRow(int row,const double *toWhat);//Set the values in this row to these values
	void swapRow(int row1,int row2);//Swap these two rows
	void scaleRow(const double scaleBy,int rowSum);//data[rowSum][*]*=scale
	void scaleAddRow(int rowSrc,const double scaleBy,int rowSum);//data[rowSum][*]+=data[rowSrc][*]*scale

//Matrix operations:
//Add m to this Matrix.  m must be the same size as this Matrix
	void add(const Matrix &m);
	
//Solve this Matrix using (non-naive) gaussian elimination.  
//  Returns true if the solution was sucessful (non-singular), false otherwise.
// This Matrix must have more columns than rows.
	bool solve(void);
	
//Invert this Matrix using non-naive gaussian elimination.
// Inv will be re-allocated to the same size as this Matrix.
//  This Matrix must be square.
//  Returns true if the solution was sucessful.
	bool invert(Matrix &inverse,Matrix *tmp=0) const;
	
//Put the transpose of this Matrix into the given Matrix,
// which will be re-allocated to hold it.
	void transpose(Matrix &dest) const;
	
//Set dest=this * by
//  Dest will be re-allocated to the appropriate size (this->rows x by->cols).
//  The number of rows of by must equal the number of columns of this Matrix.
	void product(const Matrix &by,Matrix &dest) const;

//Apply this Matrix to the given "vector".  
// Src must be at least as long as at the number of Matrix columns
// Dest must be at least as long as the number of Matrix rows
// Src and dest cannot be equal.
	void apply(const double *src,double *dest) const;
};

}; //end namespace osl

#endif //__OSL_MATRIX_H


