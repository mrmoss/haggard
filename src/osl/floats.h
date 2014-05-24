/**
 SSE and AVX implementation of Dr. Lawlor's "floats" class.
 Dr. Orion Lawlor, lawlor@alaska.edu, 2011-10-17 (public domain)
*/
#ifndef __OSL_FLOATS_H__

#ifdef __AVX__
#include <immintrin.h> /* Intel's AVX intrinsics header */

class floats; // forward declaration

// One set of boolean values
class bools {
	__m256 v; /* 8 boolean values, represented as all-zeros or all-ones 32-bit masks */
public:
	enum {n=8}; /* number of bools inside us */
	bools(__m256 val) {v=val;}
	__m256 get(void) const {return v;}
	
	/* Combines sets of logical operations */
	bools operator&&(const bools &rhs) const {return _mm256_and_ps(v,rhs.v);}
	bools operator||(const bools &rhs) const {return _mm256_or_ps(v,rhs.v);}
	bools operator!=(const bools &rhs) const {return _mm256_xor_ps(v,rhs.v);}
	
	/* Use masking to combine the then_part (for true bools) and else part (for false bools). */
	floats if_then_else(const floats &then,const floats &else_part) const; 
	
	/** 
	  Return true if *all* our bools are equal to this single value.
	*/
	bool operator==(bool allvalue) const {
		int m=_mm256_movemask_ps(v); /* 8 bits == high bits of each of our bools */
		if (allvalue==false)    return m==0; /* all false */
		else /*allvalue==true*/ return m==255; /* all true (every bit set) */
	}
	
	/**
	  Return true if *any* of our bools are true.
	*/
	bool any(void) const {
		return _mm256_movemask_ps(v)!=0;
	}
};


/**
  Represents an entire set of float values.
*/
class floats {
	__m256 v; /* 8 floating point values */
public:
	enum {n=8}; /* number of floats inside us */
	floats() {}
	floats(__m256 val) {v=val;}
	void operator=(__m256 val) {v=val;}
	__m256 get(void) const {return v;}
	floats(float x) {v=_mm256_broadcast_ss(&x);}
	void operator=(float x) {v=_mm256_broadcast_ss(&x);}
	
	floats(const float *src) {v=_mm256_loadu_ps(src);}
	void operator=(const float *src) {v=_mm256_loadu_ps(src);}

	/* Load from 32-byte aligned memory */
	void load_aligned(const float *src) {v=_mm256_load_ps(src);}
	
	/** Basic arithmetic, returning floats */
	friend floats operator+(const floats &lhs,const floats &rhs)
		{return _mm256_add_ps(lhs.v,rhs.v);}
	friend floats operator-(const floats &lhs,const floats &rhs)
		{return _mm256_sub_ps(lhs.v,rhs.v);}
	friend floats operator*(const floats &lhs,const floats &rhs)
		{return _mm256_mul_ps(lhs.v,rhs.v);}
	friend floats operator/(const floats &lhs,const floats &rhs)
		{return _mm256_div_ps(lhs.v,rhs.v);}
	floats operator+=(const floats &rhs) {v=_mm256_add_ps(v,rhs.v); return *this; }
	floats operator-=(const floats &rhs) {v=_mm256_sub_ps(v,rhs.v); return *this; }
	floats operator*=(const floats &rhs) {v=_mm256_mul_ps(v,rhs.v); return *this; }
	floats operator/=(const floats &rhs) {v=_mm256_div_ps(v,rhs.v); return *this; }
	
	/** Comparisons, returning "bools" */
	bools operator==(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_EQ_OQ);}
	bools operator!=(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_NEQ_OQ);}
	bools operator<(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_LT_OQ);}
	bools operator<=(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_LE_OQ);}
	bools operator>(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_GT_OQ);}
	bools operator>=(const floats &rhs) const {return _mm256_cmp_ps(v,rhs.v,_CMP_GE_OQ);}

	/* Store to unaligned memory */
	void store(float *ptr) const { _mm256_storeu_ps(ptr,v); }
	/* Store with mask */
	void store_mask(float *ptr,const bools &mask) const 
		{ _mm256_maskstore_ps(ptr,v,mask.get()); }
	/* Store to 256-bit aligned memory (if not aligned, will segfault!) */
	void store_aligned(float *ptr) const { _mm256_store_ps(ptr,v); }

	/* Extract one float from our set.  index must be between 0 and 7 */
	float &operator[](int index) { return ((float *)&v)[index]; }
	float operator[](int index) const { return ((const float *)&v)[index]; }

	friend ostream &operator<<(ostream &o,const floats &y) {
		for (int i=0;i<n;i++) o<<y[i]<<" ";
		return o;
	}
};

inline floats bools::if_then_else(const floats &then,const floats &else_part) const {
	return _mm256_or_ps( _mm256_and_ps(v,then.get()),
		 _mm256_andnot_ps(v, else_part.get())
	);
}
inline floats max(const floats &a,const floats &b) {
	return _mm256_max_ps(a.get(),b.get());
}
inline floats min(const floats &a,const floats &b) {
	return _mm256_min_ps(a.get(),b.get());
}
inline floats sqrt(const floats &v) {return _mm256_sqrt_ps(v.get());}
inline floats rsqrt(const floats &v) {return _mm256_rsqrt_ps(v.get());}

#elif defined(__SSE__) /* SSE implementation of above */

#include <xmmintrin.h> /* Intel's SSE intrinsics header */

class floats; // forward declaration

// One set of boolean values
class bools {
	__m128 v; /* 4 boolean values, represented as all-zeros or all-ones 32-bit masks */
public:
	enum {n=4}; /* number of bools inside us */
	bools(__m128 val) {v=val;}
	__m128 get(void) const {return v;}
	
	/* Combines sets of logical operations */
	bools operator&&(const bools &rhs) const {return _mm_and_ps(v,rhs.v);}
	bools operator||(const bools &rhs) const {return _mm_or_ps(v,rhs.v);}
	bools operator!=(const bools &rhs) const {return _mm_xor_ps(v,rhs.v);}
	
	/* Use masking to combine the then_part (for true bools) and else part (for false bools). */
	floats if_then_else(const floats &then,const floats &else_part) const; 
	
	/** 
	  Return true if *all* our bools are equal to this single value.
	*/
	bool operator==(bool allvalue) const {
		int m=_mm_movemask_ps(v); /* 4 bits == high bits of each of our bools */
		if (allvalue==false)    return m==0; /* all false */
		else /*allvalue==true*/ return m==15; /* all true (every bit set) */
	}
	
	/**
	  Return true if *any* of our bools are true.
	*/
	bool any(void) const {
		return _mm_movemask_ps(v)!=0;
	}
};


/**
  Represents an entire set of float values.
*/
class floats {
	__m128 v; /* 4 floating point values */
public:
	enum {n=4}; /* number of floats inside us */
	floats() {}
	floats(__m128 val) {v=val;}
	void operator=(__m128 val) {v=val;}
	__m128 get(void) const {return v;}
	floats(float x) {v=_mm_load1_ps(&x);}
	void operator=(float x) {v=_mm_load1_ps(&x);}
	
	floats(float a,float b,float c,float d) 
		{v=_mm_setr_ps(a,b,c,d);}
	
	// By default, assume data is not aligned
	floats(const float *src) {v=_mm_loadu_ps(src);}
	void operator=(const float *src) {v=_mm_loadu_ps(src);}
	
	// Use this if your data is 16-byte aligned.
	void load_aligned(const float *src) {v=_mm_load_ps(src);}
	
	/** Basic arithmetic, returning floats */
	friend floats operator+(const floats &lhs,const floats &rhs)
		{return _mm_add_ps(lhs.v,rhs.v);}
	friend floats operator-(const floats &lhs,const floats &rhs)
		{return _mm_sub_ps(lhs.v,rhs.v);}
	friend floats operator*(const floats &lhs,const floats &rhs)
		{return _mm_mul_ps(lhs.v,rhs.v);}
	friend floats operator/(const floats &lhs,const floats &rhs)
		{return _mm_div_ps(lhs.v,rhs.v);}
	floats operator+=(const floats &rhs) {v=_mm_add_ps(v,rhs.v); return *this; }
	floats operator-=(const floats &rhs) {v=_mm_sub_ps(v,rhs.v); return *this; }
	floats operator*=(const floats &rhs) {v=_mm_mul_ps(v,rhs.v); return *this; }
	floats operator/=(const floats &rhs) {v=_mm_div_ps(v,rhs.v); return *this; }
	
	/** Comparisons, returning "bools" */
	bools operator==(const floats &rhs) const {return _mm_cmpeq_ps (v,rhs.v);}
	bools operator!=(const floats &rhs) const {return _mm_cmpneq_ps(v,rhs.v);}
	bools operator<(const floats &rhs) const  {return _mm_cmplt_ps (v,rhs.v);}
	bools operator<=(const floats &rhs) const {return _mm_cmple_ps (v,rhs.v);}
	bools operator>(const floats &rhs) const  {return _mm_cmpgt_ps (v,rhs.v);}
	bools operator>=(const floats &rhs) const {return _mm_cmpge_ps (v,rhs.v);}

	/* Store to unaligned memory */
	void store(float *ptr) const { _mm_storeu_ps(ptr,v); }
	/* Store to 128-bit aligned memory (if not aligned, will segfault!) */
	void store_aligned(float *ptr) const { _mm_store_ps(ptr,v); }

	/* Extract one float from our set.  index must be between 0 and 3 */
	float &operator[](int index) { return ((float *)&v)[index]; }
	float operator[](int index) const { return ((const float *)&v)[index]; }

	friend ostream &operator<<(ostream &o,const floats &y) {
		for (int i=0;i<n;i++) o<<y[i]<<" ";
		return o;
	}
};

inline floats bools::if_then_else(const floats &then,const floats &else_part) const {
	return _mm_or_ps( _mm_and_ps(v,then.get()),
		 _mm_andnot_ps(v, else_part.get())
	);
}
inline floats max(const floats &a,const floats &b) {
	return _mm_max_ps(a.get(),b.get());
}
inline floats min(const floats &a,const floats &b) {
	return _mm_min_ps(a.get(),b.get());
}
inline floats sqrt(const floats &v) {return _mm_sqrt_ps(v.get());}
inline floats rsqrt(const floats &v) {return _mm_rsqrt_ps(v.get());}


#else /* FIXME: altivec, etc */
#error "You need some sort of SSE or AVX intrinsics for osl/floats.h to work."
#endif

#endif /* defined(this header) */
