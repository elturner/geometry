#ifndef NORMAL_H
#define NORMAL_H

/* This class defines normal vectors,
 * and the operations on them.
 *
 * This class inherits the point class
 * since both are effectively vectors,
 * so any operations used on points
 * are also useful on normals.
 */

#include <math.h>
#include "point.h"
#include "parameters.h"

class normal_t: public point_t
{
	/*** functions ***/
	public:
	
	/* constructors (inherited) */
	normal_t() : point_t() {};
	normal_t(double* p) : point_t(p) {};
	normal_t(double x, double y) : point_t(x, y) {};

	/* additional geometry functions */

	/* disp:
	 *
	 * 	Computes displacement between two points.
	 */
	inline void disp(point_t& a, point_t& b)
	{
		int i;

		/* compute each dimension */
		for(i = 0; i < NUM_DIMS; i++)
			this->set(i, b.get(i) - a.get(i));
	};

	/* dir:
	 *
	 * 	Computes the noramlized direction of the
	 * 	displacement between two points.
	 *
	 * 	In other words, this := (b - a) / |b - a|
	 */
	inline void dir(point_t& a, point_t& b)
	{
		/* get displacement and normalize */
		disp(a, b);
		this->normalize();
	};

	/* dot:
	 *
	 * 	Take dot product with another normal.
	 */
	inline double dot(normal_t& other)
	{
		int i;
		double d;

		/* compute over each dimension */
		d = 0;
		for(i = 0; i < NUM_DIMS; i++)
			d += this->get(i) * other.get(i);

		/* return dot product */
		return d;
	};

	/* angle:
	 *
	 * 	Computes the angle between the two
	 * 	normal vectors, traversing from this
	 * 	vector to the argument in a counter-
	 * 	clockwise manner.
	 *
	 * 	Returns angle in [-pi, pi]
	 */
	inline double angle(normal_t& other)
	{
		/* compute the sin of the angle */
		double s = (this->get(0)*other.get(1)
			- this->get(1)*other.get(0));

		/* compute the cos of the angle */
		double c = this->dot(other);

		/* get angle */
		if(c >= 0)
			return asin(s);
		if(s >= 0)
			return acos(c);
		return asin(c) - M_PI/2;	
	}

	/* iszero:
	 *
	 * 	Returns true iff this structure has zero magnitude.
	 */
	inline bool iszero()
	{
		int i;
		for(i = 0; i < NUM_DIMS; i++)
			if(this->get(i) != 0)
				return false;
		return true;
	};

	/* magnitude:
	 *
	 * 	Returns the magnitude of this vector.
	 */
	inline double magnitude()
	{
		return sqrt(this->dot(*this));
	};

	/* normalize:
	 *
	 * 	Reset magnitude to unity.
	 */
	inline void normalize()
	{
		int i;
		double m;

		/* find magnitude */
		m = this->magnitude();
		if(m <= 0)
			return;
		m = 1/m;

		/* normalize */
		for(i = 0; i < NUM_DIMS; i++)
			this->set(i, this->get(i) * m);
	};

	/* weighted_sum:
	 *
	 * 	Sums this normal with the specified one,
	 * 	using the weights specified.  Stores result
	 * 	in this normal.
	 *
	 *	Result will not necessarily have unit magnitude.
	 *
	 * arguments:
	 *
	 * 	myweight -	The weight given to this normal
	 * 	other -		The other normal
	 * 	otherweight -	The weight given to the other normal
	 */
	inline void weighted_sum(double myweight, normal_t& other,
					double otherweight)
	{
		int i;

		/* adjust each dimension */
		for(i = 0; i < NUM_DIMS; i++)
			this->set(i, myweight*this->get(i) 
					+ otherweight*other.get(i));
	};

	/* rotate90:
	 *
	 * 	Rotates this normal by 90 degrees counter-clockwise.
	 */
	inline void rotate90()
	{
		double x = this->get(0);
		this->set(0, -(this->get(1)));
		this->set(1, x);
	};

	/* flip:
	 *
	 * 	Reverses direction of normal.
	 */
	inline void flip()
	{
		int i;

		/* adjust each dimension */
		for(i = 0; i < NUM_DIMS; i++)
			this->set(i, -(this->get(i)));
	};
};

#endif
