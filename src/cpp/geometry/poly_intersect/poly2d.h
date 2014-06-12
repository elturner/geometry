#ifndef POLY2D_H
#define POLY2D_H

/**
 * @file poly2d.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines functions useful for checking intersections
 * of 2D shape primitives and polygons.
 */

/**
 * This namespace holds functions for 2D polygon intersection tests
 */
namespace poly2d
{
	/**
	 * Checks if a point intersects an axis-aligned bounding box
	 *
	 * Checks if a given point is contained in an axis-aligned
	 * bounding box.  Yes, this is a simple test.  Edges of
	 * box are considered inclusive.
	 *
	 * @param x     x-coordinate of point
	 * @param y     y-coordinate of point
	 * @param xmin  left edge of bounding box
	 * @param ymin  bottom edge of bounding box
	 * @param xmax  right edge of bounding box
	 * @param ymax  top edge of bounding box
	 *
	 * @return      Returns true iff (x,y) is in box
	 */
	inline bool point_in_aabb(double x, double y,
	                          double xmin, double ymin,
	                          double xmax, double ymax)
	{
		if(x < xmin || x > xmax)
			return false;
		if(y < ymin || y > ymax)
			return false;
		return true;
	}

	/**
	 * Checks if a line segment intersects an axis-aligned bounding box
	 *
	 * Given two points that define a line segment,
	 * will check if that line intersects a specified bounding box.
	 *
	 * @param ax        The x-coordinate of point a of the line
	 * @param ay        The y-coordinate of point a of the line
	 * @param bx        The x-coordinate of point b of the line
	 * @param by        The y-coordinate of point b of the line
	 * @param bounds_x  The {xmin, xmax} bounds of the box
	 * @param bounds_y  The {ymin, ymax} bounds of the box
	 *
	 * @return          Returns true iff line defined by (a,b) 
	 *                  intersects box.
	 */
	inline bool line_in_aabb(double ax, double ay, double bx, double by,
	                         double bounds_x[2], double bounds_y[2])
	{
		double tmin, tmax, tymin, tymax;
		double invdir_x, invdir_y;
		int sx, sy;

		/* initialize values */
		invdir_x = 1.0 / (bx - ax);
		invdir_y = 1.0 / (by - ay);
		sx = (invdir_x < 0);
		sy = (invdir_y < 0);

		/* compute intersections in x-coordinates */
		tmin = (bounds_x[sx] - ax)
		 		* invdir_x; 
		tmax = (bounds_x[1-sx] - ax)
		 		* invdir_x; 
		 
		/* get equivalent values for y-coordinates */
		tymin = (bounds_y[sy] - ay)
		 		* invdir_y; 
		tymax = (bounds_y[1-sy] - ay)
		 		* invdir_y;
		 
		/* check that line intersects the xy square of
		 * the projection of this cube */
		if ((tmin > tymax) || (tymin > tmax))
			return false; /* no intersection */
		if (tymin > tmin)
			tmin = tymin; /* flip so ray is positive */
		if (tymax < tmax)
			tmax = tymax; /* flip so ray is positive */
			
		/* check if line is too short to intersect box */
		if(tmin > tmax || tmin > 1 || tmax < 0)
			return false;

		/* line segment must intersect */
		return true;
	}

	/**
	 * Determines the 2D orientation of three points
	 *
	 * The value will positive if pqr are oriented counter-clockwise,
	 * negative if they are oriented clockwise, and zero if they
	 * are colinear.
	 *
	 * The return value is the signed area of the parallelogram
	 * defined by the angle pqr.
	 *
	 * @param px   The x-coordinate of point p
	 * @param py   The y-coordinate of point p
	 * @param qx   The x-coordinate of point q
	 * @param qy   The y-coordinate of point q
	 * @param rx   The x-coordinate of point r
	 * @param ry   The y-coordinate of point r
	 *
	 * @return  Returns signed-area of parallelogram defined by pqr
	 */
	inline double orient2D(double px, double py,
	                       double qx, double qy,
	                       double rx, double ry)
	{
		/* compute determinant of matrix:
		 *
		 * 	(px-rx)		(py-ry)
		 *
		 * 	(qx-rx)		(qy-ry)
		 */
		return (px-rx)*(qy-ry) - (py-ry)*(qx-rx);
	}

	/**
	 * Checks if point s is in triangle pqr
	 *
	 * Will check if the given 2D point resides inside the given
	 * triangle.  Note that the edges of the triangle are considered
	 * inclusive.
	 *
	 * The triangle pqr must be given in counter-clockwise order
	 *
	 * @param px   The x-coordinate of point p
	 * @param py   The y-coordinate of point p
	 * @param qx   The x-coordinate of point q
	 * @param qy   The y-coordinate of point q
	 * @param rx   The x-coordinate of point r
	 * @param ry   The y-coordinate of point r
	 * @param sx   The x-coordinate of point s
	 * @param sy   The y-coordinate of point s
	 *
	 * @return     Returns true iff s is inside triangle pqr
	 */
	inline bool point_in_triangle(double px, double py,
	                              double qx, double qy,
	                              double rx, double ry,
	                              double sx, double sy)
	{
		double opq, oqr, orp;

		/* orient each edge */
		opq = orient2D(px,py,qx,qy,sx,sy);
		oqr = orient2D(qx,qy,rx,ry,sx,sy);
		orp = orient2D(rx,ry,px,py,sx,sy);

		/* point is inside iff all orientations are positive */
		return (opq >= 0) && (oqr >= 0) && (orp >= 0);
	}

	/**
	 * Computes the circumcenter of the triangle p,q,r.
	 *
	 * The circumcenter of the triangle pqr is stored in the
	 * vector (sx,sy).  This function will also compute the
	 * circumradius of the given triangle, whose square is
	 * returned.
	 *
	 * All triangles are assumed to be represented with counter-
	 * clockwise ordering.
	 *
	 * @param px   The x-coordinate of point p
	 * @param py   The y-coordinate of point p
	 * @param qx   The x-coordinate of point q
	 * @param qy   The y-coordinate of point q
	 * @param rx   The x-coordinate of point r
	 * @param ry   The y-coordinate of point r
	 * @param sx   Where to store the x-coordinate of point s
	 * @param sy   Where to store the y-coordinate of point s
	 *
	 * @return     Returns square of circum-radius of pqr
	 */
	inline double triangle_circumcenter(double px, double py,
	                                    double qx, double qy,
	                                    double rx, double ry,
	                                    double& sx, double& sy)
	{
		double a, b;

		/* compute circule */
		a = (qx*qx - px*px + qy*qy - py*py)*(ry-py)
		    - (rx*rx - px*px + ry*ry - py*py)*(qy-py);
		a = a / (2 * ( (qx-px)*(ry-py) - (qy-py)*(rx-px) ) );
		b = (qy*qy - py*py + qx*qx - px*px)*(rx - px)
		    - (ry*ry - py*py + rx*rx - px*px)*(qx-px);
		b = b / ( 2 * ( (qy-py)*(rx-px) - (qx-px)*(ry-py) ) );

		/* store results */
		sx = a;
		sy = b;

		/* return the circumradius of this triangle */
		a = px-sx;
		b = py-sy;
		return ( a*a + b*b );
	}
}

#endif
