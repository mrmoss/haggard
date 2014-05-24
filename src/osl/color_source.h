/**
 ColorSource: fill in a pixel background.
 Often used for debugging and development.

Orion Sky Lawlor, olawlor@acm.org, 2004/2/20
*/
#ifndef __OSL_COLORSOURCE_H
#define __OSL_COLORSOURCE_H

#include "osl/graphics.h"

namespace osl { namespace gui {

/**
 Returns a color at each pixel based on the values of 
 a set of parameter points.
*/
class ColorSource {
public:
	virtual ~ColorSource();
	
	/// Initialize for this set of offscreen points and this display size.
	///  This routine is called before getColor or draw.
	virtual void setup(int nPoints,const osl::Vector2d *points,int wid,int ht);
	
	/// Return the color of this offscreen point.
	///  This is called for every pixel of the window.
	virtual osl::graphics2d::Color getColor(const osl::Vector2d &loc) =0;
	
	/// Draw any overlay on top of the image.
	///  This will be overlain with the pixel values from getColor.
	virtual void draw(osl::graphics2d::Graphics &grafport,const osl::Matrix2d &onFmOff);
	
	/// Display this source in a GUI window until the 
	/// window is closed.
	void run(int nPoints,const osl::Vector2d *points);
};

}; }; 

#endif
