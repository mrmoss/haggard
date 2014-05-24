/**
 Utility function to draw field using MSL
*/
#include "msl/2d.hpp"
#include "cyberalaska/uav_field.h"


extern int window_size;
extern double field_size; // in uav_field.cpp


template <class object>
inline void draw_object(const object &obj,msl::sprite &glyph,float alpha,float sz)
{
	glyph.draw(obj.x,obj.y,
			0,0,sz/256,sz/256,
			msl::color(1,1,1,alpha) );
}

inline void draw_list(const std::vector<vec2> &list,msl::sprite &glyph,float alpha,float sz)
{
	for (unsigned int o=0;o<list.size();o++) {
		const vec2 &p=list[o];
		draw_object(p,glyph,alpha,sz);
	}
}

// Set up OpenGL to draw field coordinates
inline vec2 draw_field_setup()
{ 
	// scale OpenGL coordinate system from pixels to real field coords
	glLoadIdentity();
	glTranslatef(-window_size/2.0,-window_size/2.0,0.0);
	float scale=window_size/(field_size+2.0);
	glScalef(scale,scale,1.0f);
	glTranslatef(+1.0f,+1.0f,0.0f);

	// Extract coordinate system conversion
	float mat[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,mat);
	vec2 mat_scale(1.0/mat[0],1.0/mat[5]);
	vec2 mat_offset(mat[12],mat[13]);
	// coordinate-convert mouse to field coords
	vec2 m=vec2(msl::mouse_x,msl::mouse_y)-mat_offset;
	m.x*=mat_scale.x;
	m.y*=mat_scale.y;
	vec2 field_mouse=m;

	// sketch in field outline
	glBegin(GL_LINES);
	glColor4f(1,1,1,1);
	for (int line=0;line<=field_size;line+=2) {
		glVertex2f(line,0);
		glVertex2f(line,field_size);
		glVertex2f(0,line);
		glVertex2f(field_size,line);
	}
	glEnd();
	
	return field_mouse;
}

// Draw the hikers and obstacles in this field
inline void draw_field(const AK_uav_field &field,float alpha)
{
	// Draw solid copy of reported obstacles, (and dim copies of true obstacles)
	static msl::sprite obstacle_image("images/obstacle.png");
	float sz=0.9;
	draw_list(field.obstacles,obstacle_image,alpha,sz);

	// As above, now with hikers
	sz=1.3;
	static msl::sprite hiker_image("images/LawloRex.png");
	draw_list(field.hikers,hiker_image,alpha,sz);
}

// Draw the UAV sprites
inline void draw_UAV(const vec2 &target,const vec2 &detect) 
{
	static msl::sprite uav_image("images/UAV.png");
	float sz=1.5;
	draw_object(target,uav_image,0.3,sz);
	draw_object(detect,uav_image,1.0,sz);
}


// Draw the pilot's state dialog
inline void draw_state(const std::string &state,float alpha=1.0)
{
	// Splat state "dialog"
	static msl::sprite state_setup("images/state_setup.png");
	static msl::sprite state_prep("images/state_prep.png");
	static msl::sprite state_ready("images/state_ready.png");
	static msl::sprite state_land("images/state_land.png");
	static msl::sprite state_done("images/state_done.png");
	msl::sprite *draw_state=0;
	if (state=="setup") draw_state=&state_setup;
	if (state=="prep")  draw_state=&state_prep;
	if (state=="ready") draw_state=&state_ready;
	if (state=="land")  draw_state=&state_land;
	if (state=="done")  draw_state=&state_done;
	if (draw_state!=0) draw_state->draw(field_size/2,field_size/2, 
		0.0,0, 8.0/512,4.0/256, msl::color(1,1,1,alpha));
}

