//Haggard Source
//	Created By:		Mike Moss and Ann Tupek
//	Modified On:	05/19/2014

//2D Header
#include <msl/2d.hpp>

//Bullseye Keeper Header
#include <cyberalaska/bullseye_keeper.hpp>

//Falconer Header
#include <falconer/falconer.hpp>

//IO Stream Header
#include <iostream>

//Parrot Simulation Header
#include "parrot_simulation.hpp"

//Vector Header
#include <vector>

//Global Variables
ardrone a;
bool auto_pilot=false;
parrot_simulation parrot_sim;
bullseye_keeper* eye;

//Main
int main(int argc,char* argv[])
{
	//Commandline Arguments Vector
	std::vector<std::string> command_line_args;

	//Convert Char* to Strings
	for(int ii=1;ii<argc;++ii)
		command_line_args.push_back(argv[ii]);

	//Parse Command Line Arguments
	int camera=0;
	std::string serial_port="/dev/ttyUSB0";
	unsigned int serial_baud=57600;

	for(unsigned int ii=0;ii<command_line_args.size();++ii)
	{
		if(msl::starts_with(command_line_args[ii],"--cam")&&ii+1<command_line_args.size())
		{
			camera=msl::to_int(command_line_args[ii+1]);
			++ii;
		}
		else if(msl::starts_with(command_line_args[ii],"--serial")&&ii+1<command_line_args.size())
		{
			serial_port=command_line_args[ii+1];
			++ii;
		}
		else if(msl::starts_with(command_line_args[ii],"--baud")&&ii+1<command_line_args.size())
		{
			serial_baud=msl::to_int(command_line_args[ii+1]);
			++ii;
		}
		else
		{
			std::cout<<"Unrecognized command line argument "<<command_line_args[ii]<<"!\n";
			exit(1);
		}
	}

	//Setup Camera
	eye=new bullseye_keeper(camera,640,480);

	//Start MSL 2D
	return msl::start_2d("Haggard",640,480);
}

//Setup (Happens once.)
void setup()
{
	//Load Text
	msl::set_text_font("src/msl/verdana.ttf");
	msl::set_text_size(12);

	//Connect Parrot
	if(a.connect())
	{
		//Debug Output
		std::cout<<":)"<<std::endl;

		//Setup Parrot
		a.set_level();
		a.set_outdoor_mode(false);
		a.set_using_shell(false);
		a.set_using_brushless_motors(true);
		a.set_min_altitude(50);
		a.set_max_altitude(1000);
	}

	//Bad Connection
	else
	{
		std::cout<<":("<<std::endl;
		exit(0);
	}
}

//Loop (Happens as fast as possible.)
void loop(const double dt)
{
	//Update Parrot Navigation Data
	a.navdata_update();

	//Emergency Mode
	if(msl::input_check_pressed(kb_r))
		a.emergency_mode_toggle();

	//Takeoff
	if(msl::input_check_pressed(kb_t))
		a.takeoff();

	//Land
	if(msl::input_check_pressed(kb_space))
		a.land();

	//Auto Pilot Toggle
	if(msl::input_check_pressed(kb_enter))
		auto_pilot=!auto_pilot;

	//Manuevering Variables
	float speed=0.8;
	float pitch=0;
	float roll=0;
	float altitude=0;
	float yaw=0;

	//Auto Pilot
	if(auto_pilot)
	{
	}

	//Manual Mode
	else
	{
		//Lateral Movement
		if(msl::input_check(kb_w))
			pitch=-speed;
		if(msl::input_check(kb_s))
			pitch=speed;
		if(msl::input_check(kb_a))
			roll=-speed;
		if(msl::input_check(kb_d))
			roll=speed;
		if(msl::input_check(kb_up))
			altitude=speed;
		if(msl::input_check(kb_down))
			altitude=-speed;

		//Rotate
		if(msl::input_check(kb_q))
			yaw=-speed;
		if(msl::input_check(kb_e))
			yaw=speed;
	}

	//Update Parrot Simulation
	parrot_sim.flying=a.flying();
	parrot_sim.emergency=a.emergency_mode();
	parrot_sim.low_battery=a.low_battery();
	parrot_sim.bad_motor=!a.motors_good();
	parrot_sim.battery=a.battery_percent();
	parrot_sim.loop(dt);

	//Maneuver Parrot
	a.manuever(altitude,pitch,roll,yaw);

	//Camera Update
	std::vector<vec3> bulls=eye->update();

	if(bulls.size()>0)
	{
		parrot_sim.x=bulls[0].x;
		parrot_sim.y=bulls[0].y;
		parrot_sim.dir=bulls[0].z*180.0/M_PI-90;
	}
}

//Draw (Happens as fast as possible.)
void draw()
{
	//Load Sprites
	static msl::sprite spr_parrot("images/parrot.png");
	static msl::sprite spr_prop("images/prop_ccw.png");
	static msl::sprite spr_led("images/led.png");
	static msl::sprite spr_low_battery("images/battery.png");
	static msl::sprite spr_bad_motor("images/engine.png");

	//Move Parrot Sprite Origin to Center of Parrot
	spr_parrot.set_origin(0,-24);

	//Draw Parrot Simulation
	parrot_sim.draw(spr_parrot,spr_prop,spr_low_battery,spr_bad_motor,spr_led,0.25);

	double two_feet_in_cm=60.96;

	for(double xx=0;xx<=5;++xx)
		for(double yy=0;yy<=5;++yy)
			msl::draw_rectangle_center(
				-two_feet_in_cm*5/2.0+xx*two_feet_in_cm,
				-two_feet_in_cm*5/2.0+yy*two_feet_in_cm,
				two_feet_in_cm,two_feet_in_cm,false,msl::color(0,1,0,1));
}