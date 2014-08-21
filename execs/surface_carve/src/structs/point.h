#ifndef POINT_H
#define POINT_H

/* point.h
 *
 * This file defines the struct
 * used to store points, as well
 * as the functions used to manipulate
 * them
 */

/* this struct defines a single point */
typedef struct point
{
	/* the following represent position
	 * in 3D space.  The z-component represents
	 * vertical elevation */
	double x; /* meters */
	double y; /* meters */
	double z; /* meters */
	double timestamp;
	
	/* The following fields are defined
	 * in XYZ files, but are not used by
	 * this program:
	 *
	 * int r;
	 * int g;
	 * int b;
	 *	
	 * int id;
	 * int seriel;
	 */
} point_t;

/* This struct defines a bounding box in 3D space */ 
typedef struct boundingbox
{
	double x_min;
	double x_max;
	double y_min;
	double y_max;
	double z_min;
	double z_max;

} boundingbox_t;

/* dist_sq:
 *
 *	Will compute the square of the distance between
 *	a and b.
 *
 * arguments:
 *
 * 	a,b - 	The points to analyze
 *
 * return value:
 *
 * 	Returns the square of the distance
 */
double dist_sq(point_t& a, point_t& b);

/* midpoint:
 *
 *	m = (a + b)/2
 */
void midpoint(point_t& m, point_t& a, point_t& b);

/* boundingbox_init:
 *
 * 	Initializes a bounding box
 */
void boundingbox_init(boundingbox_t& bbox);

/* boundingbox_update:
 *
 * 	Updates a bounding box struct to include the given point.
 */
void boundingbox_update(boundingbox_t& bbox, point_t& p);

/* boundingbox_shift:
 *
 *	Shifts the specified bounding box to be centered at the
 *	given location.
 *
 * arguments:
 *
 * 	bbox -		The bounding box to modify
 * 	cx,cy,cz -	The center of the box after the shift
 */
void boundingbox_shift(boundingbox_t& bbox, 
		double cx, double cy, double cz);

/* boundingbox_contains:
 *
 *	Returns true iff the bounding box bbox contains the
 *	location (x,y,z).
 */
bool boundingbox_contains(boundingbox_t& bbox,
		double x, double y, double z);

#endif
