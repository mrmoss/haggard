/*
Camera and 4x4 projection matrix.
FIXME: This header is superceded by "osl/viewpoint.h".

Orion Sky Lawlor, olawlor@acm.org, 2003/2/21
*/
#ifndef __OSL_CAMERA_H
#define __OSL_CAMERA_H

#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif
#ifndef __OSL_VECTOR3D_H
#  include "osl/vector3d.h"
#endif
#ifndef __OSL_MATRIX3D_H
#  include "osl/matrix3d.h"
#endif

namespace osl { namespace graphics3d {

using osl::Vector2d;
using osl::Vector3d;
using osl::Matrix3d;

class Camera {
	Vector3d E; //< Eye point (PHIGS Projection Reference Point)
	Vector3d R; //< Projection plane origin (PHIGS View Reference Point)
	Vector3d X,Y; //< Pixel-length axes of projection plane (Y is PHIGS View Up Vector)
	Vector3d Z; //< Arbitrary-length normal to projection plane (PHIGS View Plane Normal)
	Matrix3d m; //< 4x4 projection matrix
	int wid,ht; //< Width and height of view plane, in pixels
	
	/// Fill our projection matrix m with values from E, R, X, Y, Z
	///   Assert: X, Y, and Z are orthogonal
	void buildM(void);
public:
	/// Build a camera at eye point E pointing toward R, with up vector Y.
	///   The up vector is not allowed to be parallel to E-R.
	/// This is an easy-to-use, but restricted (no off-axis) routine.
	/// It is normally followed by discretize or discretizeFlip.
	Camera(const Vector3d &E_,const Vector3d &R_,Vector3d Y_=Vector3d(0,1,1.0e-8));
	
	/// Make this camera, fresh-built with the above constructor, have
	///  this X and Y resolution and horizontal full field-of-view (degrees).
	/// This routine rescales X and Y to have the appropriate length for
	///  the field of view, and shifts the projection origin by (-w/2,-h/2). 
	void discretize(int w,int h,double hFOV);
	
	/// Like discretize, but flips the Y axis (for typical raster viewing)
	void discretizeFlip(int w,int h,double hFOV);
	
	
	/// Build a camera at eye point E for view plane with origin R
	///  and X and Y as pixel sizes.  Note X and Y must be orthogonal.
	/// This is a difficult-to-use, but completely general routine.
	Camera(const Vector3d &E_,const Vector3d &R_,
		const Vector3d &X_,const Vector3d &Y_,int w,int h);
	
	/// Return the center of projection (eye point)
	inline const Vector3d &getEye(void) const {return E;}
	/// Return the projection plane origin (View Reference Point)
	inline const Vector3d &getOrigin(void) const {return R;}
	/// Return the projection plane pixel-length X axis
	inline const Vector3d &getX(void) const {return X;}
	/// Return the projection plane pixel-length Y axis (View Up Vector)
	inline const Vector3d &getY(void) const {return Y;}
	
	/// Return the number of pixels in the X direction
	inline int getXsize(void) const { return wid; }
	/// Return the number of pixels in the Y direction
	inline int getYsize(void) const { return ht; }
	
	/// Return the 4x4 projection matrix.  This is a column-wise
	/// matrix, meaning the translation portion is the rightmost column.
	inline const Matrix3d &getMatrix(void) const {return m;}
	
	/// Project this point into the camera volume.
	///  The projected point is (return.x,return.y);
	///  return.z is 1.0/depth: +inf at the eye to 1 at the projection plane
	inline Vector3d project(const Vector3d &in) const {
		float w=1.0f/(float)(
		  m(3,0)*in.x+m(3,1)*in.y+m(3,2)*in.z+m(3,3)
		);
		return Vector3d(
		  w*(m(0,0)*in.x+m(0,1)*in.y+m(0,2)*in.z+m(0,3)),
		  w*(m(1,0)*in.x+m(1,1)*in.y+m(1,2)*in.z+m(1,3)),
		  w*(m(2,0)*in.x+m(2,1)*in.y+m(2,2)*in.z+m(2,3))
		);
	}
	
	/// Backproject this view plane point into world coordinates
	inline Vector3d viewplane(const Vector2d &v) const {
		return R+v.x*X+v.y*Y;
	}
};

}; }; /* end namespace */

#endif

