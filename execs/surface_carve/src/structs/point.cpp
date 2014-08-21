#include "point.h"
#include <float.h>

double dist_sq(point_t& a, point_t& b)
{
	double dx, dy, dz;

	/* compute distance */
	dx = (a.x) - (b.x);
	dy = (a.y) - (b.y);
	dz = (a.z) - (b.z);
	return (dx*dx) + (dy*dy) + (dz*dz);
}

void midpoint(point_t& m, point_t& a, point_t& b)
{
	m.x = 0.5 * (a.x + b.x);
	m.y = 0.5 * (a.y + b.y);
	m.z = 0.5 * (a.z + b.z);
}

void boundingbox_init(boundingbox_t& bbox)
{
	bbox.x_min = bbox.y_min = bbox.z_min = DBL_MAX;
	bbox.x_max = bbox.y_max = bbox.z_max = -DBL_MAX;
}

void boundingbox_update(boundingbox_t& bbox, point_t& p)
{
	/* check x dimension */
	if(p.x < bbox.x_min)
		bbox.x_min = p.x;
	if(p.x > bbox.x_max)
		bbox.x_max = p.x;

	/* check y dimension */
	if(p.y < bbox.y_min)
		bbox.y_min = p.y;
	if(p.y > bbox.y_max)
		bbox.y_max = p.y;

	/* check z dimension */
	if(p.z < bbox.z_min)
		bbox.z_min = p.z;
	if(p.z > bbox.z_max)
		bbox.z_max = p.z;
}

void boundingbox_shift(boundingbox_t& bbox,
		double cx, double cy, double cz)
{
	double dx, dy, dz;

	/* get relative shift values */
	dx = cx - 0.5*(bbox.x_max + bbox.x_min);
	dy = cy - 0.5*(bbox.y_max + bbox.y_min);
	dz = cz - 0.5*(bbox.z_max + bbox.z_min);

	/* edit box */
	bbox.x_min += dx;
	bbox.x_max += dx;
	bbox.y_min += dy;
	bbox.y_max += dy;
	bbox.z_min += dz;
	bbox.z_max += dz;
}

bool boundingbox_contains(boundingbox_t& bbox,
		double x, double y, double z)
{
	return (	   bbox.x_min <= x && x <= bbox.x_max
			&& bbox.y_min <= y && y <= bbox.y_max
			&& bbox.z_min <= z && z <= bbox.z_max );
}
