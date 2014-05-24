/**
 This file builds a cyberalaska/robot class to represent a two-wheel drive
 Rover5 chassis, with two ultrasonic distance sensors on the front.

 The corresponding firmware is cyberalaska/rovoduino/vision_neato_driver.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2014-02-21 (Public Domain)
*/
#include "../cyberalaska/porthread.h"

#include "../cyberalaska/hindenburg.hpp"

#include "../msl/2d.hpp"


using namespace cyberalaska;

class hindenburg_robot : public robot {
public:
	hindenburg uav;

	hindenburg_robot(const metadata_general &meta,const std::string &serial_port)
		:robot(meta),uav(serial_port,115200)
	{
		uav.connect();

		const int P=0;
		const int I=1;
		const int D=2;

		uav.leveler.gains[P]=3;
		uav.leveler.gains[I]=0.5;
		uav.leveler.gains[D]=200;

		uav.leveler.limit=100;
		uav.leveler.target=30;
		uav.leveler.smoothing=0.3;
		uav.altitude_limit=60;
	}

	void loop()
	{
		uint16_t move_speed=150;
		uint16_t pitch=1500;
		uint16_t roll=1500;
		uint16_t yaw=1500;

		if(msl::input_check_pressed('t')||msl::input_check_pressed('T'))
			uav.arm();

		if(msl::input_check(' '))
		{
			uav.disarm();
			uav.alt_hold=false;
		}

		if(msl::input_check('w')||msl::input_check('W'))
			pitch+=move_speed;
		if(msl::input_check('s')||msl::input_check('S'))
			pitch-=move_speed;
		if(msl::input_check('a')||msl::input_check('A'))
			roll-=move_speed;
		if(msl::input_check('d')||msl::input_check('D'))
			roll+=move_speed;
		if(msl::input_check('q')||msl::input_check('Q'))
			yaw-=move_speed;
		if(msl::input_check('e')||msl::input_check('E'))
			yaw+=move_speed;
		if(msl::input_check(kb_up))
			uav.throttle+=2;
		if(msl::input_check(kb_down))
			uav.throttle-=2;

		if(msl::input_check_pressed(kb_enter))
			uav.alt_hold=!uav.alt_hold;

		uav.maneuver(pitch,roll,yaw);

		uav.update();
	}
};

void hindenburg_comm_thread(void *hindenburgptr) {
	//while(true)
		//((hindenburg_robot*)hindenburgptr)->loop();
}

robot *make_hindenburg(const std::string &port) {
	static const metadata_general meta_hindenburg(
		"air robot with quadx configuration", // description
		"uav/hindenburg", // model
		"2014-03 mrm"); // version
	printf("Attempting to contact Hindenburg...\n");
	hindenburg_robot *rptr=new hindenburg_robot(meta_hindenburg,port);
	if (!rptr->uav.good()) { // bad connection
		std::cerr<<"Hindenburg not on serial port '"<<port<<"': "<<rptr->status<<"\n";
		delete rptr;
		return NULL;
	}
	else
	{
		std::cerr<<"Hindenburg found on serial port '"<<port<<"': "<<rptr->status<<"\n";
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

