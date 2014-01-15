#include "geometry.h"
#include <float.h>
#include "../structs/point.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

void geom_center(point_t& r, point_t& a, point_t& b, point_t& c)
{
	int i;

	/* set r to be sum of the three input points */
	r = a;
	r += b;
	r += c;
	
	/* compute average in each dimension */
	for(i = 0; i < NUM_DIMS; i++)
		r.set(i, r.get(i)/3);
}

double geom_orient2D(point_t& p, point_t& q, point_t& r)
{
	double px, py, qx, qy, rx, ry;
	
	/* compute determinant of matrix:
	 *
	 * 	(px-rx)		(py-ry)
	 *
	 * 	(qx-rx)		(qy-ry)
	 */
	px = p.get(0);
	py = p.get(1);

	qx = q.get(0);
	qy = q.get(1);

	rx = r.get(0);
	ry = r.get(1);

	/* return the determinant */
	return (px-rx)*(qy-ry) - (py-ry)*(qx-rx);
}

double geom_line_intersect(point_t& v0,point_t& v1,point_t& w0,point_t& w1)
{
	bool v_vert, w_vert;
	double v_slope, w_slope, q;

	/* check edge cases */
	v_vert = (v0.get(0) == v1.get(0));
	w_vert = (w0.get(0) == w1.get(0));

	/* check which lines, if any, are vertical */
	if(v_vert && w_vert)
		return DBL_MAX; /* lines parallel */
	else if(v_vert) /* v is vertical, but w is not */
	{
		/* get intersection point by finding w(v_x) */
		w_slope = (w1.get(1) - w0.get(1)) 
				/ (w1.get(0) - w0.get(0));
		
		q = w0.get(1) + w_slope*(v0.get(0) - w0.get(0));
		
		/* q represents the y-value of the intersection point,
		 * so return the fraction along the vertical v this is */
		return ((q - v0.get(1)) / (v1.get(1) - v0.get(1)));
	}
	else if(w_vert) /* w is vertical, but v is not */
	{
		/* get intersection point by finding v(w_x) */
		v_slope = (v1.get(1) - v0.get(1)) 
				/ (v1.get(0) - v0.get(0));
		
		q = v0.get(1) + v_slope*(w0.get(0) - v0.get(0));
		
		/* q represents the y-value of the intersection point,
		 * so return the fraction along the vertical w this is */
		return ((q - w0.get(1)) / (w1.get(1) - w0.get(1)));
	}

	/* neither line is vertical, so we can compute slopes */
	v_slope = (v1.get(1) - v0.get(1)) / (v1.get(0) - v0.get(0));
	w_slope = (w1.get(1) - w0.get(1)) / (w1.get(0) - w0.get(0));

	/* check if parallel */
	if(v_slope == w_slope)
		return DBL_MAX;

	/* get x-coordinate of intersection point */
	q = ((w0.get(1) - w_slope*w0.get(0)) 
		- (v0.get(1) - v_slope*v0.get(0))) / (v_slope - w_slope);

	/* return fraction of intersection along v */
	return (q - v0.get(0)) / (v1.get(0) - v0.get(0));
}

