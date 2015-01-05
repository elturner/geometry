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
	 * Checks if two AABBs intersect each other
	 *
	 * Given the geometry of two axis-aligned bounding boxes,
	 * will check for an intersection.  The order of the boxes to
	 * this call does not matter.
	 *
	 * Intersections are computed excluding the edges of the boxes,
	 * which requires strict inequalities.
	 * 
	 * @param ax    The {xmin, xmax} bounds of box 'a'
	 * @param ay    The {ymin, ymax} bounds of box 'a'
	 * @param bx    The {xmin, xmax} bounds of box 'b'
	 * @param by    The {ymin, ymax} bounds of box 'b'
	 *
	 * @return      Returns true iff an intersection occurs
	 */
	inline bool aabb_in_aabb(const double ax[2], const double ay[2],
				const double bx[2], const double by[2])
	{
		/* check if intersect occurs in x-direction */
		if(ax[0] >= bx[1] || bx[0] >= ax[1])
			return false; /* no overlap in x */

		/* check if intersect occurs in y-direction */
		if(ay[0] >= by[1] || by[0] >= ay[1])
			return false; /* no overlap in y */
	
		/* an intersection must occur */
		return true;
	}

	/**
	 * Checks if two axis-aligned bounding boxes abut
	 *
	 * Checks if the two given aabb's abut (that is, they share an
	 * edge, but have zero overlapping area).  The two boxes are
	 * denoted as 'a' and 'b', where the min/max bounds in each
	 * dimension are given for each box.
	 *
	 * Additionally, an error threshold is given to account for
	 * any sort of rounding error that is occurred in the
	 * construction of these bounds.
	 *
	 * @param ax    The {xmin, xmax} bounds of box 'a'
	 * @param ay    The {ymin, ymax} bounds of box 'a'
	 * @param bx    The {xmin, xmax} bounds of box 'b'
	 * @param by    The {ymin, ymax} bounds of box 'b'
	 * @param err   The error threshold for computation
	 *
	 * @return      Returns true iff the boxes abut
	 */
	inline bool aabb_pair_abut(double ax[2], double ay[2],
	                           double bx[2], double by[2], double err)
	{
		/* check if any of the four possible edge-pairs abut */

		/* check if overlapped in y */
		if(ay[0] < by[1] && by[0] < ay[1])
		{
			/* ax-min touches bx-max */
			if(ax[0] >= bx[1] - err && ax[0] <= bx[1] + err)
				return true;

			/* ax-max touches bx-min, and overlapped in y */
			if(bx[0] >= ax[1] - err && bx[0] <= ax[1] + err)
				return true;
		}

		/* check if overlapped in x */
		if(ax[0] < bx[1] && bx[0] < ax[1])
		{
			/* ay-min touches by-max */
			if(ay[0] >= by[1] - err && ay[0] <= by[1] + err)
				return true;

			/* ax-max touches bx-min, and overlapped in y */
			if(by[0] >= ay[1] - err && by[0] <= ay[1] + err)
				return true;
		}

		/* no checks match */
		return false;
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

	/**
	 * Computes the linear center of a triangle, given its vertices
	 *
	 * Will find the linear center of the given triangle, based on
	 * the input vertices.  This is just the average position of
	 * the three vertices given.
	 *
	 * @param px   The x-coordinate of point p
	 * @param py   The y-coordinate of point p
	 * @param qx   The x-coordinate of point q
	 * @param qy   The y-coordinate of point q
	 * @param rx   The x-coordinate of point r
	 * @param ry   The y-coordinate of point r
	 * @param sx   Where to store the x-coordinate of point s
	 * @param sy   Where to store the y-coordinate of point s
	 */
	inline void triangle_center(double px, double py,
	                            double qx, double qy,
	                            double rx, double ry,
	                            double& sx, double& sy)
	{
		/* compute average */
		sx = (px + qx + rx) / 3;
		sy = (py + qy + ry) / 3;
	}

	/**
	 * Finds the intersection point between two line segments
	 *
	 * Given the endpoints of two line segments, will determine
	 * the point of intersection, and return the fraction along
	 * the first line of that point:
	 *
	 * intersection point = v0 + (v1-v0)*<return value>
	 *
	 * @param v0x  The X-val of starting point of first line segment
	 * @param v0y  The Y-val of starting point of first line segment
	 * @param v1x  The X-val of ending point of first line segment
	 * @param v1y  The Y-val of ending point of first line segment
	 * @param w0x  The X-val of starting point of second line segment
	 * @param w0y  The Y-val of starting point of second line segment
	 * @param w1x  The X-val of ending point of second line segment
	 * @param w1y  The Y-val of ending point of second line segment
	 *
	 * @return  Returns fraction along line v of intersection point
	 */
	inline double line_intersect(double v0x, double v0y,
				double v1x, double v1y,
				double w0x, double w0y,
				double w1x, double w1y)
	{
		bool v_vert, w_vert;
		double v_slope, w_slope, q;

		/* check if either line is vertical, for efficiency */
		v_vert = (v0x == v1x);
		w_vert = (w0x == w1x);

		/* check edge case of both lines vertical */
		if( v_vert && w_vert )
			return 0; /* parallel lines */
		else if(v_vert) /* v is vertical, w non-vertical */
		{
			/* get intersection point by finding w(v_x) */
			w_slope = (w1y - w0y) / (w1x - w0x);
			q = w0y + w_slope*(v0x - w0x);

			/* q represents the y-value at intersection point,
			 * so return fraction along v */
			return (q - v0y) / (v1y - v0y);
		}
		else if(w_vert) /* w is vertical, v non-vertical */
		{
			/* get intersection point by finding v(w_x) */
			v_slope = (v1y - v0y) / (v1x - v0x);
			q = v0y + v_slope*(w0x - v0x);

			/* q represents the y-value at intersection point,
			 * so return the fraction along v */
			return (q - w0y) / (w1y - w0y);
		}
		
		/* neither line is vertically aligned, so we
		 * can compute slopes */
		v_slope = (v1y - v0y) / (v1x - v0x);
		w_slope = (w1y - w0y) / (w1x - w0x);

		/* check if parallel */
		if(v_slope == w_slope)
			return 0; /* parallel lines */
		
		/* get x-coordinate of intersection point */
		q = ((w0y - w_slope*w0x) - (v0y - v_slope*v0x))
				/ (v_slope - w_slope);
		
		/* get fraction of intersection along v */
		return (q - v0x) / (v1x - v0x);
	}
}

#endif
