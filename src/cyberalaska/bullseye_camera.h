/**
Grabs images from an OpenCV-supported camera, and passes them to the bullseye detector.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-06 (Public Domain)
*/
#ifndef __CYBERALASKA_BULLSEYE_CAMERA_H
#define __CYBERALASKA_BULLSEYE_CAMERA_H

#include "../cyberalaska/timestamp.h"
#include "../cyberalaska/bullcolor.h"
#include "../cyberalaska/coords.h"

/** Extract bullseye images from a camera. */
class bullseye_camera : public cyberalaska::timestamped {
public:
	/** Extract the latest copy of the detected bullseyes. 
		All bullseyes above our internal vote threshold are reported.
		Bullseyes are in descending vote count, so [0] is the most likely.
	
	  There is no point in calling this function unless 
	  	our timestamped::has_newer reports a new frame.
	*/
	virtual std::vector<bullcolor> extract() =0;
	
	/** Close the camera. */
	virtual ~bullseye_camera();
};

/** Create a camera.  Returns NULL if there was a problem creating the camera. */
bullseye_camera *make_bullseye_camera(int cvCameraNumber);

#endif

