/*
Orion's Standard Library-- Calibrated Raster Images
  Orion Sky Lawlor, olawlor@acm.org, 1/26/2003
*/
#ifndef __OSL_CALIBRATE_H
#define __OSL_CALIBRATE_H

#ifndef __OSL_RASTER_H
#  include "osl/raster.h"
#endif
#ifndef __OSL_COLOR_H
#  include "osl/color.h"
#endif
#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif
#ifndef __OSL_VECTOR3D_H
#  include "osl/vector3d.h"
#endif

namespace osl { namespace calibrate {

using osl::graphics2d::Raster;
using osl::graphics2d::ColorRaster;
using osl::graphics2d::RgbaRaster;
using osl::graphics2d::Color;

/// Describes the date an image was taken, using the Gregorian calendar.
struct ImageDate {
	int year; ///< Years AD
	int month; ///< Month number, 1-12
	int day; ///< Day of month, 1-31
	double hour; ///< Hours since midnight
};

/// Describes the location on the Earth the image was taken.
struct ImageLocation {
	double latitude; ///< Degrees north of equator.
	double longitude; ///< Degrees east of greenwhich.
};

/// Private image calibration data.
class PrivateImageCalibration {
public:
	virtual ~PrivateImageCalibration();
};

/**
 * Describes a radiometrically calibrated camera's response.
 */
class CameraRadiometry {
public:
	/// This silly, dangerous macro gives the maximum 
	///   "data number" (DN), or pixel value.
	enum {MAX_DN=256};
	
	/// Returns linear brightness given DN. [Radiance]
	///   The maximum value dnToBright[MAX_DN-1] is always 1.0
	float dnToBright[MAX_DN];
	
	enum {nBrightness=4};
	/**
	 * Radial radiometric distortion coefficients.
	 *  Given a native camera input radius r, the 
	 *  brightness compensation factor b is:
	 *       b = 1.0 + sum_i=0..nBrightness-1 r^(2*i+2) radialBrightness[i]
	 * [radiance fraction / pixels^(2*i+2)]
	 */
	double radialBrightness[nBrightness];
	
	/// Return the factor to scale image brightness by, to 
	///  compensate for vignetting and other radius-dependent effects.
	///  @param r2 Squared distance from camera center pixel, expressed
	///            as a fraction of the scene width.
	inline double getBrightnessCompensation(double r2) const {
		return (((
		 radialBrightness[3]*r2
		+radialBrightness[2])*r2
		+radialBrightness[1])*r2
		+radialBrightness[0])*r2
		+1.0;
	}
	
	/// Convert parameters from per-scenewidth
	///   to per-pixel.
	void discretize(int wid,int ht);
	
	CameraRadiometry();
};

/**
 * Describes a geometrically and radiomatrically calibrated image.
 */
class ImageCalibration {
public:
	/// Date the image was taken, in UTC.
	ImageDate date;
	
	/// Location on the earth the image was taken.
	ImageLocation location;
	
/// Geometric calibration parameters:
	/// Center of projection. [pixels]
	Vector2d center;
	
	/**
	 * Size of an undistorted pixel, when projected to the z=1 plane
	 * if the camera is at the origin, pointing in the +z direction.
	 * [distance/pixel]
	 */
	Vector2d pixelSize;
	
	/**
	 * Radial geometric distortion coefficient.  Given an 
	 *  undistorted output radius r, the actual camera 
	 *  input radius r_i is:
	 *        r_i = r + r^3 * radialDistortion3
	 * [pixels/(pixel^3)]
	 */
	double radialDistortion3; 
	
	/// Return the actual camera pixel for this undistorted pixel:
	inline Vector2d getCameraLocation(const Vector2d &v) const {
		Vector2d rVec=(v-center);
		double r2=rVec.magSqr();
		return center+rVec*(1+r2*radialDistortion3);
		// Note: this is like rVec.mag()*(r+r*r*r*radialDist3)
	}
	
/// Radiometric calibration parameters:
	/// Image's intrinsic, linear brightness. [radiance]
	double brightness;
	inline double getBrightness(void) const {return brightness;}
	
	/// Image's scaling factor.  This is the linear
	///  value to scale up to unit brightness, by default
	///  this is the image's intrinsic brightness. [radiance]
	double scaling;
	
	CameraRadiometry radiometry;
	
	/// Return the factor to scale image brightness by, to 
	///  compensate for vignetting and other location-dependent effects.
	inline double getBrightnessCompensation(const Vector2d &v) const {
		double r2=(v-center).magSqr();
		return radiometry.getBrightnessCompensation(r2);
	}
	
	/// Private image data (blur kernel, etc.)
	PrivateImageCalibration *privateData;
	
	/// Compute center, and convert radius-dependent
	///  parameters from per-scenewidth to per-pixel.
	void discretize(int wid,int ht,double HFov);
	
	ImageCalibration();
	~ImageCalibration();
};

/**
 * Read this EXIF image's calibration information into dest.
 */
void ExifCalibration(const char *fileName,ImageCalibration &dest);


/**
 * A radiometrically and geometrically calibrated image.
 */
class CalibratedRaster : public ColorRaster {
	ImageCalibration cal;
public:
	/**
	 * Read and calibrate this (EXIF) image file.
	 *   scaling is the brightness scale to use, or the 
	 * image's max brightness if not given.  E.g., to 
	 * make two images have the same brightness scale, 
	 * use the same value here, like img1.getCalibration().getBrightness().
	 */
	CalibratedRaster(const char *fName,double scaling=-1);

	/**
	 * Calibrate the image src using the calibration
	 * information listed in cal.
	 */
	CalibratedRaster(const Raster &src,const ImageCalibration &cal)
	{ calibrate(src,cal); }

	/**
	 * Calibrate the image src using the calibration
	 * information listed in cal.
	 */
	void calibrate(const Raster &src,const ImageCalibration &cal);

	const ImageCalibration &getCalibration(void) const { return cal; }

	~CalibratedRaster();
};




}; };

#endif
