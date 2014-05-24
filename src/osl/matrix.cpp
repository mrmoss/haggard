/*
Orion's Standard Library
Orion Sky Lawlor, 11/3/1999
NAME:		osl/matrix.h

DESCRIPTION:	C++ Matrix library (no templates)

This file provides routines for creating, reading in,
adding, multiplying, and inverting NxM double matrices.

Define MATRIX_BOUNDS_CHECK to get bounds-checking (default is off).
*/
#include "osl/matrix.h"
using namespace osl;

typedef double *realPtr;

//---------- vector

double matVector::dot(const matVector &v) const
{
	double ret=0.0;
	for (int i=0;i<len;i++)
		ret+=data[i]*v[i];
	return ret;
}


allocVector::allocVector(int len_,double initVal)
	:matVector(new double[len_],len_)
{
	for (int i=0;i<len_;i++)
		getData()[i]=initVal;
}

allocVector::allocVector(const double *dataToCopy,int len_)
	:matVector(new double[len_],len_)
{
	for (int i=0;i<len_;i++)
		getData()[i]=dataToCopy[i];	
}

allocVector::~allocVector()
{
	delete[] getData();
}

//---------- Matrix
void Matrix::allocate(int Nrow,int Ncol)//Allocate "data" to contain row x col
{
	allocRows=rows=Nrow;
	allocCols=cols=Ncol;
	if (allocRows!=-1)
	{
		data=new realPtr[allocRows];
		for (int r=0;r<allocRows;r++)
			data[r]=new double[allocCols];
	}
}
void Matrix::deallocate(void) //Free "data"
{
	if (allocRows!=-1)
	{
		for (int r=0;r<allocRows;r++)
			delete [] data[r];
		delete [] data;
	}
	data=(double **)NULL;
	allocRows=allocCols=rows=cols=-1;
}

//Creation routines:
Matrix::Matrix()//Create empty Matrix
{
	allocate(-1,-1);
}
Matrix::Matrix(int dim)//Create identity Matrix
{
	allocate(dim,dim);
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		data[r][c]=(r==c)?double(1):double(0);
}
Matrix::Matrix(int Nrow,int Ncol,const double initTo)//Create Matrix whose values are all initTo
{
	allocate(Nrow,Ncol);
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		data[r][c]=initTo;
}
Matrix::Matrix(int Nrow, int Ncol,const double **in_data)//Create Matrix whose data is given
{
	allocate(Nrow,Ncol);
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		data[r][c]=in_data[r][c];
}
Matrix::Matrix(const Matrix &m)//Copy given Matrix
{
	allocate(m.rows,m.cols);
	int r,c;
	for (r=0;r<rows;r++) for (c=0;c<cols;c++)
		data[r][c]=m(r,c);
}
Matrix::~Matrix()//Destructor
{
	deallocate();
}

//Set size to given.  If shrinking a Matrix, no
// reallocation is performed and no values are lost.
void Matrix::resize(int Nrow,int Ncol)
{
	if (Nrow<=allocRows && Ncol<=allocCols)
	{//No reallocation needed-- just set sizes
		rows=Nrow;cols=Ncol;
	} else
	{//Reallocation needed-- 
		deallocate();
		allocate(Nrow,Ncol);
	}
}

//Get a column as a vector (an expensive operation)
void Matrix::getCol(int col,double *dest) const
{
	for (int r=0;r<rows;r++)
		dest[r]=data[r][col];	
}

//Set the values in this column to these values
void Matrix::setCol(int col,const double *toWhat)
{
	for (int r=0;r<rows;r++)
		data[r][col]=toWhat[r];
}

void Matrix::getRow(int row,double *dest) const
{
	const double *rp=data[row];
	for (int c=0;c<cols;c++)
		dest[c]=rp[c];
}


//Row operations:
void Matrix::setRow(int row,const double *toWhat)//Set the values in this row to these values
{
	double *rp=data[row];
	for (int c=0;c<cols;c++)
		rp[c]=toWhat[c];
}
void Matrix::swapRow(int row1,int row2)//Swap these two rows
{
	double *tmp=data[row1];
	data[row1]=data[row2];
	data[row2]=tmp;
}
void Matrix::scaleRow(const double scaleBy,int rowSum)//data[rowSum][*]*=scale
{
	double *rpSum=data[rowSum];
	for (int c=0;c<cols;c++)
		rpSum[c]*=scaleBy;
}
void Matrix::scaleAddRow(int rowSrc,const double scaleBy,int rowSum)//data[rowSum][*]+=data[rowSrc][*]*scale
{
	double *rpSrc=data[rowSrc],*rpSum=data[rowSum];
	for (int c=0;c<cols;c++)
		rpSum[c]+=rpSrc[c]*scaleBy;
}

//Matrix operations:
//Add m to this Matrix.  m must be the same size as this Matrix
void Matrix::add(const Matrix &m)
{
	for (int r=0;r<rows;r++) for (int c=0;c<cols;c++)
		data[r][c]+=m(r,c);
}

//Solve this Matrix using (non-naive) gaussian elimination.  
//  Returns true if the solution was sucessful (non-singular), false otherwise.
// This Matrix must have more columns than rows.
bool Matrix::solve(void)
{
	int pivotCol,r,c;
	for (pivotCol=0;pivotCol<rows;pivotCol++)
	{
		//Find a suitable row to pivot on
		int pivotRow=-1;//Row to pivot on
		double pivotVal=0.0;
		for (r=pivotCol;r<rows;r++) {
			double val=fabs(data[r][pivotCol]);
			if (pivotVal<val)
				{pivotVal=val;pivotRow=r;}
		}
		if (pivotRow==-1) return false;//We only found zeros in a pivot column-- singular Matrix!
		//Swap that row into the pivot position
		if (pivotRow!=pivotCol)
			swapRow(pivotRow,pivotCol);
		pivotRow=pivotCol;
		scaleRow(double(1)/data[pivotRow][pivotCol],pivotRow);
		double *pr=data[pivotRow];
		//Scale the rest of the Matrix
		for (r=0;r<rows;r++)
			if (r!=pivotRow)
			{
				double *rest=data[r];
				double scale=rest[pivotCol];
				if (scale==0.0) continue; //Optimization
				for (c=pivotCol+1;c<cols;c++)
					rest[c]-=pr[c]*scale;
				rest[pivotCol]=0;
			}
	}
	return true;//It worked!
}

//Invert this Matrix using non-naive gaussian elimination.
//  Inv will be re-allocated to the same size as this Matrix.
//  This Matrix must be square.
//  Returns true if the solution was sucessful.
bool Matrix::invert(Matrix &inv,Matrix *tmp) const
{
	//Construct a temporary augmented Matrix, which contains us on the left and the identity Matrix on the right
	Matrix *aug;
	if (tmp) {
		(aug=tmp)->resize(rows,2*cols); 
	} else aug=new Matrix(rows,2*cols,double(0));
	int r,c;
	for (r=0;r<rows;r++)
	{
		double *curR=data[r],*augR=aug->data[r];
		for (c=0;c<cols;c++)
		{
			augR[c]=curR[c];//Augmented Matrix has us on the left;
			augR[cols+c]=0;
		}
		augR[cols+r]=double(1);//Augmented Matrix has identity on the right
	}
	//Row-reduce the augmented Matrix
	if (!aug->solve()) return false;//Row-reduction failed!  We're singular.
	//Copy "inv" from the right side of the augmented Matrix.  Left side is now identity (and useless)
	inv.resize(rows,cols);
	for (r=0;r<rows;r++)
	{
		double *invR=inv.data[r],*augR=aug->data[r];
		for (c=0;c<cols;c++)
			invR[c]=augR[cols+c];
	}
	if (tmp==NULL) delete aug;
	return true;//It worked!
}

//Put the transpose of this Matrix into the given Matrix,
// which will be re-allocated to hold it.
void Matrix::transpose(Matrix &dest) const
{
	dest.resize(cols,rows);
	int r,c;
	for (r=0;r<rows;r++)
		for (c=0;c<cols;c++)
			dest(c,r)=data[r][c];
}


//Set dest=this * by
//  Dest will be re-allocated to the appropriate size (this->rows x by->cols).
//  The number of rows of "by" must equal the number of columns of this Matrix.
void Matrix::product(const Matrix &by,Matrix &dest) const
{
	dest.resize(rows,by.cols);
	int destR,destC,inner;
	for (destR=0;destR<rows;destR++)
	{
		const double *dataR=data[destR];
		for (destC=0;destC<by.cols;destC++)
		{
		//Sum up value of dest.data[destR][destC]:
			double sum=double(0);
			for (inner=0;inner<cols;inner++)
				sum+=dataR[inner]*by.data[inner][destC];
			dest.data[destR][destC]=sum;
		}
	}
}

//Apply this Matrix to the given "vector".  
// Src must be at least as long as at the number of Matrix columns
// Dest must be at least as long as the number of Matrix rows
// Src and dest cannot be equal.
void Matrix::apply(const double *src,double *dest) const
{
	int r,c;
	//Sum the application of this Matrix to src
	for (r=0;r<rows;r++)
	{
		const double *dataR=data[r];
		double sum=0;
		for (c=0;c<cols;c++)
			sum+=dataR[c]*src[c];
		dest[r]=sum;
	}		
}



