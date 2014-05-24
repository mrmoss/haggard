/**
Very simple GUI wrapper: show bitmaps on the screen at given locations.
	Implementations could be OpenGL for local display, 
	or browser-based (JPEG transport, canvas or WebGL compositing).

Dr. Orion Sky Lawlor, lawlor@alaska.edu, 2013-11-08 (Public Domain)
*/
#ifndef __CYBERALASKA_GUI_H
#define __CYBERALASKA_GUI_H

#include <GL/glew.h>  //OpenGL (I'd be happy with GL/gl.h, but msl uses GLEW)

#include "../msl/2d.hpp" /* basic OpenGL */
#include "../msl/sprite.hpp" /* textures */
#include "../cyberalaska/vec3.h"
#include "../cyberalaska/coords.h"
#include "../rasterCV/raster.h" /* for image templates */




namespace cyberalaska {


/* This function does all the OpenGL setup work. */
void gui_setup(void);

/* Draw this rasterCV IMAGE template (raster_store, raster_image, etc) with OpenGL */
template <class IMAGE>
void draw_image(IMAGE &img,const coords &c) 
{
	//Enable Texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0); // default texture 
	int filter=GL_LINEAR;
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,filter);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,filter);

	rasterCV::glTexImage2D(img); // <- magic template override in raster.h

	// Splat out the image
	glColor4d(1,1,1,1);
	glBegin(GL_QUADS);
		glTexCoord2f(0,1);
		glVertex3fv(c.world_from_texture(vec3(0,1,0)));
		
		glTexCoord2f(1,1);
		glVertex3fv(c.world_from_texture(vec3(1,1,0)));
		
		glTexCoord2f(1,0);
		glVertex3fv(c.world_from_texture(vec3(1,0,0)));
		
		glTexCoord2f(0,0);
		glVertex3fv(c.world_from_texture(vec3(0,0,0)));
	glEnd();

	//Disable Texture
	glDisable(GL_TEXTURE_2D);
}



}; /* end namespace */

#endif /* defined (this header) */


