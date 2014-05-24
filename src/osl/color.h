/*
Orion's Standard Library
Orion Sky Lawlor, 4/7/2002
NAME:		osl/color.h

DESCRIPTION:	C++ Graphics library

Basic Color classes.
*/
#ifndef __OSL_COLOR_H
#define __OSL_COLOR_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif

namespace osl { namespace graphics2d {

class HsbColor; //Hue-Saturation-Brightness
class CmykColor; //Cyan-Magenta-Yellow-blacK
class YCrCbColor; //br(Y)ghtness-ColorRed-ColorBlue

// This is a floating-Point Color class.
// 0 -> black; 1.0 -> "white"
class Color {
public:
	//Color channels--
	// 0 Red (500-700nm) 650e-9
	// 1 Green (450-620nm) 500e-9
	// 2 Blue (350-500nm) 420e-9

	float r,g,b,a;
	const static Color red,orange,yellow,green,cyan,blue,purple,
		pink,black,white,gray,grey,clear;
	
	//Constructors
	Color() {a=1.0f;}
	Color(const Color &val) {r=val.r;g=val.g;b=val.b;a=val.a;}
	explicit Color(int gray) {r=g=b=(float)gray;a=1.0f;}
	explicit Color(float gray) {r=g=b=gray;a=1.0f;}
	explicit Color(double gray) {r=g=b=(float)gray;a=1.0f;}
	Color(const HsbColor &c);
	Color(const CmykColor &c);
	Color(const YCrCbColor &c);
	Color(float Nr,float Ng,float Nb) {r=Nr,g=Ng,b=Nb,a=1.0f;}

    //Non-premultiplied alpha constructor: does the alpha multiply itself
	Color(float Nr,float Ng,float Nb,float Na) 
	  {r=Nr*Na; g=Ng*Na; b=Nb*Na; a=Na;}

    //Premultiplied alpha constructor: just copy the fields
	class premultiplied { //Lets us distinguish premultiplied from normal constructor
		float value;
	public:
		explicit premultiplied(float v) :value(v) { }
		float val(void) const {return value;}
	};
	Color(float Nr,float Ng,float Nb,const premultiplied &p) 
	  {r=Nr; g=Ng; b=Nb; a=p.val();}
	
	//Default copy constructor & assignment operator
	explicit Color(const byte *p)
		{r=p[0]*(1/255.0f),g=p[1]*(1/255.0f),b=p[2]*(1/255.0f);a=1.0f;}
	
	Color &operator=(int val) {r=g=b=val;a=1.0f;return *this;}
	Color &operator=(float val) {r=g=b=val;a=1.0f;return *this;}
	Color &operator=(double val) {r=g=b=val;a=1.0f;return *this;}
	Color &operator=(const Color &val) {r=val.r;g=val.g;b=val.b;a=val.a; return *this;}
	
	
	//Access as array
	float &operator[](int i) {return (&r)[i];}
	const float &operator[](int i) const {return (&r)[i];}
	
	//Comparison operators
	bool operator==(const Color &c) const 
	  {return r==c.r && g==c.g && b==c.b && a==c.a;}
	bool operator!=(const Color &c) const 
	  {return r!=c.r || g!=c.g || b!=c.b || a!=c.a;}
	
	//Complement this Color: return an opposite color
	Color comp(void) const
		{return Color(1-r,1-g,1-b,premultiplied(1-a));}
	
	//Overlay us by s.  Opacity==0 -> return *this; opacity==1 ->return s
	Color blend(const Color &s) const 
	{
		float usFrac=1.0f-s.a;
		return s+(*this)*usFrac;
	}
	
	//"Premultiply" our alpha channel
	void alpha_premultiply(void) {r*=a;g*=a;b*=a;}
	//"Demultiply" our alpha channel
	void alpha_demultiply(void) {float inv=1.0f/a;r*=inv;g*=inv;b*=inv;}
	//Adjust us and our alpha channel
	void scaleAlpha(float by) {
		r*=by;g*=by;b*=by;a*=by;
	}
	
	Color &clipPos(void) { //Ensure no component exceeds 1
		if (r>1.0f) r=1.0f;
		if (g>1.0f) g=1.0f;
		if (b>1.0f) b=1.0f;
		return *this;
	}
	Color &clipNeg(void) { //Ensure no component is less than 0
		if (r<0.0f) r=0.0f;
		if (g<0.0f) g=0.0f;
		if (b<0.0f) b=0.0f;
		return *this;
	}
	Color &clip(void) {clipPos(); return clipNeg();}
	
	//Apply the given exponent (values > 1 make darks darker)
	void exp(float factor);
	
	//Return this Color's best approximation as greyscale
	// From http://www.inforamp.net/~poynton/ColorFAQ.html
	double asGray(void) const {return r*0.2126f+g*0.7152f+b*0.0722f;}
	
	//Return this Color's perceptual "weight", on 0..1
	double weight(void) const {return a*asGray();}
	
	//Scale Color components to byte (must already be scaled/clipped)
	byte redByte(void) const {return byte(255.99f*r);}
	byte greenByte(void) const {return byte(255.99f*g);}
	byte blueByte(void) const {return byte(255.99f*b);}
	byte alphaByte(void) const {return byte(255.99f*a);}
	
	//Get our value as bytes
	void getBytes(byte *dest,int dR,int dG,int dB) const
		{dest[dR]=redByte();dest[dG]=greenByte();dest[dB]=blueByte();}
	void getRgb(byte *dest) const {getBytes(dest,0,1,2);}
	void getBgr(byte *dest) const {getBytes(dest,2,1,0);}
	void getRgba(byte *dest) const {getRgb(dest);dest[3]=alphaByte();}
	void getBgra(byte *dest) const {getBgr(dest);dest[3]=alphaByte();}
	void getGray(byte *dest) const {dest[0]=byte(255.99*asGray());}
	
	//Set our value from bytes
	const static float scaleFromByte;
	static Color makeBytes(const byte *s,int dR,int dG,int dB) 
		{return Color(scaleFromByte*s[dR],scaleFromByte*s[dG],scaleFromByte*s[dB]);}
	static Color makeBytes(const byte *s,int dR,int dG,int dB,int dA) 
		{return Color(scaleFromByte*s[dR],scaleFromByte*s[dG],scaleFromByte*s[dB],
			premultiplied(scaleFromByte*s[dA]));}
	static Color makeRgb(const byte *s) {return makeBytes(s,0,1,2);}
	static Color makeBgr(const byte *s) {return makeBytes(s,2,1,0);}
	
	static Color makeRgba(const byte *s) {return makeBytes(s,0,1,2,3);}
	static Color makeBgra(const byte *s) {return makeBytes(s,2,1,0,3);}
	static Color makeGray(const byte *s) 
	  {float f=scaleFromByte*s[0];return Color(f,f,f);}
	
	void setRgb(const byte *s) {*this=makeRgb(s);}
	void setBgr(const byte *s) {*this=makeBgr(s);}
	void setRgba(const byte *s) {*this=makeRgba(s);}
	void setBgra(const byte *s) {*this=makeBgra(s);}
	void setGray(const byte *s) {*this=makeGray(s);}
	
	Color &add(const Color &by) {r+=by.r; g+=by.g; b+=by.b; a+=by.a; return *this;}
	void addScale(const Color &by,float s) 
		{r+=s*by.r; g+=s*by.g; b+=s*by.b; a+=s*by.a;}
	//Scale this Color by another: multiply corresponding components
	Color &scale(const Color &by) {r*=by.r;g*=by.g;b*=by.b; return *this;}
	Color &scale(float by) {r*=by;g*=by;b*=by; return *this;}

	Color operator+(const Color &o) const 
		{return Color(r+o.r, g+o.g, b+o.b, premultiplied(a+o.a));}
	Color &operator+=(const Color &o) 
		{r+=o.r; g+=o.g; b+=o.b; a+=o.a;return *this;}
	Color operator-(const Color &o) const 
		{return Color(r-o.r, g-o.g, b-o.b, premultiplied(a-o.a));}
	Color operator*(float c) const
		{return Color(r*c, g*c, b*c, premultiplied(a*c));}
	Color &operator*=(float c)
		{r*=c; g*=c; b*=c; a*=c; return *this;}
	Color &operator*=(double c)
		{return this->operator*=((float)c);}
	Color &operator*=(const Color& o)
		{r*=o.r; g*=o.g; b*=o.b; a*=o.a;return *this;}
	friend Color operator*(float c,const Color &o)
		{return Color(o.r*c, o.g*c, o.b*c, premultiplied(o.a*c));}
	friend Color operator*(double cd,const Color &o)
		{float c=(float)cd; return Color(o.r*c, o.g*c, o.b*c, premultiplied(o.a*c));}
	Color operator*(const Color& c) const
		{return Color(r*c.r, g*c.g, b*c.b, premultiplied(a*c.a));}
};

}; }; /*end namespace*/

#endif /* def(thisHeader) */
