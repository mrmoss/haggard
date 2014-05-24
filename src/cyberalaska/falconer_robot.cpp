/**
 This file builds a cyberalaska/robot class to represent a two-wheel drive
 Rover5 chassis, with two ultrasonic distance sensors on the front.

 The corresponding firmware is cyberalaska/rovoduino/vision_neato_driver.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2014-02-21 (Public Domain)
*/
#include "../cyberalaska/porthread.h"

#include "../falconer/falconer.hpp"

#include "../msl/2d.hpp"


using namespace cyberalaska;

class falconer_robot : public robot {
public:
	ardrone a;

	falconer_robot(const metadata_general &meta)
		:robot(meta)
	{
		a.connect();
	}

	void loop()
	{
		a.navdata_update();

		float speed=0.8;
		float pitch=0;
		float roll=0;
		float altitude=0;
		float yaw=0;

		if(msl::input_check(kb_escape))
			exit(0);

		if(msl::input_check_pressed(kb_r))
			a.emergency_mode_toggle();

		if(msl::input_check_pressed(kb_1))
			a.set_video_feed_front();

		if(msl::input_check_pressed(kb_2))
			a.set_video_feed_bottom();

		if(msl::input_check(kb_w))
			pitch=-speed;

		if(msl::input_check(kb_s))
			pitch=speed;

		if(msl::input_check(kb_a))
			roll=-speed;

		if(msl::input_check(kb_d))
			roll=speed;

		if(msl::input_check(kb_q))
			yaw=-speed;

		if(msl::input_check(kb_e))
			yaw=speed;

		if(msl::input_check(kb_up))
			altitude=speed;

		if(msl::input_check(kb_down))
			altitude=-speed;

		if(msl::input_check_pressed(kb_t))
			a.takeoff();

		if(msl::input_check_pressed(kb_space))
			a.land();

		a.manuever(altitude,pitch,roll,yaw);
	}
};

void falconer_comm_thread(void *falconerptr) {
	//while(true)
		//((falconer_robot*)falconerptr)->loop();
}

robot *make_falconer() {
	static const metadata_general meta_falconer(
		"air robot with quadx configuration", // description
		"uav/falconer", // model
		"2014-04 mrm"); // version
	printf("Attempting to contact falcon...\n");
	falconer_robot *rptr=new falconer_robot(meta_falconer);
	if (!rptr->a.good()) { // bad connection
		std::cerr<<"Falcon not on wifi "<<rptr->status<<"\n";
		delete rptr;
		return NULL;
	}
	else
	{
		std::cerr<<"Falcon found on serial port "<<rptr->status<<"\n";
	}

	robot &r=*rptr;

// Create left and right motors
	r.mobility="heli";
	for (int motor=0;motor<4;motor++) {
		actuator_t *drive=new actuator_t;
		drive->type="speed";
		r.drive.push(drive);
	}

// Ultrasonic sensors on left [0] and right [1] sides:
	static const metadata_sensor meta_ultrasonic(
		"ultrasonic distance sensor", // description
		"HC-SR04", // model
		"2013-11 osl", // version
		"cm", // range is reported in centimeters
		40 // field of view, degrees
	);
	sensor_t *ultrasonic=new sensor_t(meta_ultrasonic);
	ultrasonic->set_location(vec3(4,0,0));
	ultrasonic->set_direction(vec3(0,0,-1)); // sensor is looking this direction
	r.sense.push(ultrasonic);

	// Build a communication thread to talk to the hindenburg.
	porthread_detach(porthread_create(hindenburg_comm_thread,rptr));

	return rptr;
}

