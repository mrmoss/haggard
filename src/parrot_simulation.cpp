#include "parrot_simulation.hpp"

#include <msl/2d_util.hpp>

parrot_simulation::parrot_simulation():flying(false),emergency(false),battery(0.0),
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

void parrot_simulation::draw(const msl::sprite& body,const msl::sprite& prop,const msl::sprite& led,
	const double x,const double y,const double scale)
{
	//LED Color
	msl::color led_color(0,1,0,1);

	if(emergency)
		led_color=msl::color(1,0.3,0,1);

	//Prop Locations
	double xoff=100*scale;
	double yoff_top=76*scale;
	double yoff_bot=124*scale;

	//Draw LEDs
	led.draw(x-xoff,y+yoff_top,-prop_rotation+10,0,-scale,scale,led_color);
	led.draw(x+xoff,y+yoff_top,prop_rotation+35,0,scale,scale,led_color);
	led.draw(x+xoff,y-yoff_bot,-prop_rotation+90,0,-scale,scale,led_color);
	led.draw(x-xoff,y-yoff_bot,prop_rotation+120,0,scale,scale,led_color);

	//Draw Parrot
	body.draw(x,y,0,0,scale,scale);

	//Draw Props
	prop.draw(x-xoff,y+yoff_top,-prop_rotation+10,0,-scale,scale);
	prop.draw(x+xoff,y+yoff_top,prop_rotation+35,0,scale,scale);
	prop.draw(x+xoff,y-yoff_bot,-prop_rotation+90,0,-scale,scale);
	prop.draw(x-xoff,y-yoff_bot,prop_rotation+120,0,scale,scale);

	//Draw Battery Meter
	double batt_v=battery/100.0;
	double batt_w=64*scale;
	double batt_h=16*scale;
	double batt_x=x-batt_w/2.0;
	double batt_y=y-batt_h/2.0;;
	msl::draw_rectangle(batt_x,batt_y,batt_w,batt_h,true,msl::color(0.4,0.4,0.4,1));
	msl::draw_rectangle(batt_x,batt_y,batt_w*batt_v,batt_h,true,msl::color(1-batt_v,batt_v,0,1));
	msl::draw_rectangle(batt_x,batt_y,batt_w,batt_h,false,msl::color(0,0,0,1));
}