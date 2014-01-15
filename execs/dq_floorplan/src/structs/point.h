#ifndef POINT_H
#define POINT_H

/* this file defines the point_t class,
 * which represents a unique location in space
 *
 *
 * For speed purposes, most of these functions are
 * inlined.  I apologize about putting code in the
 * header file.
 */

#include <ostream>
#include <math.h>
#include "parameters.h"

using namespace std;

/* the following classes are defined in this file */
class point_t;

/* the following is the definition of the point_t class */
class point_t
{
	/*** parameters ***/
	private:

	/* the location of the point in space */
	double pos[NUM_DIMS];

	/*** functions ***/
	public:

	/* constructors */
	point_t();
	point_t(double* p);
	point_t(double x, double y);
	~point_t();

	/* accessors */

	/* set:
	 *
	 * 	Sets position of this point.
	 *
	 * arguments:
	 *
	 * 	p -	An array of double denoting coordinates.
	 */
	inline void set(double* p)
	{
		int i;
	
		/* copy position */
		for(i = 0; i < NUM_DIMS; i++)
			this->pos[i] = p[i];
	};

	/* set:
	 *
	 * 	Sets the i'th dimension of the position of this point.
	 */
	inline void set(int i, double x)
	{
		/* check validity of arguments */
		if(i < 0 || i >= NUM_DIMS)
			return;
		this->pos[i] = x;
	};

	/* get:
	 *
	 * 	Gets the i'th dimension of the position.  If i is
	 * 	out of range, returns 0.
	 */
	inline double get(int i) const
	{
		/* check validity of arguments */
		if(i < 0 || i >= NUM_DIMS)
			return 0;
		return this->pos[i];
	};

	/* random:
	 *
	 * 	Resets this point's position to be a sample of a 
	 * 	zero-mean uniform distribution with the specified
	 * 	width.
	 *
	 * arguments:
	 *
	 * 	w -	The point will fall in a box of size [-w,w]
	 */
	void random(double w);

	/* geometry */

	/* dist_sq:
	 *
	 * 	Returns the squared distance (L2) of this point from
	 *	the argument.
	 */
	inline double dist_sq(point_t& other)
	{
		double d, di;
		int i;
	
		/* accumulate L2 distance */
		d = 0;
		for(i = 0; i < NUM_DIMS; i++)
		{
			di = this->pos[i] - other.get(i);
			d += di*di;
		}
	
		/* return the total distance over all dims */
		return d;
	};

	/* dist_L_inf:
	 *
	 * 	Returns the L-infinity distance between this point
	 * 	and the specified point (aka the max dimensional length
	 * 	of the displacement).
	 */
	inline double dist_L_inf(point_t& other)
	{
		int i;
		double d, di;
	
		/* check each dimension for max distance */
		d = -1;
		for(i = 0; i < NUM_DIMS; i++)
		{
			di = fabs(this->pos[i] - other.get(i));
			if(di > d)
				d = di;
		}

		/* return max dim */
		return d;
	};

	/* dist_from_segment:
	 *
	 * 	Given a line segment represented by two end points,
	 * 	will return the distance of this point to the closest
	 * 	position on this segment.
	 */
	double dist_from_segment(point_t& a, point_t& b);

	/* operators */
	
	inline bool operator == (const point_t& rhs) const
	{
		int i;

		/* check each dimension */
		for(i = 0; i < NUM_DIMS; i++)
			if(this->pos[i] != rhs.get(i))
				return false;
		return true;
	};

	inline point_t operator = (const point_t& rhs)
	{
		int i;

		/* copy over position */
		for(i = 0; i < NUM_DIMS; i++)
			this->pos[i] = rhs.get(i);
	
		/* return the value of this point */
		return (*this);
	};

	inline point_t operator += (const point_t& rhs)
	{
		int i;
	
		/* add offset to this point */
		for(i = 0; i < NUM_DIMS; i++)
			this->pos[i] += rhs.get(i);

		/* return the value of this point */
		return (*this);
	};

	inline point_t operator -= (const point_t& rhs)
	{
		int i;

		/* add offset to this point */
		for(i = 0; i < NUM_DIMS; i++)
			this->pos[i] -= rhs.get(i);

		/* return the value of this point */
		return (*this);
	};

	/* debugging */

	/* print:
	 *
	 * 	Prints this point to the specified file stream.
	 *
	 * example:
	 *
	 * 	point_t x();
	 * 	x.print(cout);
	 *
	 * 	yields:
	 *
	 * 		<0, 0>
	 *
	 * 	No newline is printed.
	 */
	void print(ostream& os);
};

#endif
