/**
A group of robots.

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-05 (Public Domain)
*/
#ifndef __CYBERALASKA__ROBOTWORLD_H
#define __CYBERALASKA__ROBOTWORLD_H

#include "../cyberalaska/robot.h"
#include "../cyberalaska/bullseye_camera.h"

namespace cyberalaska {

/**
  This class is our crossroads for integrating different modules.
*/
class robotworld {
public:
	/// A list of ground robots.
	object_array<robot> ground;

	/// A list of air robots.
	object_array<robot> air;
	
	/// A list of cameras.
	object_array<bullseye_camera> camera;
	
	
	/// Set up the robot world.  Check the length of the lists to see if 
	///  it's acceptable for your purposes.
	robotworld(int argc,char *argv[]);
};


};
#endif

