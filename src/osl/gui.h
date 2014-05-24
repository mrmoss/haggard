/*
Defines the osl::gui interface.  
Individual Windows created using this interface
are NOT threadsafe-- a single Window is not reentrant.
However, different Windows ARE threadsafe-- multiple
threads can access separate Windows.

Orion Sky Lawlor, olawlor@acm.org, 5/19/2001
*/
#ifndef __OSL_GUI_H
#define __OSL_GUI_H

#ifndef __OSL_H
#  include "osl/osl.h"
#endif
#ifndef __OSL_GRAPHICS_H
#  include "osl/graphics.h"
#endif
#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif

namespace osl { 
 namespace gui {

using osl::Point;

//Modelled after java.awt.Event
class Event {
public:
	/// Different kinds of events.
	typedef enum {
		EVENT_MAJOR=0xf0,//Mask for major Event type
		EVENT_MINOR=0x0f,//Mask for minor Event type
		
		INVALID_EVENT=0, //Invalid major Event type
		
		KEY_EVENT=0x10,  //Major Event type for keyboard
		KEY_PRESS, KEY_RELEASE, //Key going up and down
		KEY_TYPE, //A typed character
		
		MOUSE_EVENT=0x20, //Major Event type for mouse
		MOUSE_PRESS, MOUSE_RELEASE, MOUSE_CLICK,
		MOUSE_ENTER, MOUSE_EXIT,
		MOUSE_MOVE, MOUSE_DRAG,
		
		WINDOW_EVENT=0x70, //Major Event type for Window
		WINDOW_ACTIVATE, WINDOW_DEACTIVATE, 
		WINDOW_MOVE, WINDOW_RESIZE, 
		WINDOW_REPAINT, WINDOW_CLOSE,

		MENU_EVENT=0x80, //A menu item was selected

		LAST_EVENT=0xFF
	} type_t;
	
	/// Command key modifiers
	enum { 
		NONE_MASK=0,
		SHIFT_MASK=1<<0,
		CTRL_MASK=1<<1,
		META_MASK=1<<2,  /*On Macs, the command key*/
		ALT_MASK=1<<3
	};
	typedef unsigned char modifiers_t;
	
	/// Special "action" keys (beyond Unicode).
	typedef enum {
		ACTION_START=0x1100ff00,
		HOME,END,PGUP,PGDN,
		UP,DOWN,LEFT,RIGHT,
		F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,
		PRINT_SCREEN,SCROLL_LOCK,CAPS_LOCK,NUM_LOCK,
		PAUSE,INSERT,
		SHIFT,RSHIFT,CTRL,RCTRL,ALT,RALT,META,RMETA,
		ACTION_END
	} actionKey_t;
	
	/// Regular key sequences (ASCII/Unicode)
	typedef enum {
		ENTER='\n',RETURN='\r',BACK_SPACE='\b',TAB='\t',
		SPACE=' ',ESCAPE=27,DELETE=127
	} key_t;
	
	/// Alternate names for some keys
	enum {
		ESC=ESCAPE,DEL=DELETE
	};
private:
	type_t type;///< Kind of Event
	double when;///< Time of Event (seconds, from osl::time())
	modifiers_t modifiers;///< Modifier keys held down (bitmask)
	int key;///< System-dependent keycode (Key Events)
	int charTyped;///< Unicode character (KEY_TYPE Events)
	Point loc;///< Location of Event (Mouse, Window Events only)
	int buttons;///< Mouse buttons now held down (bitmask)
public:
	Event(type_t t,double w,modifiers_t m=NONE_MASK) 
		:loc(-1,-1)
	{
		type=t; when=w; modifiers=m;
		key=charTyped=-1;
		buttons=-1;
	}
	Event(type_t t,double w,modifiers_t m,int k,int c) 
		:loc(-1,-1)
	{
		type=t; when=w; modifiers=m;
		key=k;
		charTyped=c;
		buttons=-1;
	}
	Event(type_t t,double w,modifiers_t m,Point l,int b) 
		:loc(l)
	{
		type=t; when=w; modifiers=m;
		key=charTyped=-1;
		buttons=b;
	}
	
	type_t getType(void) const {return type;}
	modifiers_t getModifiers(void) const {return modifiers;}
	bool shiftDown(void) const {
		return 0!=(modifiers&SHIFT_MASK);
	}
	bool ControlDown(void) const {
		return 0!=(modifiers&CTRL_MASK);
	}
	bool metaDown(void) const {
		return 0!=(modifiers&META_MASK);
	}
	double getWhen(void) const {return when;}

	/// Get selected menu item code
	int getMenuItem(void) const {return key;}
	
	/// Get system-dependent key code
	int getKeycode(void) const {return key;}
	/// Get Unicode key pressed
	int getKey(void) const {return charTyped;}
	
	void translate(int dx, int dy) {translate(Point(dx,dy));}
	void translate(const Point &p) {loc=loc+p;}
	int getX(void) const {return loc.x;}
	int getY(void) const {return loc.y;}
	const Point &getLoc(void) const {return loc;}
	int getButtons(void) const {return buttons;}
	bool buttonDown(int bNo=0) const {return 0!=(buttons & (1<<bNo));}
};

/**
  An EventHandler is passed all types of events. 
  It's used for various sorts of event filtering.
*/
class EventHandler : public Noncopyable {
public:
	virtual ~EventHandler();
	virtual void handleEvent(const Event &e) =0;
};

/// Parameters for creating a top-level Window.
class WindowCreateOptions {
public:
	//Bitmask for Window creation options
	typedef enum {
		ORDINARY=   0, //Regular Window, with frame
		NOBORDER=   1<<0, //Window has no border (e.g., for toolbars)
		DIALOG=     1<<1, //Dialog box, which may have a smaller frame
		INVISIBLE=  1<<2, //Window initially hidden
		MENU=       1<<3, //Window will have a menu bar
		NOBACKING=  1<<3  //Window will not have a backing store
	} window_t;
private:
	Point size;//Window initial size
	const char *title;//Window initial title
	window_t options;
public:
	//Create an ordinary, visible Window of the given size
	WindowCreateOptions(Point size_,const char *title_="Default Window Name",window_t opt_=ORDINARY)
		:size(size_),title(title_),options(opt_) { }
	WindowCreateOptions(int wid,int ht,const char *title_="Default Window Name",window_t opt_=ORDINARY)
		:size(wid,ht),title(title_),options(opt_) { }
	
	//Create an initially invisible Window with a stupid size (1x1)
	WindowCreateOptions(const char *title_,window_t opt_=INVISIBLE)
		:size(1,1),title(title_),options(opt_) { }
	
	Point getSize(void) const {return size;}
	const char *getTitle(void) const {return title;}
	window_t getOptions(void) const {return options;}
	bool hasOption(window_t o) const {return (options&o)!=0;}
	WindowCreateOptions addOption(window_t o) const {
		return WindowCreateOptions(size,title,(window_t)(options|o));
	}
};

/// A top-level Window (in Java, a "Frame").
class Window : public EventHandler {
	/// System-dependent implementation Window-- everything but 
	///  the callbacks just pass the buck here.
	Window *toolkit;
protected:
	Point topLeft,size;///< Coordinates and extent of Window
	boolean isVisible;///< Window visibility flag
	const char *title;///< Current Window title
	/// Factory method for system-dependent toolkit Window
	static Window *buildToolkit(const WindowCreateOptions &opts,Window *forWindow);
	bool needsRepaint; ///< If true, we need to repaint this window
		
	/// Toolkit-building interface: create a Window without a toolkit
	class toolkitWindowConstructor { };
	Window(const WindowCreateOptions &opts,toolkitWindowConstructor tag);
public:
	/// Create a new Window, with an attached toolkit.
	Window(const WindowCreateOptions &opts);
	virtual ~Window();//Window destroyed
	
	/// Change window location
	virtual void setLocation(const Point &topLeft_);
	void setLocation(int x,int y) {setLocation(Point(x,y));}
	Point getLocation(void) const {return topLeft;}

	/// Change window size
	virtual void setSize(const Point &size);
	void setSize(int w,int h) {setSize(Point(w,h));}
	Point getSize(void) const {return size;}

	/// Change window visibility
	virtual void setVisible(boolean to=true);
	boolean getVisible(void) const {return isVisible;}

	/// Change window title
	virtual void setTitle(const char *str);
	const char *getTitle(void) const {return title;}
	
	/**
	 * Draw the given Raster at the given (topleft) location
	 * in this Window.
	 */
	virtual void draw(const ::osl::graphics2d::Raster &r,int x,int y);
	
	/// Call repaint after the next event lookup.
	///  Low-priority: won't block real work.
	inline void repaintLater(void) { needsRepaint=true; }
	
	/// Recreate and paint a new image to the screen.
	///  All window classes must override this method, which must 
	///  eventually call draw.
	virtual void repaint(void) =0;
	
// Event handling
	/**
	 * If an Event is pending for this Window,
	 *  call handleEvent and return true.  
 	 * Otherwise return false.
 	 */
	virtual bool checkEvent(void);
	
	/// Sleep until an Event comes in for this Window, then call handleEvent
	virtual void waitEvent(void);
	
	/// Keep calling waitEvent until the window is closed
	virtual void runEventLoop(void);
	
//The remaining methods are user-overridable callbacks.
	
	/// Handle an incoming Event (default: dispatch based on type)
	virtual void handleEvent(const Event &e);
	
	//Default Event responses: empty
	/// Mouse button(s) going down.
	virtual void mousePressed(const Event &e);
	/// Mouse button(s) going back up.
	virtual void mouseReleased(const Event &e);
	/// Mouse button(s) went down and then back up.
	virtual void mouseClicked(const Event &e);
	/// Mouse entering the window.
	virtual void mouseEntered(const Event &e);
	/// Mouse leaving the window.
	virtual void mouseExited(const Event &e);
	/// Mouse moved *without* buttons down. ("hovering")
	virtual void mouseMoved(const Event &e);
	/// Mouse moved *with* buttons down. ("dragging")
	virtual void mouseDragged(const Event &e);
    	
	/// The key e.getKey() has been pressed down
	virtual void keyPressed(const Event &e);
	/// The key e.getKey() has been lifted up
	virtual void keyReleased(const Event &e);
	/// The key e.getKey() has been pressed and released, or "typed"
	virtual void keyTyped(const Event &e);
	
	/// This menu item has been selected
	virtual void menuEvent(const Event &e);
	
	/// This window has been activated, to the foreground
	virtual void windowActivated(const Event &e);
	/// This window has been sent to the background
	virtual void windowDeactivated(const Event &e);
	/// This window has been moved on the screen
	virtual void windowMoved(const Event &e);
	/// This window's size has been changed
	virtual void windowResized(const Event &e);
	
	/// Redraw yourself (default: if have backing store, redraw)
	virtual void windowRepaint(const Event &e);
	/// You're about to be closed (default: set windowDone to true)
	virtual void windowClosing(const Event &e);
	
};



//A Window with an explicit back buffer and (optional) Rasterizer
class RasterizerWindow : public Window {
	typedef Window super;
protected:
	::osl::graphics2d::RgbRaster rast;//Output Window image
	::osl::graphics2d::Rasterizer grafport;//Output Window Graphics Device
	::osl::graphics2d::Font *defaultFont;//A reasonable Font
	bool reallocateRast(const Point &p);
public:
	/// Recreate the image (called for each repaint).
	///  You must override this class if you inherit from RasterizerWindow.
	virtual void paint(::osl::graphics2d::Graphics &grafport) =0;
	
	/// This window's size has been changed.  Reallocate rast.
	virtual void windowResized(const Event &e);
	
	/// Recreate and paint a new image to the screen.
	///  Prepares rast, and calls paint(grafport);
	virtual void repaint(void);
	
	RasterizerWindow(const WindowCreateOptions &opts);
	virtual ~RasterizerWindow();
};

 };
}; //End namespaces

#endif /* def(thisHeader) */
