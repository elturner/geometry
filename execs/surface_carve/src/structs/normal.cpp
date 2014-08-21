#include "normal.h"
#include <math.h>
#include "triangulation.h"
#include "../util/parameters.h"

void normal_of_tri(normal_t& norm, triangle_t* t)
{
	double m;

	/* get determinant elements using triangle vertices */
	double ux = t->v[0]->x - t->v[2]->x;
	double uy = t->v[0]->y - t->v[2]->y;
	double uz = t->v[0]->z - t->v[2]->z;
	double vx = t->v[1]->x - t->v[2]->x;
	double vy = t->v[1]->y - t->v[2]->y;
	double vz = t->v[1]->z - t->v[2]->z;
	
	/* compute normal */
	norm.x = (uy * vz) - (uz * vy);
	norm.y = (uz * vx) - (ux * vz);
	norm.z = (ux * vy) - (uy * vx);

	/* compute magnitude */
	m = sqrt( (norm.x * norm.x) + (norm.y * norm.y) 
				+ (norm.z * norm.z) );

	/* normalize */
	norm.x = norm.x / m;
	norm.y = norm.y / m;
	norm.z = norm.z / m;
}

void normal_average(normal_t& avg, normal_t& a, double aw, 
					normal_t& b, double bw)
{
	double m;

	/* store the un-normalized direction in avg */
	avg.x = aw*a.x + bw*b.x;
	avg.y = aw*a.y + bw*b.y;
	avg.z = aw*a.z + bw*b.z;

	/* find magnitude */
	m = sqrt( (avg.x * avg.x) + (avg.y * avg.y) 
				+ (avg.z * avg.z) );

	/* normalize */
	if(fabs(m) > APPROX_ZERO)
	{
		avg.x /= m;
		avg.y /= m;
		avg.z /= m;
	}
}

double height_from_plane(point_t& p, normal_t& pn, point_t& pp)
{
	normal_t q;

	/* get location of r relative to pp */
	q.x = p.x - pp.x;
	q.y = p.y - pp.y;
	q.z = p.z - pp.z;
	return NORMAL_DOT(q, pn);
}

void project_point_to_plane(point_t& s, point_t& r, 
				normal_t& pn, point_t& pp)
{
	normal_t q;
	double d;

	/* get location of r relative to pp */
	q.x = r.x - pp.x;
	q.y = r.y - pp.y;
	q.z = r.z - pp.z;
	d = NORMAL_DOT(q, pn);

	/* subtract orthogonal component */
	s.x = r.x - d*pn.x;
	s.y = r.y - d*pn.y;
	s.z = r.z - d*pn.z;
}

void project_point_to_plane_plane(point_t& dest, point_t& src,
					normal_t& n1, point_t& p1,
					normal_t& n2, point_t& p2)
{
	normal_t r, n3;
	double h1, h2, c1, c2;
	double d, ds;

	/* record source location */
	r.x = src.x;
	r.y = src.y;
	r.z = src.z;

	/* compute the dot-product of the normals */
	d = NORMAL_DOT(n1, n2);
	ds = d*d;

	/* compute the height of each plane */
	h1 = NORMAL_DOT(p1, n1);
	h2 = NORMAL_DOT(p2, n2);

	/* find the coefficents for the line of intersection.  This
	 * approach described on:
	 *
	 * en.wikipedia.org/wiki/Plane_(geometry)#Line_of
	 * 		_intersection_between_two_planes
	 */
	c1 = ( h1 - h2*d ) / ( 1 - ds );
	c2 = ( h2 - h1*d ) / ( 1 - ds );

	/* get direction along this line */
	n3.x = (n1.y * n2.z) - (n1.z * n2.y);
	n3.y = (n1.z * n2.x) - (n1.x * n2.z);
	n3.z = (n1.x * n2.y) - (n1.y * n2.x);
	d = NORMAL_MAGNITUDE(n3);
	n3.x /= d;
	n3.y /= d;
	n3.z /= d;
	
	/* represent any point along this line */
	dest.x = c1 * n1.x + c2 * n2.x;
	dest.y = c1 * n1.y + c2 * n2.y;
	dest.z = c1 * n1.z + c2 * n2.z;

	/* find closest point on this line to src */
	r.x -= dest.x;
	r.y -= dest.y;
	r.z -= dest.z;

	d = NORMAL_DOT(r, n3);
	
	dest.x += d * n3.x;
	dest.y += d * n3.y;
	dest.z += d * n3.z;
}

void intersect_three_planes(point_t& dest, normal_t& n1, point_t& p1,
					normal_t& n2, point_t& p2,
					normal_t& n3, point_t& p3)
{
	normal_t p, q, s;
	double h1, h2, c1, c2;
	double d, ds, lambda;

	/* first, compute line of intersection of two planes */

	/* compute the dot-product of the normals */
	d = NORMAL_DOT(n1, n2);
	ds = d*d;

	/* compute the height of each plane */
	h1 = NORMAL_DOT(p1, n1);
	h2 = NORMAL_DOT(p2, n2);

	/* find the coefficents for the line of intersection.  This
	 * approach described on:
	 *
	 * en.wikipedia.org/wiki/Plane_(geometry)#Line_of
	 * 		_intersection_between_two_planes
	 */
	c1 = ( h1 - h2*d ) / ( 1 - ds );
	c2 = ( h2 - h1*d ) / ( 1 - ds );

	/* get direction along this line */
	s.x = (n1.y * n2.z) - (n1.z * n2.y);
	s.y = (n1.z * n2.x) - (n1.x * n2.z);
	s.z = (n1.x * n2.y) - (n1.y * n2.x);
	d = NORMAL_MAGNITUDE(s);
	s.x /= d;
	s.y /= d;
	s.z /= d;
	
	/* represent any point along this line */
	p.x = (c1 * n1.x + c2 * n2.x);
	p.y = (c1 * n1.y + c2 * n2.y);
	p.z = (c1 * n1.z + c2 * n2.z);

	q.x = p3.x - p.x;
	q.y = p3.y - p.y;
	q.z = p3.z - p.z;

	/* get intersection of this ray with third plane */
	lambda = NORMAL_DOT(n3, q) / NORMAL_DOT(n3, s);

	/* store resulting point */
	dest.x = p.x + lambda * s.x;
	dest.y = p.y + lambda * s.y;
	dest.z = p.z + lambda * s.z;
}
