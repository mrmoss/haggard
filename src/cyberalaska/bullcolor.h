/**
Adds a color gradient based angle detection scheme to the bullseye detector.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-06 (Public Domain)
*/
#ifndef __CYBERALASKA__BULLCOLOR_H
#define __CYBERALASKA__BULLCOLOR_H

#include "../rasterCV/bullseye.h"
#include "../cyberalaska/vec3.h"

// Silly utility function: like atan2, but takes a vector and returns degrees.
//   The vector (1,0,0) is 0.0 degrees.
inline double atan2_deg(cyberalaska::vec3 &dir) {
	return atan2(dir.y,dir.x)*180.0/M_PI;
}

/**
  Extracts additional data from around a previously detected bulls-eye.
*/
class bullcolor : public bullseyeInfo {
public:
	cyberalaska::vec3 pixel; ///< pixel location of robot center (with z==0)
	float radius; ///< Estimate of bullseye radius (pixels)

	cv::Scalar color; ///< Whole-bullseye average color

	cyberalaska::vec3 dir; ///< Camera-coordinates direction vector (unit length)
	float angle; ///< 2D angle, as +-180 degrees (along camera's x axis -> 0.0 degrees)
	float confidence; ///< Confidence in our angle estimate (arbitrary units, 1.0 is decent)

	/**
	 Extract color data around this bullseye, and use it to estimate position
	*/
	bullcolor(const bullseyeInfo &bi,cv::Mat &rgbImage);
};


#endif

