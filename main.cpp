//Haggard Source
//	Created By:		Mike Moss
//	Modified On:	05/18/2014

//Required Libraries:
//	gl
//	glew
//	glu
//	glut/freeglut
//	ftgl

#include <iostream>
#include <msl/socket.hpp>
#include <msl/string_util.hpp>
#include <msl/2d.hpp>
#include <string>
#include <falconer/falconer.hpp>

ardrone a;

int main()
{
	return msl::start_2d("ardrone",640,360);
}

void setup()
{
	msl::set_text_font("msl/verdana.ttf");
	msl::set_text_size(12);

	if(a.connect())
	{
		std::cout<<":)"<<std::endl;

		a.set_level();
		a.set_outdoor_mode(false);
		a.set_using_shell(false);
		a.set_using_brushless_motors(true);
		a.set_min_altitude(50);
		a.set_max_altitude(1000);
	}
	else
	{
		std::cout<<":("<<std::endl;
		exit(0);
	}
}

void loop(const double dt)
{
	a.navdata_update();

	float speed=0.8;
	float pitch=0;
	float roll=0;
	float altitude=0;
	float yaw=0;

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

void draw()
{
	std::string data;
	data+="Pitch:\t\t"+msl::to_string(a.pitch())+"\n";
	data+="Roll:\t\t"+msl::to_string(a.roll())+"\n";
	data+="Yaw:\t\t"+msl::to_string(a.yaw())+"\n";
	data+="Altitude:\t"+msl::to_string(a.altitude())+"\n";

	std::string data2;
	data2+="Battery:\t"+msl::to_string(a.battery_percent())+"%\n";

	if(a.emergency_mode())
		data2+="Emergency Mode\n";

	if(a.low_battery())
		data2+="Low Battery\n";

	if(!a.motors_good())
		data2+="Bad Motor\n";

	if(a.flying())
		data2+="Flying\n";
	else
		data2+="Landed\n";

	msl::draw_text(-msl::window_width/2.0,msl::window_height/2.0-100,data);
	msl::draw_text(-msl::window_width/2.0,msl::window_height/2.0-125,data2);
}
