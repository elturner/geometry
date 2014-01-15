#include "point.h"
#include <iomanip>
#include <ostream>
#include <stdlib.h>
#include <math.h>
#include "../util/parameters.h"

using namespace std;

/* constructors */
point_t::point_t()
{
	int i;
	
	/* default position */
	for(i = 0; i < NUM_DIMS; i++)
		this->pos[i] = 0.0;
}

point_t::point_t(double* p)
{
	/* set position to argument */
	this->set(p);
}

point_t::point_t(double x, double y)
{
	this->pos[0] = x;
	this->pos[1] = y;
}

point_t::~point_t() { /* no processing required */ }

/* accessors */
	
void point_t::random(double w)
{
	int i;
	
	/* set random values */
	for(i = 0; i < NUM_DIMS; i++)
		this->pos[i] = w*(((rand() % 1000000)/1000000.0) - 0.5);
}

/* geometry */

double point_t::dist_from_segment(point_t& a, point_t& b)
{
	int i;
	point_t d, p;
	double m, t, n, ni, dist;

	/* first, get projection onto the line */
	m = 0;
	for(i = 0; i < NUM_DIMS; i++)
	{
		d.pos[i] = b.pos[i] - a.pos[i];
		p.pos[i] = this->pos[i] - a.pos[i];
		m += d.pos[i] * d.pos[i];
	}
	m = sqrt(m);

	/* d is now the displacement of b from a, with length m */
	/* p is now the displacement of this point from a */

	t = 0;
	n = 0;
	for(i = 0; i < NUM_DIMS; i++)
	{	
		d.pos[i] /= m;
		t += d.pos[i] * p.pos[i];
	}

	/* t is now the projection of this point along the direciton
	 * of ray ab (i.e. tangent length) */
	
	for(i = 0; i < NUM_DIMS; i++)
	{
		ni = p.pos[i] - (t*d.pos[i]);
		n += ni*ni;
	}
	n = sqrt(n);

	/* n is now the distance in the orthogonal direction (i.e. 
	 * normal length) */
	
	/* check if projection is within bounds of segment */
	if(t < 0)
		t = -t;
	else if(t >= m)
		t = t-m;
	else
		t = 0;

	/* compute net distance with tangent and normal */
	dist = sqrt(t*t + n*n);
	return dist;
}

/* debugging */
	
void point_t::print(ostream& os)
{
	os << fixed << setprecision(9)
	   << "<" << this->pos[0] << ", " << this->pos[1] << ">";
}
