#include "parrot_simulation.hpp"

#include <msl/2d_util.hpp>

parrot_simulation::parrot_simulation():flying(false),emergency(false),
	low_battery(false),bad_motor(false),battery(0.0),x(0),y(0),dir(0),
	prop_rotation(0),prop_rotation_speed(0),
	prop_rotation_speed_inc(200),prop_rotation_speed_max(60)
{}

void parrot_simulation::loop(const double dt)
{
	//Prop Animation
	prop_rotation+=prop_rotation_speed;

	if(flying)
	{
		if(prop_rotation_speed<=0)
			prop_rotation_speed=1;

		prop_rotation_speed+=prop_rotation_speed_inc*dt;

		if(prop_rotation_speed>prop_rotation_speed_max)
			prop_rotation_speed=prop_rotation_speed_max;
	}
	else
	{
		if(prop_rotation_speed>0)
			prop_rotation_speed-=prop_rotation_speed_inc*dt;
		else
			prop_rotation_speed=0;
	}
}

void parrot_simulation::draw(const msl::sprite& body,const msl::sprite& prop,
	const msl::sprite& batt,const msl::sprite& motor,
	const msl::sprite& led,const double scale)
{
	//LED Color
	msl::color led_color_front(0,1,0,1);
	msl::color led_color_back(0,1,0,1);

	if(flying)
		led_color_back=msl::color(1,0.3,0,1);

	if(emergency)
	{
		led_color_front=msl::color(1,0.3,0,1);
		led_color_back=msl::color(1,0.3,0,1);
	}


	//Prop Locations
	double len=msl::point_distance(0,0,100,100)*scale;
	double dir_tr=(dir+45)*M_PI/180.0;
	double dir_tl=(dir+135)*M_PI/180.0;
	double dir_bl=(dir+225)*M_PI/180.0;
	double dir_br=(dir+315)*M_PI/180.0;

	//Draw LEDs
	led.draw(x+cos(dir_tl)*len,y+sin(dir_tl)*len,dir-prop_rotation,0,scale,scale,led_color_front);
	led.draw(x+cos(dir_tr)*len,y+sin(dir_tr)*len,dir-prop_rotation,0,scale,scale,led_color_front);
	led.draw(x+cos(dir_br)*len,y+sin(dir_br)*len,dir-prop_rotation,0,scale,scale,led_color_back);
	led.draw(x+cos(dir_bl)*len,y+sin(dir_bl)*len,dir-prop_rotation,0,scale,scale,led_color_back);

	//Draw Parrot
	body.draw(x,y,dir,0,scale,scale);

	//Draw Props
	prop.draw(x+cos(dir_tl)*len,y+sin(dir_tl)*len,dir-prop_rotation+45,0,-scale,scale);
	prop.draw(x+cos(dir_tr)*len,y+sin(dir_tr)*len,dir-prop_rotation-45,0,scale,scale);
	prop.draw(x+cos(dir_br)*len,y+sin(dir_br)*len,dir-prop_rotation-45,0,-scale,scale);
	prop.draw(x+cos(dir_bl)*len,y+sin(dir_bl)*len,dir-prop_rotation+45,0,scale,scale);

	//Draw Battery Meter
	double batt_v=battery/100.0;
	double batt_w=64*scale;
	double batt_h=16*scale;
	double batt_x=x-batt_w/2.0;
	double batt_y=y+batt_h/2.0;
	msl::draw_rectangle(batt_x,batt_y,batt_w,batt_h,true,msl::color(0.4,0.4,0.4,1));
	msl::draw_rectangle(batt_x,batt_y,batt_w*batt_v,batt_h,true,msl::color(1-batt_v,batt_v,0,1));
	msl::draw_rectangle(batt_x,batt_y,batt_w,batt_h,false,msl::color(0,0,0,1));

	//Draw Error Icons
	if(low_battery)
		batt.draw(x,y+batt_h+batt.height()*scale/2.0,0,0,scale,scale);
	if(bad_motor)
		motor.draw(x,y-batt_h-motor.height()*scale/2.0,0,0,scale,scale);
}