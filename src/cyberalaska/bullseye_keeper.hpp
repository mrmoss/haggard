/**
Manages the OpenCV classes that detect bullseyes.
Output is a list of x,y centimeter coordinates, and 
z axis is radians of CCW rotation (0.0 means pointing along X axis)

Orion Lawlor & Mike Moss, 2014-04 (Public Domain)
*/
#ifndef BULLSEYES_HPP
#define BULLSEYES_HPP

#include "opencv2/opencv.hpp"
#include <vector>
#include "cyberalaska/vec3.h"
using cyberalaska::vec3;
#include <algorithm>
#include "osl/vec2.h"
#include "cyberalaska/coords.h" /* for scaling to real world coordinates */

class bullseye_keeper
{
	public:
		bullseye_keeper(const int camera,const int width=640,const int height=480);
		operator bool() const;
		bool operator!() const;
		bool good() const;

		std::vector<vec3> update();

	private:
		cyberalaska::coords camcoords; // convert pixels to centimeters
		cyberalaska::image_size camsize;
		cv::VideoCapture _cap;
		int _width;
		int _height;
};

#endif
