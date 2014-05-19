//Haggard Source
//	Created By:		Mike Moss and Ann Tupek
//	Modified On:	05/18/2014

//2D Header
#include <msl/2d.hpp>

//Falconer Header
#include <falconer/falconer.hpp>

//IO Stream Header
#include <iostream>

//Parrot Simulation Header
#include "parrot_simulation.hpp"

//Global Variables
ardrone a;
bool auto_pilot=false;
parrot_simulation parrot_sim;

//Main
int main()
{
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
	parrot_sim.battery=a.battery_percent();
	parrot_sim.loop(dt);

	//Maneuver Parrot
	a.manuever(altitude,pitch,roll,yaw);
}

//Draw (Happens as fast as possible.)
void draw()
{
	//Load Sprites
	static msl::sprite spr_parrot("images/parrot.png");
	static msl::sprite spr_prop("images/prop_ccw.png");
	static msl::sprite spr_led("images/led.png");

	//Draw Parrot Simulation
	parrot_sim.draw(spr_parrot,spr_prop,spr_led,0,0,0.5);
}