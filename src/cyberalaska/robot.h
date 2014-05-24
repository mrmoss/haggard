/**
Abstract interface satisfied by all robots:
A robot is a collection of sensors and actuators

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-04 (Public Domain)
*/
#ifndef __CYBERALASKA__ROBOT_H
#define __CYBERALASKA__ROBOT_H
#include "../cyberalaska/sensor.h"
#include <stdexcept>
#include <vector>

namespace cyberalaska {

/**
 A list of sensors or actuators.
 This interface is modeled after JavaScript's builtin Array.
*/
template <class OBJECT>
class object_array {
	/** These are the objects in our array.
	  They're stored by pointer, not by value, so they can be subclasses.
	*/
	std::vector<OBJECT *> objects;
public:
	/** This is the number of objects in our array, or 0 if we're empty.
	*/
	int length;

	/** Access an object by index.
	    Thows an exception if the index doesn't exist.
	*/
	OBJECT &operator[](int index) {
		if (index<0 || index>=length) throw std::runtime_error("Robot object array index out of bounds!");
		return *objects[index];
	}

	/** Add this object to the end of our array.
	    The object is passed by pointer, an MUST be allocated with new.
	*/
	void push(OBJECT *obj) {
		if (obj==NULL) return; /* this allows us to x.push(tryCreate()) */
		objects.push_back(obj);
		length++;
	}

	object_array() {
		length=0;
	}
	// To destroy the array, destroy each object.
	~object_array() {
		for (int o=0;o<length;o++) delete objects[o];
		length=0;
	}
private:
	// Do not copy or assign arrays.
	object_array(const object_array<OBJECT> &obj);
	void operator=(const object_array<OBJECT> &obj);
};


/**
  An actuator represents a way for the robot to affect the world,
  like a motor, heater, etc.
*/
class actuator_t : public timestamped {
public:
	/** Contains a programmatically commanded value.
	   0.0 means neutral (do nothing).
	   +1.0 means maximum forward
	   -1.0 means maximum backward
	*/
	double command;

	/// Set the current command to this value
	void write(double new_command) {
		command=new_command;
		update_timestamp();
	}

	/// We first multiply the above by this value (default is 1.0)
	double scale;
	/// Then we add this adjustment (default is 0.0)
	double offset;

	/// Finally, we clamp the value to this limit (default is 1.0)
	double limit;

	/** Extract the current scaled, offset, and limited value. */
	double read() const {
		double v=command;
		v*=scale;
		v+=offset;
		if (v<-limit) return -limit;
		if (v>+limit) return +limit;
		else return v;
	}

	/** A human-readable actuator class.
		"torque" commands a motor's torque;
		"speed" commands a motor's speed;
		"angle" commands a motor's position;
		"heat" commands a heater's output;
		"temp" commands a heater's target;
	*/
	std::string type;

	actuator_t()
		:command(0.0), scale(1.0), offset(0.0), limit(1.0), type("unknown") {}

	template <class PUP>
	void pup(PUP &p) {
		pup(p,command,"command");
		pup(p,scale,"scale");
		pup(p,offset,"offset");
		pup(p,limit,"limit");
		pup(p,type,"type");
		timestamped::pup(p);
	}
};


/**
 Describes a general robot, a collection of sensors and actuators.
 Note that a robot data structure is fundamentally *passive*, used
 to shuttle data between active control and communication elements.
*/
class robot : public timestamped {
public:
	/** This is the robot's metadata: model number, version, etc. */
	const metadata_general &metadata;

	/** This is the current communication status.
	  SUBTLE: it's a const char * because it's modified by a communication thread,
	  but could be read at any time, which can cause problems with std::string.
	*/
	const char *status;

	/**
	  Describes how the robot can move around:
	  	"none" for no motion, like a furnace
	  	"car" for drive forward/backward and steer
	  	"tank" for differential drive, can pivot in place
	  	"crab" for translation wheels (e.g., meccanum)
	  	"fly" for fixed-wing aircraft, must circle to maintain altitude
	  	"heli" for hover-capable
	*/
	std::string mobility;

	/**
	  Motion control actuators.  In all cases, 0.0 is neutral,
	  +1.0 is maximum forward, and -1.0 is maximum backwards (if applicable).

	  The number and meaning of the channels depends on mobility:
		"car": [0] is gas+/brake-, [1] is steering left-/right+.
	  	"tank": [0] is left track, [1] is right track.
	  	"crab": [0] is back left, [1] is back right, [2] is front left, [3] is front right
	  	"fly" or "heli": [0] is throttle (can't be negative), [1] is aileron or roll, [2] is rudder or yaw, [3] is elevator or pitch
	*/
	object_array<actuator_t> drive;


	/// List of actuators other than drive actuators.
	///  Exact values are robot-dependant, but might point a camera, move an arm, etc.
	object_array<actuator_t> act;

	/// Each sensor on the robot.
	object_array<sensor_t> sense;


/// Location control
	/// The robot's current world-coordinates location
	vec3 location;
	/// Use this function to update the robot's location, and update the timestamp
	void set_location(const vec3 &loc) {
		location=loc;
		update_timestamp();
	}

	/// The robot's current direction (2D yaw angle), in world coordinates.
	///   This is an angle, in degrees, from -180 to +180.
	///   An angle of 0 means the robot is driving down the +x axis.
	double angle;

	/// The robot's current orientation (3D rotation), in world coordinates.
	///   The robot's local coordinates start out aligned with 3D world coordinates.
	///   Robot X: points forward, in direction of robot's travel
	///   Robot Y: points to the robot's left
	///   Robot Z: points up (toward the sky)
	ortho_frame coordinates;


	robot(const metadata_general &metadata_)
		:metadata(metadata_),
		 status("connecting"),
		 mobility("unknown"),
		 location(0,0,0) {}


	template <class PUP>
	void pup(PUP &p) {
		int version=1;
		pup(p,version,"version");
		pup(p,mobility,"mobility");

		pup(p,drive,"drive");
		pup(p,act,"act");
		pup(p,sense,"sense");

		pup(p,location,"location");
		pup(p,angle,"angle");
		pup(p,coordinates,"coordinates");
		timestamped::pup(p);
	}

	virtual void loop()
	{
	}
};


}; /* end namespace */

#endif /* end include guard */

