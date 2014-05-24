/*
Defines the pointsetwindow interface

Orion Sky Lawlor, olawlor@acm.org, 10/11/2002
*/
#ifndef __OSL_GUI_POINTSET_H
#define __OSL_GUI_POINTSET_H

#ifndef __OSL_GUI_H
#  include "osl/gui.h"
#endif
#ifndef __OSL_VECTOR2D_H
#  include "osl/vector2d.h"
#endif
#ifndef __OSL_MATRIX3D_H
#  include "osl/matrix3d.h"
#endif

namespace osl { 
 namespace gui {

using osl::Point;
using osl::Vector2d;
using osl::Matrix2d;

/**
 * A CoordMapWindow contains a private internal coordinate system.
 */
class CoordMapWindow : public RasterizerWindow {
	typedef RasterizerWindow super;
	Matrix2d orig_m; //non-pixel version of map: 
	Matrix2d m_off2on; //off to onscreen map
	Matrix2d m_on2off; //on to offscreen map
public:
	/// Initialize this window with a unit map.
	CoordMapWindow(const WindowCreateOptions &o);
	
	/// Set up the offscreen->onscreen matrix, to map these
	///  ranges onscreen.  Does not call redraw.
	virtual void setRanges(double minX,double maxX,
		double minY,double maxY);
	
	/// Set the offscreen->onscreen matrix, which projects to (0,1)
	virtual void setMatrix(const Matrix2d &m_);
	
	/// Return the off-to-onscreen map (onFmOff)
	inline const Matrix2d &getMatrix(void) const {return m_off2on;}
	/// Return the on-to-offscreen map (offFmOn)
	inline const Matrix2d &getInverseMatrix(void) const {return m_on2off;}
	
	/// Coordinate mappings:
	inline Vector2d off2on(const Vector2d &v) const {
		return m_off2on.applyInline(v);
	}
	inline Vector2d on2off(const Vector2d &v) const {
		return m_on2off.applyInline(v);
	}
	inline Vector2d on2off(const Point &p) const {
		return on2off(Vector2d(p.x,p.y));
	}
	
	/// Updates the matrix for the new size.
	virtual void windowResized(const Event &e);
};

/**
 * A PointSetWindow allows you to draw and interact with a small set of Points.
 */
class impl_PointSet;
class PointSetWindow : public CoordMapWindow {
	typedef CoordMapWindow super;
	impl_PointSet *s;
public:
	PointSetWindow(const WindowCreateOptions &o,
		int nPts,const Vector2d *v);
	virtual ~PointSetWindow();
	
	/// GUI Interaction: drag points around
	virtual void mousePressed(const Event &e);
	virtual void mouseDragged(const Event &e);
	virtual void mouseReleased(const Event &e);
	virtual void keyTyped(const Event &e);
	
	/// Pointset accessor routines
	int getPoints(void) const;
	
	inline Vector2d getOn(int p) const { //Get onscreen location of point 
		return off2on(getOff(p));
	}
	const Vector2d *getOn(void) const;
	
	Vector2d getOff(int p) const; //Get offscreen location of point p
	const Vector2d *getOff(void) const;
	
	/// Pointset modification routines.  Call repaint when done
	virtual int addPoint(const Vector2d &newLoc);
	virtual void movePoint(int p,const Vector2d &newLoc);
	virtual void deletePoint(int p);
	
	/// Read/write points from this ASCII file:
	void readPoints(const char *fName);
	void writePoints(const char *fName);
	
	/// Recreate the image (called whenever a point moves)
	virtual void paint(::osl::graphics2d::Graphics &grafport);
	
	/// Rescale view mapping to fit points.
	///  If squareAspect is true, view region will be square.
	void rescaleMap(bool squareAspect=true);
};

 };
}; //End namespaces

#endif /* def(thisHeader) */
