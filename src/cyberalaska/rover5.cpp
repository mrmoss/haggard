/**
 This file builds a cyberalaska/robot class to represent a two-wheel drive
 Rover5 chassis, with two ultrasonic distance sensors on the front.

 The corresponding firmware is cyberalaska/rovoduino/vision_neato_driver.

 Dr. Orion Lawlor, lawlor@alaska.edu, 2014-02-21 (Public Domain)
*/
#include "../cyberalaska/porthread.h"

/* Onboard sensor package */
#define SENSORS_ENCODERS 0   /* onboard wheel rotation encoders */
#define SENSORS_ULTRASONIC 0 /* ultrasonic distance sensors */
#define SENSORS_NEATO 1   /* Neato XV-11 Laser Distance Sensor 360 scanner */

#define SERIAL_USE_MSL 1    /* 1 for MSL serial, 0 for OSL serial */

#if SERIAL_USE_MSL
#  include "../msl/serial.hpp"
#  include "../msl/time_util.hpp"
#else
#  include "../osl/serial.h"
#  include "../osl/serial.cpp"
#endif
using namespace cyberalaska;

#include "../cyberalaska/serial_packet.h"



#if SENSORS_NEATO
#include "../cyberalaska/neato_sensor.h"
#endif





class rover5 : public robot {
public:
/* FIXME: create a "class comm" that represents a binary communication channel.
   Implementations would be MSL serial, OSL serial, binary sockets, HTTP-wrapped sockets,
   binary files, cloned telemetry capture, verbose data-dump wrapper, etc.
 */
 	int serial_timeout;
#if SERIAL_USE_MSL /* MSL serial */
	msl::serial comm;
	A_packet_formatter<rover5> pkt;

	int available() { return comm.available(); }
	void write(unsigned char command) {
		comm.write(&command,1);
	}
	virtual void write(const unsigned char *data,int length) {
		//printf("PC sending Rover5 %d bytes of data: 0x%02x ... 0x%02x\n",
			//length, (int)data[0],(int)data[length-1]);
		comm.write(data,length);
	}
	int read() {
		unsigned char ret;
		unsigned long timer=msl::millis()+serial_timeout;
		 while (comm.available()<1&&msl::millis()<timer)
			porthread_yield(1);

		if (comm.read(&ret,1)!=1) return -1;
		else return ret;
	}

	bool comm_good;

	rover5(const metadata_general &meta,const std::string &port)
		:robot(meta), serial_timeout(1000), comm(port,57600), pkt(*this), comm_good(false)
	{
		comm.connect();

		if (!comm.good()) status="error opening port";
		else ping(); // check if we've got connectivity
	}
#else /* OSL serial (seems to run just the same) */
	SerialPort comm;
	A_packet_formatter<rover5> pkt;

	int available() { return comm.available(); }
	void write(unsigned char command) {
		comm.write(command);
	}
	virtual void write(const unsigned char *data,int length) {
		for (int i=0;i<length;i++) write(data[i]);
	}
	int read() {
		unsigned char ret;
		if (comm.Input_wait(serial_timeout)<=0) return -1;
		comm.Read(&ret,1);
		return ret;
	}

	bool comm_good;

	rover5(const metadata_general &meta,const std::string &port)
		:robot(meta), serial_timeout(1000), pkt(*this), comm_good(false)
	{
		comm.Open(port);
		comm.Set_baud(57600);

		if (!comm.Is_open()) status="error opening port";
		else ping(); // check if we've got connectivity
	}
#endif
	/// Read serial data until we have the next entire packet,
	///  and handle the packet
	void read_until_packet(const char *from_code) {
		double start_time=cyberalaska::time();
		A_packet p;
		p.valid=0;
		while (start_time+0.7>cyberalaska::time() ) {
			int r=pkt.read_packet(p);
			if (r==-1) continue; // more data coming--keep looping
			if (r==0) { porthread_yield(0); } // waiting for data
			else { /* r==1: have a whole packet now */
				handle_packet(p,from_code);
				return;
			}
		}
		// getting here means a timeout occurred--reset receiver
		pkt.reset();
		//printf("Rover5 PC-side timeout from %s\n",from_code);
	}

	/// Handle this serial data packet arriving from the rover
	void handle_packet(const A_packet &p,const char *from_code) {
		//printf("Rover5 sent PC %s serial packet: cmd=0x%x len=%d  at %s\n",
			//p.valid?"valid":"corrupt",p.command,p.length,from_code);
		if (!p.valid) { /* corrupted serial packet */
			//printf("Rover5 serial checksum error on packet type 0x%x (%d bytes)\n",
				//p.command,p.length);
		}
		else if (p.command==0x0) { /* ping response */
			status="connected";
			comm_good=true;
		}
		else if (p.command==0x3) { /* motor response */
			const char *err=handle_motor_packet(p);
			if (err!=NULL) printf("Rover5 motor err: %s\n",err);
		}
#if SENSORS_NEATO
		else if (p.command==0xD) { /* depth data */
			const char *err=handle_neato_packet(p);
			if (err!=NULL) printf("Rover5 neato err: %s\n",err);
		}
#endif
		else
		{ /* some unrecognized command */
			//printf("Rover5 %s: unexpected packet response code 0x%x\n",
				//from_code,(int)p.command);
			for (int i=0;i<p.length;i++)
				//printf("    packet data[%d]='%c' or 0x%x\n",i,(char)p.data[i],(int)p.data[i]);
			fflush(stdout);
		}
	}


	// Do a basic ping test, to make sure the robot is still synced.
	//   Return code is essentially stored in "comm_good".
	void ping(void) {
		pkt.write_packet(0x0,0,0); // ping request
		read_until_packet("ping");
	}

	// Scale a motor command for this power level (-1.0 is full reverse, 0.0 is stop, 1.0 is full forward).
	signed char motor_power_scale(float power) {
		int dir=1; // assume forwards movement
		if (power<0) {dir=0; power=-power;}
		if (power<0.15) power=0.0; // just causes annoying whine
		if (power>1.0) power=1.0; // limit max power
		if (dir==0) power=-power;

		int ipower=(int)(power*0x7f); // -0x7f to +0x7f range

		return (signed char)ipower;
	}

	double motor_timestamp;
	void request_motors(void)
	{
		signed char speeds[2];
		for (int side=0;side<2;side++) {
			if (drive[side].is_current())
				speeds[side]=motor_power_scale(drive[side].read());
			else
				speeds[side]=0;
		}
		pkt.write_packet(0x3,sizeof(speeds),speeds);
		motor_timestamp=cyberalaska::time();
	}

	const char *handle_motor_packet(const A_packet &p) {
		if (p.command!=0x3) {
			return "Unexpected packet type!";
		}
		unsigned short e[2];
		if (!p.get(e)) return "Unexpected packet length!";

		// FIXME: write into robot sensor or drive objects (somewhere!)
		//printf("rover5encoders	%d	%d\n",e[0],e[1]);
		//printf("Motor roundtrip: %.1f ms\n",1000.0*(cyberalaska::time()-motor_timestamp));

		return NULL; // no error, OK
	}

#if SENSORS_NEATO
	neato_sensor_t *neato;
	int last_batch_index;
	double neato_timestamp;

	// Request a Neato distance report from the Arduino
	//   Returns NULL on success; or a human-readable error message.
	void request_neato(void) {
		pkt.write_packet(0xD,0,0); /* sensor code for neato Depth request */
		neato_timestamp=cyberalaska::time();
	}

	const char *handle_neato_packet(const A_packet &p) {
		if (p.command!=0xD) {
			return "Unexpected response to 0xD depth command";
		}
		NeatoLDSbatch b;
		if (!p.get(b)) return "Unexpected packet length returned";

		// sanity check returned batch
		if (b.index<0 || b.index>=360) return "spin index invalid (spinning up?)";
		if (b.index==last_batch_index) return "duplicate batch index";
		last_batch_index=b.index;
		if (b.errors>0) return "serial errors detected on Arduino side";

		// write to our directions
		for (int i=0;i<NeatoLDSbatch::size;i++) {
			int d=i+b.index;
			if (d>=0 && d<neato_sensor_t::ndir) neato->dir[d]=b.dir[i];
		}
		neato->update_timestamp(neato_timestamp); // fixme: per-direction timestamps?
		//printf("Neato roundtrip: %.1f ms\n",1000.0*(cyberalaska::time()-neato_timestamp));

		return NULL; // OK
	}
#endif

	/* Run this rover's communication thread, forever. */
	void comm_thread() {
		int comm_count=0;
		while (1) {
			if ((comm_count%100)==0)
			{ /* periodically resynchronize */
				ping();
			}

			// Command left and right motors
			request_motors();
			read_until_packet("motors");

#if SENSORS_ULTRASONIC
			// Read ultrasonic sensors
			int n_sense=2;
			for (int s=0;s<n_sense;s++) {
				write(0x90+s);
			}
			// You could merge these two loops, but
			//   it's faster to queue up two requests (ww-rr is lower latency than w-r w-r)
			for (int s=0;s<n_sense;s++) {
				int ret=read();
				if (ret>=0)
				{
					sense[s].set_value(ret);
				}
			}
#endif

#if SENSORS_NEATO
			request_neato();
			read_until_packet("neato");
#endif

			comm_count++;
		}
	}
};

void rover5_comm_thread(void *rover5ptr) {
	((rover5 *)rover5ptr)->comm_thread();
}

robot *make_rover5(const std::string &port) {
	static const metadata_general meta_rover(
		"ground robot with two tank tracks", // description
		"rover5/rovoduino", // model
		"2014-03 osl"); // version
	printf("Attempting to contact Rover5...\n");
	rover5 *rptr=new rover5(meta_rover,port);
	if (!rptr->comm_good) { // bad connection
		std::cerr<<"Rover5 not on serial port '"<<port<<"': "<<rptr->status<<"\n";
		delete rptr;
		return NULL;
	}
	robot &r=*rptr;

// Create left and right motors
	r.mobility="tank";
	for (int motor=0;motor<2;motor++) {
		actuator_t *drive=new actuator_t;
		drive->type="torque";
		r.drive.push(drive);
	}

#if SENSORS_ULTRASONIC
// Ultrasonic sensors on left [0] and right [1] sides:
	static const metadata_sensor meta_ultrasonic(
		"ultrasonic distance sensor", // description
		"HC-SR04", // model
		"2013-11 osl", // version
		"cm", // range is reported in centimeters
		40 // field of view, degrees
	);
	for (double side=1.0;side>=-1.0;side-=2.0) // for side in {+1,-1}
	{
		sensor_t *ultrasonic=new sensor_t(meta_ultrasonic);
		double angle=20.0*M_PI/180.0; // degrees pointing deviation from robot centerline
		double c=cos(angle), s=sin(angle);
		double rad=11.0; // centimeters from robot's center to sensor center
		ultrasonic->set_location(vec3(rad*c,rad*s*side,0));
		ultrasonic->set_direction(vec3(c,s*side,0)); // sensor is looking this direction
		r.sense.push(ultrasonic);
	}
#endif

#if SENSORS_NEATO
	static const metadata_sensor meta_neato(
		"laser distance sensor", // description
		"XV-11", // model
		"2014-03 osl", // version
		"mm", // range is reported in millimeters
		360 // field of view, degrees
	);
	neato_sensor_t *neato=new neato_sensor_t(meta_neato);
	double angle=0.0*M_PI/180.0; // degrees pointing deviation from robot centerline
	double c=cos(angle), s=sin(angle);
	double rad=20.0; // centimeters from robot's center to sensor center
	neato->set_location(vec3(rad,0,0));
	neato->set_direction(vec3(c,s,0)); // sensor is looking this direction
	r.sense.push(neato);
	rptr->neato=neato;
#endif

	// Build a communication thread to talk to the rover.
	porthread_detach(porthread_create(rover5_comm_thread,rptr));

	return rptr;
}

