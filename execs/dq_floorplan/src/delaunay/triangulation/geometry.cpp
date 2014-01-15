#include "geometry.h"
#include "vertex.h"
#include <math.h>

double geom_dist_sq(vertex_t* p, vertex_t* q)
{
	double x, y;

	/* displacement */
	x = p->pos[VERTEX_X_IND] - q->pos[VERTEX_X_IND];
	y = p->pos[VERTEX_Y_IND] - q->pos[VERTEX_Y_IND];

	/* magnitude */
	return (x*x + y*y);
}

double geom_triangle_area(vertex_t* p, vertex_t* q, vertex_t* r)
{
	double ux, uy, vx, vy;

	/* get vectors */
	ux = p->pos[0] - r->pos[0];
	uy = p->pos[1] - r->pos[1];
	vx = q->pos[0] - r->pos[0];
	vy = q->pos[1] - r->pos[1];

	/* compute half of cross product */
	return (ux*vy - uy*vx)/2;
}

int geom_line_intersect(vertex_t* v0, vertex_t* v1, 
				vertex_t* w0, vertex_t* w1, vertex_t* x)
{
	int v_vert, w_vert;
	double v_slope, w_slope, v_y_off, w_y_off, q;
	vertex_t intersect;

	/* check arguments */
	if(!v0 || !v1 || !w0 || !w1 || v0 == v1 || w0 == w1)
		return -1;

	/* check edge cases */
	v_vert = (v0->pos[VERTEX_X_IND] == v1->pos[VERTEX_X_IND]);
	w_vert = (w0->pos[VERTEX_X_IND] == w1->pos[VERTEX_X_IND]);

	if(v_vert && w_vert)
		return 0; /* lines parallel */
	else if(v_vert)
	{
		/* line v is vertical, but w is not. Get intersection
		 * point by finding w(v_x) */
		w_slope = (w1->pos[VERTEX_Y_IND] - w0->pos[VERTEX_Y_IND])
			/ (w1->pos[VERTEX_X_IND] - w0->pos[VERTEX_X_IND]);
		w_y_off = w0->pos[VERTEX_Y_IND] 
					- w_slope*(w0->pos[VERTEX_X_IND]);
		
		/* get intersect of lines */
		intersect.pos[VERTEX_X_IND] = v0->pos[VERTEX_X_IND];
		intersect.pos[VERTEX_Y_IND] = w_slope 
					* v0->pos[VERTEX_X_IND] + w_y_off;
	}
	else if(w_vert)
	{
		/* line w is vertical, but v is not.  Get intersection
		 * point by finding v(w_x) */
		v_slope = (v1->pos[VERTEX_Y_IND] - v0->pos[VERTEX_Y_IND])
			/ (v1->pos[VERTEX_X_IND] - v0->pos[VERTEX_X_IND]);
		v_y_off = v0->pos[VERTEX_Y_IND] 
					- v_slope*(v0->pos[VERTEX_X_IND]);
		
		/* get intersect of lines */
		intersect.pos[VERTEX_X_IND] = w0->pos[VERTEX_X_IND];
		intersect.pos[VERTEX_Y_IND] = v_slope 
					* w0->pos[VERTEX_X_IND] + v_y_off;
	}
	else
	{
		/* get slopes and y-intercepts of lines */
		v_slope = (v1->pos[VERTEX_Y_IND] - v0->pos[VERTEX_Y_IND])
			/ (v1->pos[VERTEX_X_IND] - v0->pos[VERTEX_X_IND]);
		w_slope = (w1->pos[VERTEX_Y_IND] - w0->pos[VERTEX_Y_IND])
			/ (w1->pos[VERTEX_X_IND] - w0->pos[VERTEX_X_IND]);
	
		v_y_off = v0->pos[VERTEX_Y_IND] 
					- v_slope*(v0->pos[VERTEX_X_IND]);
		w_y_off = w0->pos[VERTEX_Y_IND] 
					- w_slope*(w0->pos[VERTEX_X_IND]);

		/* check to make sure lines are not parallel */
		if(v_slope == w_slope)
			return 0; /* lines parallel */

		/* get intersection point of full lines */
		intersect.pos[VERTEX_X_IND] = (w_y_off - v_y_off) 
						/ (v_slope - w_slope);
		intersect.pos[VERTEX_Y_IND] = v_slope 
				* intersect.pos[VERTEX_X_IND] + v_y_off;
	}

	/* determine if intersection point is within segment v */
	if(v0->pos[VERTEX_X_IND] < v1->pos[VERTEX_X_IND])
	{
		q = intersect.pos[VERTEX_X_IND];
		if(q < v0->pos[VERTEX_X_IND] || q > v1->pos[VERTEX_X_IND])
			return 0; /* outside segment v */
	}
	else
	{
		q = intersect.pos[VERTEX_X_IND];
		if(q > v0->pos[VERTEX_X_IND] || q < v1->pos[VERTEX_X_IND])
			return 0; /* outside segment v */
	}

	/* determine if intersection point is within segment w */
	if(w0->pos[VERTEX_X_IND] < w1->pos[VERTEX_X_IND])
	{
		q = intersect.pos[VERTEX_X_IND];
		if(q < w0->pos[VERTEX_X_IND] || q > w1->pos[VERTEX_X_IND])
			return 0; /* outside segment w */
	}
	else
	{
		q = intersect.pos[VERTEX_X_IND];
		if(q > w0->pos[VERTEX_X_IND] || q < w1->pos[VERTEX_X_IND])
			return 0; /* outside segment w */
	}

	/* optionally store intersection point */
	if(x)
	{
		if(vertex_copy(x, &intersect))
			return -2;
	}

	/* intersection found */
	return 1;
}

double geom_orient2D(vertex_t* p, vertex_t* q, vertex_t* r)
{
	double px, py, qx, qy, rx, ry;

	/* check arguments */
	if(!p || !q || !r)
		return 0;

	/* compute determinant of matrix:
	 *
	 *	(px-rx)		(py-ry)
	 *
	 *	(qx-rx)		(qy-ry)
	 */
	px = p->pos[VERTEX_X_IND];
	py = p->pos[VERTEX_Y_IND];

	qx = q->pos[VERTEX_X_IND];
	qy = q->pos[VERTEX_Y_IND];
	
	rx = r->pos[VERTEX_X_IND];
	ry = r->pos[VERTEX_Y_IND];

	/* return result */
	return (px-rx)*(qy-ry) - (py-ry)*(qx-rx);
}

double geom_incircle(vertex_t* p, vertex_t* q, vertex_t* r, vertex_t* s)
{
	double sx, sy, psx, qsx, rsx, psy, qsy, rsy, pss, qss, rss;

	/* check arguments */
	if(!p || !q || !r || !s)
		return 0;

	/* compute determinant of the matrix:
	 *
	 *	(px-sx)		(py-sy)		((px-sx)^2 + (py-sy)^2)
	 *
	 *	(qx-sx)		(qy-sy)		((qx-sx)^2 + (qy-sy)^2)
	 *	
	 *	(rx-sx)		(ry-sy)		((rx-sx)^2 + (ry-sy)^2)
	 */
	
	sx = s->pos[VERTEX_X_IND];
	sy = s->pos[VERTEX_Y_IND];

	psx = p->pos[VERTEX_X_IND] - sx;
	psy = p->pos[VERTEX_Y_IND] - sy;

	qsx = q->pos[VERTEX_X_IND] - sx;
	qsy = q->pos[VERTEX_Y_IND] - sy;
	
	rsx = r->pos[VERTEX_X_IND] - sx;
	rsy = r->pos[VERTEX_Y_IND] - sy;

	pss = psx*psx + psy*psy;
	qss = qsx*qsx + qsy*qsy;
	rss = rsx*rsx + rsy*rsy;

	/* return result */
	return psx * (qsy*rss - qss*rsy)
		- psy * (qsx*rss - qss*rsx)
		+ pss * (qsx*rsy - qsy*rsx);
}

int geom_intriangle(vertex_t* p, vertex_t* q, vertex_t* r, vertex_t* s)
{
	double opq, oqr, orp;

	/* orient each edge */
	opq = geom_orient2D(p,q,s);
	oqr = geom_orient2D(q,r,s);
	orp = geom_orient2D(r,p,s);
	
	/* check if any positions are null (which signifies ghost vertex) */
	if(!p)
		return (oqr >= 0);
	else if(!q)
		return (orp >= 0);
	else if(!r)
		return (opq >= 0);

	/* Use the orient2D function on each edge of the triangle.
	 * If s is on the left of each edge, then s must be inside
	 * the triangle. */
	return (opq >= 0) && (oqr >= 0) && (orp >= 0);
}

int geom_ontriangleedge(vertex_t* p, vertex_t* q, vertex_t* r,
							vertex_t* s)
{
	/* check arguments */
	if(!s)
		return -1;

	/* check each edge */
	if(geom_inline(p,s,q))
		return 2;
	if(geom_inline(q,s,r))
		return 0;
	if(geom_inline(r,s,p))
		return 1;

	/* not on any edge */
	return 3;
}

int geom_center(vertex_t* p, vertex_t* q, vertex_t* r, vertex_t* s)
{
	int c;

	/* check arguments */
	if(!s)
		return -1;

	/* init counter */
	c = 0;
	s->pos[VERTEX_X_IND] = 0;
	s->pos[VERTEX_Y_IND] = 0;

	/* average points */
	if(p)
	{
		s->pos[VERTEX_X_IND] += p->pos[VERTEX_X_IND];
		s->pos[VERTEX_Y_IND] += p->pos[VERTEX_Y_IND];
		c++;
	}
	
	if(q)
	{
		s->pos[VERTEX_X_IND] += q->pos[VERTEX_X_IND];
		s->pos[VERTEX_Y_IND] += q->pos[VERTEX_Y_IND];
		c++;
	}
	
	if(r)
	{
		s->pos[VERTEX_X_IND] += r->pos[VERTEX_X_IND];
		s->pos[VERTEX_Y_IND] += r->pos[VERTEX_Y_IND];
		c++;
	}

	/* divide the sum */
	if(!c)
		return -2;

	s->pos[VERTEX_X_IND] /= c;
	s->pos[VERTEX_Y_IND] /= c;
	return 0;
}

int geom_inline(vertex_t* p, vertex_t* q, vertex_t* r)
{
	double qx, qy, pqx, pqy, rqx, rqy;

	/* check arguments */
	if(!p || !q || !r)
		return 0;

	/* shift q to the origin */
	qx = q->pos[VERTEX_X_IND];
	qy = q->pos[VERTEX_Y_IND];
	pqx = p->pos[VERTEX_X_IND] - qx;
	pqy = p->pos[VERTEX_Y_IND] - qy;
	rqx = r->pos[VERTEX_X_IND] - qx;
	rqy = r->pos[VERTEX_Y_IND] - qy;

	/* compute euclidian inner product */
	if((pqx*rqx + pqy*rqy) >= 0)
		return 0; /* q is not between p and r */

	/* now check if they're colinear */

	/* compute determinant of matrix:
	 *
	 *	(rx-qx)		(ry-qy)
	 *
	 *	(px-qx)		(py-qy)
	 */
	return ((rqx*pqy - rqy*pqx) == 0);
}

double geom_circumcenter(vertex_t* p, vertex_t* q, vertex_t* r, 
						vertex_t* s)
{
	double x1, y1, x2, y2, x3, y3, a, b;

	/* check arguments */
	if(!p || !q || !r)
		return -1;

	/* get variables */
	x1 = p->pos[VERTEX_X_IND];
	y1 = p->pos[VERTEX_Y_IND];
	x2 = q->pos[VERTEX_X_IND];
	y2 = q->pos[VERTEX_Y_IND];
	x3 = r->pos[VERTEX_X_IND];
	y3 = r->pos[VERTEX_Y_IND];

	/* compute circle */
	a = (x2*x2 - x1*x1 + y2*y2 - y1*y1)*(y3-y1)
		- (x3*x3 - x1*x1 + y3*y3 - y1*y1)*(y2-y1);
	a = a / (2 * ( (x2-x1)*(y3-y1) - (y2-y1)*(x3-x1) ) );
	b = (y2*y2 - y1*y1 + x2*x2 - x1*x1)*(x3 - x1)
		- (y3*y3 - y1*y1 + x3*x3 - x1*x1)*(x2-x1);
	b = b / ( 2 * ( (y2-y1)*(x3-x1) - (x2-x1)*(y3-y1) ) );

	/* check if s is defined */
	if(s)
	{
		s->pos[VERTEX_X_IND] = a;
		s->pos[VERTEX_Y_IND] = b;
	}

	/* success */
	return sqrt((x1 - a)*(x1 - a) + (y1 - b)*(y1 - b));
}
