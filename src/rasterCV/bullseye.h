/**
OpenCV bulls-eye detector library: uses a gradient-voting scheme.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-06 (Public Domain)
*/
#ifndef __RASTERCV_BULLSEYE_H
#define __RASTERCV_BULLSEYE_H

#include <vector>
#include "opencv2/opencv.hpp" /* needed for cv::Mat */

/**
 Describes one bullseye
*/
class bullseyeInfo {
public:
	double x,y; ///< Center of bullseye, in pixels (polished to sub-pixel)
	double votes; ///< Approximate count of gradients agreeing this is the center

	/// For sorting by descending size:
	friend bool operator<(const bullseyeInfo &L,const bullseyeInfo &R) {
		return L.votes>R.votes;
	}
};

/**
 Describes detected bullseyes.
*/
class bullseyeList {
public:
	/* The center of each bullseye, sorted with most votes first. */
	std::vector<bullseyeInfo > eyes;
};

/**
  Find a list of bullseyes in this grayscale (single channel) source image.
*/
bullseyeList findBullseyes(const cv::Mat &grayImage, // source grayscale image, use cv::cvtColor(colorImg,grayImg,CV_BGR2GRAY);
	double minimumGradientMagnitude=60, // gradient steepness required to draw line (low values slower but can detect weaker)
	double minimumVotesPerEye=80, // minimum vote count to be an eye (low values more sensitive but false positives)
	double gradientVotePixels=20, // number of pixels to extend gradient (low values faster but only support small eyes)
	int minimumEyeDistance=10 // minimum distance between distinct bullseyes
	);


#endif /* defined (this header) */

