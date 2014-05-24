/**
 Student-written UAV client code.

 Orion Lawlor, lawlor@alaska.edu, 2014-05-09 (Public Domain)
*/
#include "cyberalaska/uav_field_drawing.h"
#include "cyberalaska/uav_control.h"
#include "cyberalaska/uav_control_JSON.h"
#include "osl/webservice.h"
#include "msl/time_util.hpp"
#include <iostream>
#include <ctype.h>


/************************ MSL 2D UI code **************************
*/
bool simulator_mode=true;

int sim_seed_ID=time(0)%10;
AK_uav_simulator sim(sim_seed_ID);

int main(int argc,char *argv[]) {
	control_output.state="ready";
	msl::start_2d("UAV Control & Mapping",window_size,window_size);
	return 0;
}

void setup() {
}

static double total_dt=0.0;
void loop(const double dt) {
	total_dt+=dt;
}



void draw() {
	vec2 m=draw_field_setup();
	sim.sensors.mouse_x=m.x;
	sim.sensors.mouse_y=m.y;
	
	if (simulator_mode) // reality, dimly
		draw_field(sim,0.2); 
	draw_field(control_output,1.0); // opaque
	
	// Run simulator
	if (simulator_mode) {
		sim.step(control_output.uav,total_dt); total_dt=0.0;
	}

	// Run user's control code
	AK_uav_control(sim.sensors);
	
	draw_state(control_output.state,1.0);

	draw_UAV(control_output.uav,vec2(sim.sensors.x,sim.sensors.y));
}










std::string url_escape(const std::string &src) {
	std::string dest="";
	for (unsigned int i=0;i<src.size();i++) {
		unsigned char c=src[i];

// Real URL escapes:
		if (isalpha(c) || isdigit(c)) dest+=c;
		else {
			char buf[100];
			snprintf(buf,sizeof(buf),"%02X",(int)c);
			dest+="%";
			dest+=buf;
		}
/*
// MSL workaround
		if (c=='\n' || c==' ') { // skip character
		}
		else dest+=c;
*/
	}
	// std::cout<<"URL escaped to '"<<dest<<"'\n";
	return dest;
}


/********** Server Communication ***********
  By default, we run in local-only simulator mode.
*/

/* We basically want to ignore random periodic network errors.
   We do this by converting thread abort into a throw here.
*/
int skt_error_count=0;
int throw_on_skt_abort(int errCode,const char *msg)
{
	if (skt_error_count++>20) exit(1);
	throw std::runtime_error(msg);
}

/*
  serverMode:
     'S': simulate only (default)
     'L': talk with local server
     'W': talk with web server
*/
void AK_uav_server(char serverMode)
{
	if (serverMode=='S') { simulator_mode=true; return; } // nothing to do
	simulator_mode=false;

	osl::network_progress p;
	std::string host="localhost";
	int port=8080;
	if (serverMode=='W') host="powerwall5.cs.uaf.edu";

	msl::nsleep(20000000);

	skt_set_abort(throw_on_skt_abort);
	try {
		osl::http_connection net(host,p,port);

		// Send piloting command to server
		std::string req_prefix="/uav/0/pilot?cmd=";
		std::string req_suffix=url_escape(JSON_from_AK_uav_field(control_output));
		net.send_get(req_prefix+req_suffix);
		std::string response=net.receive();

		// Get sensor data back from the server.
		//  Save local stuff like mouse position (what's it doing in there anyway?)
		AK_uav_control_sensors bak=sim.sensors;
		sim.sensors=AK_uav_control_sensors_from_JSON(response);

		if (sim.sensors.state!="mission") // sanitize sensor values
			for (int dir=0;dir<4;dir++)
				sim.sensors.hiker[dir]=sim.sensors.obstacle[dir]=1000.0;

		sim.sensors.mouse_x=bak.mouse_x;
		sim.sensors.mouse_y=bak.mouse_y;

		if (control_output.state=="mission" && sim.sensors.state=="ready") 
		{ /* do not overwrite state */ }
		else {
			control_output.state=sim.sensors.state; // mostly mirror server's state
		}

		if (control_output.state=="setup") { // zero out any stored locations
			control_output.hikers=control_output.obstacles=std::vector<vec2>();
		}
		skt_error_count=0; // it worked!
	}
	catch (std::exception &e) {
		std::cout<<"Network problem: "<<e.what()<<"\n";
	}

}





