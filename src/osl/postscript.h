/*
Adobe PostScript(tm) output utility class
Orion Sky Lawlor, olawlor@acm.org, 10/24/1999

This class implements an osl::Graphics that writes
out Postscript.
*/

#ifndef __OSL_POSTSCRIPT_H
#define __OSL_POSTSCRIPT_H

#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif

#include <stdio.h>

namespace osl { namespace graphics2d {

//A utility class for creating PostScript images
class Postscript:public Graphics {
protected:
	typedef Graphics super;
	friend class PostscriptPath;
	GraphicsState cur;//Current state of Postscript VM
	bool curText;
	void update(const GraphicsState &to,bool toText=false);
	void update(const Stroke &s);
	
	Bbox2d bbox; //Bounding box, in page coordinates
	Vector2d add(const Vector2d &o);
	Vector2d psMap(const Vector2d &p);
	
	int pageNo;
	bool isEPS;
	void startDoc(void);
	void endDoc(void);
	void printBBox(void);
	void startPage(void);
	void endPage(void);
	
	//Implementation utilities
	FILE *out;
	void newPath(void);//Start a new path
	void p_moveto(const Vector2d &v);//Set the path Point to x,y
	void p_lineto(const Vector2d &v);//Path a line from current to x,y
	
	void coord(double x);//Push given coordinate to file	
	void p(double x);//Push given value to file	
	void p(const char *str);//Push given string
	void p(const Vector2d &v);//Push vector as coordinate
	void cmd(const char *command);//Write given command to file
	
public:
	//Meta-commands (Postscript-specific)
	Postscript(const char *fName,bool isEPS=true);//Begin writing to the given file name
	void addCommand(const char *command);//Add arbitrary Postscript
	void addComment(const char *comment);//Add comment
	void nextPage(void);//Advance to next page
	~Postscript();//Done-- close file

//Standard Graphics commands
	//Name should be one of "Times", "Symbol", "Courier", or "Helvetica"
	virtual Font *newFont(const char *name,double size=14.0);
	
	virtual void character(gs &s,int c);//Draw single (non-control) char
	
	virtual void copy(cgs &s,const Raster &src);
	
	virtual void fill(cgs &gs,const Shape &s);
	
	virtual void stroke(cgs &state,const Shape &shape);

	virtual void clear(const Color &c);
};

}; };

#endif //__OSL_POST_H


