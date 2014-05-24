/**
Manages the OpenCV classes that detect bullseyes.
Output is a list of x,y centimeter coordinates, and
z axis is radians of CCW rotation (0.0 means pointing along X axis)

Orion Lawlor & Mike Moss, 2014-04 (Public Domain)
*/
#include "cyberalaska/bullseye_keeper.hpp"
#include "cyberalaska/bullcolor.h"

bullseye_keeper::bullseye_keeper(const int camera,const int width,const int height):
	camsize(width,height), _cap(camera),_width(width),_height(height)
{
	_cap.set(CV_CAP_PROP_FRAME_WIDTH,_width);
	_cap.set(CV_CAP_PROP_FRAME_HEIGHT,_height);

	const float camHeight = 183; // Set this to camera's height in cm.


	const float refHeight=203; 			// Don't change this
	const float refXFOV=445.008;		// Change this
	const float refYFOV=359.664;		// Change this

	float scale=camHeight/refHeight;
	float xFOV=refXFOV*scale;
	float yFOV=refYFOV*scale;

	// Add a negative sign to flip the Y axis to mathematical/GL "Y is up".
	yFOV=-yFOV;

	vec3 view(xFOV,yFOV,0.0); // camera's world-coordinates visible region
	vec3 offset(-xFOV/2.0,-yFOV/2.0,0); // manual fudge factor (centimeters)
	// camcoords=cyberalaska::coords(-0.5*view+offset,view); // 0,0 in center
	camcoords=cyberalaska::coords(offset,view); // 0,0 in bottom left corner

}

bullseye_keeper::operator bool() const
{
	return good();
}

bool bullseye_keeper::operator!() const
{
	return !static_cast<bool>(*this);
}

bool bullseye_keeper::good() const
{
	return _cap.isOpened();
}

std::vector<vec3> bullseye_keeper::update()
{
	static int frame_rate=0;
	std::vector<vec3> locations;

	cv::Mat frame;

	_cap>>frame;

	if(!frame.empty())
	{
		++frame_rate;

		//if(frame_rate%2)
		{
			cv::Mat gray;
			cv::cvtColor(frame,gray,CV_BGR2GRAY);

			bullseyeList bulls=findBullseyes(gray);

			locations.clear();

			for(unsigned int ii=0;ii<bulls.eyes.size();++ii)
			{
				bullcolor bc(bulls.eyes[ii],frame);

				//double x=bulls.eyes[ii].x-_width/2.0;
				//double y=bulls.eyes[ii].y-_height/2.0;
				vec3 xyz=camcoords.world_from_pixel(vec3(bulls.eyes[ii].x,bulls.eyes[ii].y,0.0),camsize);
				xyz.z=-bc.angle*M_PI/180.0; // angle in radians (Y is flipped)

				locations.push_back(xyz);
			}
		}
	}

	return locations;
}
