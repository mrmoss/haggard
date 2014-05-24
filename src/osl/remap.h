/*
Orion's Standard Library
written by 
Orion Sky Lawlor, olawlor@acm.org, 06/07/2002

Very generic templated low-level image manipulation framework.
*/
#ifndef __OSL_REMAP_H
#define __OSL_REMAP_H

#ifndef __OSL_MATRIX2D_H
#  include "osl/matrix2d.h"
#endif
#ifndef __OSL_COLOR_H
#  include "osl/color.h"
#endif
#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif
#ifndef __OSL_RASTER_H
#  include "osl/raster.h"
#endif

namespace osl { namespace remap {

using osl::Matrix2d;
using osl::graphics2d::Color;
using osl::graphics2d::Rect;

/* Basic datatypes: 
a Source (SRC) is a class with a "pixel_t" and a "pixel_t getColor(x,y) const" method.  For example, a Raster is a Source.
*/

//Virtual method implementation of SRC protocol, taking int coordinates.
template <class DTYPE>
class VirtualSourceT {
public:
	virtual ~VirtualSourceT() {} /* Just avoids compiler warnings */
	typedef Color pixel_t;
	virtual pixel_t getColor(DTYPE x,DTYPE y) const =0;
};
typedef VirtualSourceT<int> VirtualIntSource;
typedef VirtualSourceT<double> VirtualRealSource;

//An adapter to let us use templated SRC's in non-templated code:
template <class SRC,class DTYPE>
class VirtualStreamSourceT : public VirtualSourceT<DTYPE> {
	SRC src; //(Templated) sub-source we read from
public:
	VirtualStreamSourceT(const SRC &src_) :src(src_) {}
	virtual Color getColor(DTYPE x,DTYPE y) const {
		return src.getColor(x,y);
	}
};
template <class SRC>
inline VirtualIntSource *newVirtualInt(const SRC &src_)
  { return new VirtualStreamSourceT<SRC,int>(src_); }
template <class SRC>
inline VirtualRealSource *newVirtualReal(const SRC &src_)
  { return new VirtualStreamSourceT<SRC,double>(src_); }


//This SRC just adjusts the output of another SRC:
template <class SRC>
class StreamT {
protected:
	SRC src; //Sub-source we read from
public:
	StreamT(const SRC &src_) :src(src_) {}
};

//This SRC applies or un-applies gamma correction to its requests
template <class SRC>
class GammaT : public StreamT<SRC> {
	float gamma;
public:
	typedef Color pixel_t; //Need resolution of a double Color to capture gamma without quantization
	GammaT(const SRC &src_,float g) 
		:StreamT<SRC>(src_), gamma(g) {}
	inline Color getColor(int x,int y) const {
		Color r=this->src.getColor(x,y);
		r.exp(gamma);
		return r;
	}
};
//Gamma-correct this source: apply dark-darkening nonlinear filter
template <class SRC>
inline GammaT<SRC> Gamma(float gamma,const SRC &src_) {
	return GammaT<SRC>(src_,gamma);
}
//Gamma-uncorrect this source: remove dark-darkening filter
template <class SRC>
inline GammaT<SRC> Ungamma(float gamma,const SRC &src_) {
	return GammaT<SRC>(src_,(float)(1.0/gamma));
}

//This SRC pins any requests outside the given rectangle.
template <class SRC,class DTYPE>
class PinT : public StreamT<SRC> {
	Rect r;
public:
	typedef typename SRC::pixel_t pixel_t;
	PinT(const SRC &src_,const Rect &r_) 
		:StreamT<SRC>(src_), r(r_) {}
	inline pixel_t getColor(DTYPE x,DTYPE y) const {
		if (x<r.left) x=r.left;
		else if (x>r.right) x=r.right;
		if (y<r.top) y=r.top;
		else if (y>r.bottom) y=r.bottom;
		return this->src.getColor(x,y);
	}
};
template <class SRC>
inline PinT<SRC,int> PinInt(const Rect &r_,const SRC &src_) {
	return PinT<SRC,int>(src_,r_);
}
template <class SRC>
inline PinT<SRC,double> PinReal(const Rect &r_,const SRC &src_) {
	return PinT<SRC,double>(src_,r_);
}

//This SRC clips off any requests not inside the given rectangle,
// returning the given background color instead.
template <class SRC,class DTYPE>
class ClipT : public StreamT<SRC> {
public:
	typedef typename SRC::pixel_t pixel_t;
	ClipT(const SRC &src_,const Rect &r_,const Color &c_) 
		:StreamT<SRC>(src_), r(r_), rejectColor(c_) {}
	inline pixel_t getColor(DTYPE x,DTYPE y) const {
		if (y<r.top || y>=r.bottom) return rejectColor;
		if (x<r.left || x>=r.right) return rejectColor;
		return this->src.getColor(x,y);
	}
private:
	Rect r;
	pixel_t rejectColor;
};
template <class SRC>
inline ClipT<SRC,int> ClipInt(const Rect &r_,Color reject,const SRC &src_) {
	return ClipT<SRC,int>(src_,r_,reject);
}
template <class SRC>
inline ClipT<SRC,int> ClipRaster(const SRC &src_,Color reject=Color::black) {
	return ClipT<SRC,int>(src_,src_,reject);
}
template <class SRC>
inline ClipT<SRC,double> ClipReal(const Rect &r_,Color reject,const SRC &src_) {
	return ClipT<SRC,double>(src_,r_,reject);
}


//This SRC shifts its requests by the given offset.
template <class SRC,class DTYPE>
class ShiftT : public StreamT<SRC> {
	DTYPE dx,dy;
public:
	typedef typename SRC::pixel_t pixel_t;
	ShiftT(const SRC &src_,DTYPE dx_,DTYPE dy_) 
		:StreamT<SRC>(src_), dx(dx_), dy(dy_) {}
	inline pixel_t getColor(DTYPE x,DTYPE y) const {
		return this->src.getColor(dx+x,dy+y);
	}
};
template <class SRC,class DTYPE>
inline ShiftT<SRC,DTYPE> Shift(DTYPE dx_,DTYPE dy_,const SRC &src_) {
	return ShiftT<SRC,DTYPE>(src_,-dx_,-dy_);
}
template <class SRC>
inline ShiftT<SRC,double> Shift(const Vector2d &d,const SRC &src_) {
	return Shift(d.x,d.y,src_);
}

//This SRC scales its requests by the given factors.
template <class SRC,class DTYPE>
class ScaleT : public StreamT<SRC> {
	DTYPE dx,dy;
public:
	typedef typename SRC::pixel_t pixel_t;
	ScaleT(const SRC &src_,DTYPE dx_,DTYPE dy_) 
		:StreamT<SRC>(src_), dx(dx_), dy(dy_) {}
	inline pixel_t getColor(DTYPE x,DTYPE y) const {
		return this->src.getColor(dx*x,dy*y);
	}
};

template <class SRC,class DTYPE>
inline ScaleT<SRC,DTYPE> ScaleOut(DTYPE dx_,DTYPE dy_,const SRC &src_) {
	return ScaleT<SRC,DTYPE>(src_,dx_,dy_);
}
template <class SRC>
inline ScaleT<SRC,double> ScaleIn(double dx_,double dy_,const SRC &src_) {
	return ScaleT<SRC,double>(src_,(double)(1.0/dx_),(double)(1.0/dy_));
}


//This SRC applies the given matrix to its requests
template <class SRC>
class MatrixT : public StreamT<SRC> {
	Matrix2d m;
public:
	typedef typename SRC::pixel_t pixel_t;
	MatrixT(const SRC &src_,const Matrix2d &m_) 
		:StreamT<SRC>(src_), m(m_) {}
	inline pixel_t getColor(double x,double y) const {
		Vector2d v=m.applyInline(Vector2d(x,y));
		return this->src.getColor(v.x,v.y);
	}
};
template <class SRC>
inline MatrixT<SRC> Matrix(const Matrix2d &m_,const SRC &src_) {
	return MatrixT<SRC>(src_,m_);
}

//This SRC extracts colors from a Raster via its (inlined) "at" method
template <class RAST> class RasterSourceT {
	const RAST &r;
public:
	typedef typename RAST::pixel_t pixel_t;
	RasterSourceT(const RAST &r_) :r(r_) {}
	inline pixel_t getColor(int x,int y) const {
		return r.at(x,y);
	}
};
template <class RAST>
inline RasterSourceT<RAST> RasterSource(const RAST &r_) {
	return RasterSourceT<RAST>(r_);
}

//This SRC accepts floating-point color requests and makes
// only integer-coordinate color requests, via nearest-neighbor rounding.
template <class SRC>
class NearestT : public StreamT<SRC> {
public:
	typedef typename SRC::pixel_t pixel_t;
	NearestT(const SRC &src_) :StreamT<SRC>(src_) {}
	inline pixel_t getColor(double x,double y) const {
		return this->src.getColor((int)floor(x), (int)floor(y));
	}
};
template <class SRC>
inline NearestT<SRC> Nearest(const SRC &src_) {
	return NearestT<SRC>(src_);
}

//This SRC accepts floating-point color requests and makes
// only integer-coordinate color requests, via bilinear interpolation.
template <class SRC>
class BilinearT : public StreamT<SRC> {
public:
	typedef Color pixel_t; //Need to make a Color to get pixel arithmetic
	BilinearT(const SRC &src_) :StreamT<SRC>(src_) {}
	inline Color getColor(double x,double y) const {
		//x-=0.5; y-=0.5;
		int ix=(int)floor(x), iy=(int)floor(y);
		double dx=x-ix, dy=y-iy;
		Color tl=this->src.getColor(ix,iy  ), tr=this->src.getColor(ix+1,iy);
		Color bl=this->src.getColor(ix,iy+1), br=this->src.getColor(ix+1,iy+1);
		Color top=tl+dx*(tr-tl);
		Color bot=bl+dx*(br-bl);
		return top+dy*(bot-top);
	}
};
template <class SRC>
inline BilinearT<SRC> Bilinear(const SRC &src_) {
	return BilinearT<SRC>(src_);
}

//This SRC returns the area OP of the requested locations, which
// are xSz-by-ySz pixel rectangles out of the source image.
//OP is a class with "init()", "add(c)", and "Color extract()" methods.
template <class SRC,class OP>
class AreaT : public StreamT<SRC> {
	int xSz,ySz;
	OP op;
public:
	typedef typename OP::pixel_t pixel_t;
	AreaT(const SRC &src_,const OP &op_,int xSz_,int ySz_) 
		:StreamT<SRC>(src_), xSz(xSz_), ySz(ySz_), op(op_)  {}
	inline pixel_t getColor(int sx,int sy) const {
		OP lop=op;
		for (int y=0;y<ySz;y++)
		for (int x=0;x<xSz;x++)
			lop.add(this->src.getColor(x+sx,y+sy));
		return lop.extract();
	}
};
template <class SRC,class OP>
inline AreaT<SRC,OP> Area(int xSz_,int ySz_,const OP &op_,const SRC &src_) {
	return AreaT<SRC,OP>(src_,op_,xSz_,ySz_);
}

//Area operations:
class AverageOp {
	Color sum; int n;
public:
	AverageOp() {sum=Color::black; n=0;}
	typedef Color pixel_t;
	void add(const Color &src) {sum+=src; n++;}
	Color extract(void) {return sum*(1.0/n);}
};
class MaxOp {
	Color cur;
public:
	MaxOp() {cur=Color::white;}
	typedef Color pixel_t;
	void add(const Color &src) {
		for (int i=0;i<4;i++) if (cur[i]<src[i]) cur[i]=src[i];
	}
	Color extract(void) {return cur;}
};
class MinOp {
	Color cur;
public:
	typedef Color pixel_t;
	MinOp() {cur=Color::clear;}
	void add(const Color &src) {
		for (int i=0;i<4;i++) if (cur[i]>src[i]) cur[i]=src[i];
	}
	Color extract(void) {return cur;}
};


template <class SRC>
inline ShiftT<ScaleT<ShiftT<BilinearT<SRC>,double >,double >,double >
 BilinearScale(double scale,const SRC &in) 
{
	return 	Shift((double)-0.5,(double)-0.5,
		  ScaleIn(scale,scale,
		    Shift((double)0.5,(double)0.5,
		      Bilinear(in)
		    )
		  )
		);
}

//Copy the pixel source SRC into this rectangle of this DEST:
template <class SRC>
inline void copy(osl::graphics2d::Raster &dest,const Rect &r,SRC src)
{
	for (int y=r.top;y<r.bottom;y++)
	for (int x=r.left;x<r.right;x++)
		dest.setColor(x,y,src.getColor(x,y));
}
template <class SRC>
inline void copy(osl::graphics2d::RgbaRaster &dest,const Rect &r,SRC src)
{
	for (int y=r.top;y<r.bottom;y++)
	for (int x=r.left;x<r.right;x++) {
		osl::graphics2d::RgbaPixel p(src.getColor(x,y));
		dest.at(x,y)=p;
	}
}


}; };

#endif
