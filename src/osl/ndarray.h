/*
Orion's Standard Library
Orion Sky Lawlor, 9/24/1999
NAME:		osl/ndarray.h

DESCRIPTION:	Templatized N-dimentional array library

This file provides routines for dealing with N-dimentional matrices.
The data that makes up the Matrix is stored in one huge contiguous string.
Since the template parameter N is known at compile time, the compiler
can completely unroll all the N-indexed loops for very efficient code.

Define NDARRAY_BOUNDS_CHECK to get bounds-checking (default is off).
*/
#ifdef NDARRAY_BOUNDS_CHECK
#  define ndarray_check(min,x,max) assert((min<=(x))&&((x)<max))
#else
#  define ndarray_check(min,x,max) /*no check*/
#endif


typedef int indexType;

//An NdIndex is an index (element address) into an N-dimentional array.
template <int N> class NdIndex {
public:
	indexType dex[N];
	NdIndex();//No values specified
	NdIndex(indexType initValue);//All values set to initValue
	NdIndex(const indexType *initValues);//All values given
};

//An NdArray is a N-dimentional array of elemT's.
template <int N,class elemT> class NdArray {
public:
	elemT *data;//Pointer to string of data that makes up array.
		//This is organized first by dex[0], then dex[1], ... dex[N-1].
	NdIndex<N> size;//Size of data array in each dimention
	
	//Create a new empty array of the given size.
	NdArray(const NdIndex<N> &Nsize);
	
	//Access element ind of the array
	elemT &operator[] (const NdIndex<N> &ind);
};

/*An NdIterator is used to iterate through an NdArray
(can't nest "for" loops N times!)
 The iteration starts at the given min index and works to the given max index,
 proceeding in the order given, i.e.:

NdIterator<N> i(min,max)
do
    {array[i]=0;}
while (i.advance())

  has the same result as:

int i[N];
for (i[N-1]=min[N-1];i[N-1]<max[N-1];i[N-1]++)
	...
		for (i[1]=min[1];i[1]<max[1];i[1]++)
			for (i[0]=min[0];i[0]<max[0];i[0]++)
				{array[i]=0;}
*/
template <int N> class NdIterator:public NdIndex<N> {
public:
	NdIndex<N> min,max;
	
	//Constructor: we will run between min and max
	NdIterator(const NdIndex<N> &in_min,const NdIndex<N> &in_max);
	
	//Reset-- start us back at min
	void reset(void);
	
	//"advance"-- increment us through N-dimentional space.
	//  If we're done, return false; else true.
	bool advance(void);	
};

/************************************************
Implementations:
*/
//An NdIndex is an index (element address) into an N-dimentional array.
template <int N> NdIndex<N>::NdIndex() //No values specified
	{}
template <int N> NdIndex<N>::NdIndex(indexType initValue)//All values set to initValue
	{for (int i=0;i<N;i++) dex[i]=initValue;}
template <int N> NdIndex<N>::NdIndex(const indexType *initValues)//All values given
	{for (int i=0;i<N;i++) dex[i]=initValues[i];}

//An NdArray is a N-dimentional array of elemT's.
template <int N,class elemT> 
NdArray<N,elemT>::NdArray(const NdIndex<N> &Nsize) //Create a new empty array of the given size.
{
	indexType nElem=1;
	size=Nsize;
	for (int i=0;i<N;i++) nElem*=size.dex[i];
	data=new elemT[nElem];
}

template <int N,class elemT> 
elemT &
NdArray<N,elemT>::operator[] (const NdIndex<N> &ind)//Access element ind of the array
{
	ndarray_check(0,ind.dex[N-1],size.dex[N-1]);
	indexType index=ind.dex[N-1];
	for (int i=N-2;i>=0;i--)
	{
		ndarray_check(0,ind.dex[i],size.dex[i]);
		index=ind.dex[i]+size.dex[i]*index;
	}
	return data[index];
}

/*An NdIterator is used to iterate through an NdArray*/

template <int N>	//Constructor: ind will run between min and max
NdIterator<N>::NdIterator(const NdIndex<N> &in_min,const NdIndex<N> &in_max)
	{min=in_min;max=in_max;reset();}
	
template <int N>
void
NdIterator<N>::reset(void)//Reset-- start ind back at min
	{for (int i=0;i<N;i++) this->dex[i]=min.dex[i];}

//"advance"-- increment ind through N-dimentional space.
//  If we're done, return false; else true.
// Works by incrementing ind[0], and if it overflows ind[1], and ... ind[N-1], and if it overflows we're done.
template <int N>
bool
NdIterator<N>::advance(void)
{
	int i=0;
	this->dex[i]++;
	while (this->dex[i]>=max.dex[i])
	{//This dimention "overflowed"-- set it back to min and increment the next dimention (recursively)
		this->dex[i]=min.dex[i];
		if (++i==N) return false;//Check if we're done
		this->dex[i]++;//increment next dimention and loop again
	}
	return true;
}
