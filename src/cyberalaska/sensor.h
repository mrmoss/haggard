/**
Abstract interface to represent data from simple sensors.

A "sensor" is a piece of hardware that returns a value:
	- A switch returns a boolean value, 0.0 or 1.0
	- A thermistor returns a temperature, usually in degrees F or C
	- An ultrasonic sensor returns a distance
	- An infrared emitter/detector returns a reflectance

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-04 (Public Domain)
*/
#ifndef __CYBERALASKA__SENSOR_H
#define __CYBERALASKA__SENSOR_H
#include "../cyberalaska/timestamp.h"
#include "../cyberalaska/vec3.h"

namespace cyberalaska {

/** This class describes a manufactured thing's model number,
    maker, version, etc. 
*/
class metadata_general {
public:
	/** A human-readable description of what this thing is, 
	    for example "ultrasonic sensor", or "mobile ground robot". */
	std::string description;
	
	/** A short machine-friendly type string for this thing's model number,
	   for example "HC-SR04" (ultrasonic sensor), "DHT22" (temperature sensor). */
	std::string model;
	
	/** A short human-readable string describing the version number.
	    Typically, this is a date and author initials, like "2013-11 osl" 
	*/
	std::string version;
	
	
	/** Set the model used by this sensor. */
	metadata_general(const std::string &description_,
		const std::string &model_,
		const std::string &version_)
		:description(description_), model(model_), version(version_) 
	{}
	
	template <class PUP> 
	void pup(PUP &p) {
		pup(p,description,"description");
		pup(p,model,"model");
		pup(p,version,"version");
	}
};

/** This class describes a sensor's metadata.
*/
class metadata_sensor : public metadata_general {
public:
	/** A short human-readable string units for the sensor value,
	   for example "cm" for distance, "deg F" for temperature, 
	   "ir" for reflectance, or "bool" for a switch. */
	std::string units;
	
	/** This is the angle the sensor can see, in degrees.
	   For example, a typical webcam can see in a view=60 degree cone,
	   while a thermistor senses in all directions so view=360.
	*/
	double view;
	
	/** Set the metadata values for this sensor. */
	metadata_sensor(const std::string &description_,
		const std::string &model_,
		const std::string &version_,
		const std::string &units_,
		int view_)
		:metadata_general(description_,model_,version_),
		 units(units_), view(view_)
	{}
	
	template <class PUP> 
	void pup(PUP &p) {
		metadata_general::pup(p);
		pup(p,units,"units");
		pup(p,view,"view");
	}
};



/** A sensor's attributes are combined into this bit mask. */
typedef unsigned int sensor_flags_t;
enum {
	SENSOR_HAS_VALUE=0x100, ///< We have a valid value.
	SENSOR_HAS_LOCATION=0x200, ///< We have a location (located_sensor)
	SENSOR_HAS_DIRECTION=0x400, ///< We have a direction (directed_sensor)
	/// other attributes will go here.
};

/**
  Sensor data interface.  Only the owner of the sensor should
  update the values here; all others should treat them as read-only.
  (This open interface is designed to be easy to translate into JavaScript &c.)
*/
class sensor_t : public timestamped {
public:
	/** This is the sensor's metadata: units, version, etc.
	    It's a const reference because it's shared between all sensors.
	*/
	const metadata_sensor &metadata;

	/** A bitwise mask of the current sensor flags. 
	    These indicate which fields below are valid. */
	sensor_flags_t flags;
	
	
	/** Contains the last-known value of this sensor. */
	double value;
	/** Update the last-known value of this sensor. */
	void set_value(double v) {
		value=v;
		flags|=SENSOR_HAS_VALUE;
		update_timestamp();
	}
	/** Mark our sensor as not having a value (e.g., bad read) */
	void clear_value() {
		value=0.0;
		last_update=0.0;
		flags&=~SENSOR_HAS_VALUE;
	}
	
	/** The last-known robot-coordinates 3D position corresponding to
	    the last report of this sensor. 0,0,0 means unknown. */
	vec3 location;
	/** Set the location of this sensor. */
	void set_location(const vec3 &v) {
		location=v;
		flags|=SENSOR_HAS_LOCATION;
	}
	
	
	/** The last-known robot-coordinates 3D direction corresponding to
	   the last report of this sensor. 0,0,0 means unknown. */
	vec3 direction;
	/** Set the direction of this sensor. */
	void set_direction(const vec3 &v) {
		direction=v;
		flags|=SENSOR_HAS_DIRECTION;
	}
	
	sensor_t(const metadata_sensor &metadata_) 
		:metadata(metadata_), flags(0),
		value(0.0), location(0,0,0),direction(0,0,0)
		{}
	
	template <class PUP> 
	void pup(PUP &p) {
		pup(p,flags,"flags");
		if (flags&SENSOR_HAS_VALUE) pup(p,value,"value");
		if (flags&SENSOR_HAS_LOCATION) pup(p,location,"location");
		if (flags&SENSOR_HAS_DIRECTION) pup(p,direction,"direction");
		timestamped::pup(p);
	}
};



}; /* end namespace */

#endif /* end include guard */

