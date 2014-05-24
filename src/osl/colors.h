/*
Orion's Standard Library
Orion Sky Lawlor, 4/7/2002
NAME:		osl/color.h

DESCRIPTION:	C++ Graphics library

More advanced color classes.
*/
#ifndef __OSL_COLORS_H
#define __OSL_COLORS_H

#ifndef __OSL_COLOR_H
#  include "osl/color.h"
#endif

namespace osl { namespace graphics2d {

/*
Hue-Saturation-Brightness, a supposedly more intuitive way of organizing 
colors than RGB.  Used by many color pickers, and the Gimp.
Also known as hue-saturation-value.
*/
class HsbColor {
public:
	float h; //Hue, 0 (red) to 0.3333 (green) to 0.66666 (blue) to 1 (red again)
	float s; //Saturation, from 0 (gray) to 1 (full color)
	float b; //Brightness, from 0 (black) to 1 (full brightness)
	
	HsbColor() {}
	HsbColor(float h_,float s_,float b_)
		:h(h_), s(s_), b(b_) {}
	HsbColor(Color c);
};

/*
Cyan-Magenta-Yellow-blacK
The de-facto standard of the printing world.  There isn't
one unique conversion between CMYK and RGB, so I use a silly
but seemingly standard conversion-- maximize black.

Unlike everything else, when the numbers get bigger, CMYK gets
darker.
*/
class CmykColor {
public:
	float c,m,y,k; //Amounts of *ink*, from 0 to 1
	
	CmykColor() {}
	CmykColor(float c_,float m_,float y_,float k_)
		:c(c_), m(m_), y(y_), k(k_) {}
	CmykColor(Color c);
}; 

/*
br(Y)ghtness-ColorRed-ColorBlue
A bizarre but widespread representation.  Supposedly
the human eye uses something akin to this representation;
so it's used by full-color perceptual formats like JPEG and MPEG.
*/
class YCrCbColor {
public:
	float y; //Brightness (Y'), from 0 to 1
	float Cr; //red-green channel, scaled to -0.5 to 0.5
	float Cb; //blue-green channel, scaled to -0.5 to 0.5
	
	YCrCbColor() {}
	YCrCbColor(float y_,float Cr_,float Cb_)
		:y(y_), Cr(Cr_), Cb(Cb_) {}
	YCrCbColor(Color c);
};


}; }; /*end namespace*/

#endif /* def(thisHeader) */
