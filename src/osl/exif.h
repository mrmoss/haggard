/*
Orion's Standard Library-- EXIF File reader
  Orion Sky Lawlor, olawlor@acm.org, 1/29/2003
*/
#ifndef __OSL_EXIF_H
#define __OSL_EXIF_H

#ifndef __OSL_IO_H
#  include "osl/io.h"
#endif

namespace osl { namespace calibrate {

/// The information stored in an EXIF image file,
///  produced by many digital cameras.
class Exif {
	void badImage(const char *why);
public:
	Exif(osl::io::InputStream &is,bool verbose);
	~Exif();
	
	/// Print a description of this EXIF information to stdout
	void print(void);
	
	//Special marker value used by strings
	static const char *emptyString;

// General parameters:
	/// Camera manufacturer, as human-readable string.
	String Make;
	
	/// Camera model number, as human-readable string.
	String Model;
	
	/// Date and time image was taken, formatted using
	///  ISO Date/time format ("YYYY:MM:DD HH:MM:SS")
	///  in some bizarre user-set local time zone.
	String DateTime;
	
	
// Exposure parameters (APEX="Additive Photographic EXposure"--power-of-two)
	/// Equivalent to film speed (e.g., AS100)
	double ISOSpeedRating;
	
	/// Shutter speed, APEX.  ShutterTime=2^(-TimeValue)
	double TimeValue;
	
	/// Measured brightness of scene, APEX
	double BrightnessValue;
	
	/// Aquisition aperture, APEX.  FNumber=2^(0.5*ApertureValue)
	double ApertureValue;
	
	/// Exposure manual override value, APEX
	double ExposureBias;
	
	/// Return the base-2 log of the scaling factor (i.e., APEX units) 
	///    to apply to image pixels to get power
	double getPowerValue(void) const;
	
	/// Return the actual scaling factor to apply to image pixels
	/// to get power.
	double getPower(void) const {
		return pow(2.0,getPowerValue());
	}
	
	/// If true, this is an sRGB image (gamma=2.2, white point=6500K)
	bool sRGB;
	
// Scene parameters
	/// Autofocus distance to subject, meters
	double SubjectDistance;
	
	/// Focal length, in millimeters
	double FocalLength;
	
	/// Aquired image size
	int wid,ht;
	
};


}; };

#endif
