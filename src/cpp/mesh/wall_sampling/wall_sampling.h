#ifndef WALL_SAMPLING_H
#define WALL_SAMPLING_H

/**
 * @file   wall_sample.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains wall sampling classes
 *
 * @section DESCRIPTION
 *
 * This file contains classes that are used to represent
 * a 2D wall sampling of a scanned environment.  Wall samples
 * are a set of 2D points that dictate estimates of the positions
 * of strong vertical surfaces in the environment (which should
 * represent walls).
 */

#include <string>
#include <map>

/* the following classes are defined in this file */
class wall_sampling_t;
class wall_sample_t;
class wall_sample_info_t;

/**
 * The wall sample class is used to represent a 2D wall sample
 */
class wall_sample_t
{
	/* parameters */
	private:

		/* the discretized position of the wall sample,
		 * based on the dq resolution */
		int xi;
		int yi;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs a default wall sample
		 *
		 * Will construct a wall sample at the origin
		 */
		wall_sample_t();

		/**
		 * Constructs a wall sample from the specified
		 * other wall sample
		 *
		 * @param other   The other wall sample to copy
		 */
		wall_sample_t(const wall_sample_t& other)
			: xi(other.xi), yi(other.yi)
		{};

		/**
		 * Constructs a wall sample at the given discretized
		 * position.
		 *
		 * Given an index in the sampling grid, will initialize
		 * the wall sample structure for that position.
		 *
		 * @param xxi  The x-index of the wall sample
		 * @param yyi  The y-index of the wall sample
		 */
		wall_sample_t(int xxi, int yyi);

		/**
		 * Constructs a wall sample at the given resolution
		 * and position.
		 *
		 * Given the continuous position of the wall sample
		 * to generate, and the resolution of the sampling
		 * process, will initialize this wall sample
		 * structure.
		 *
		 * @param xx   The x-coordinate of the wall sample
		 * @param yy   The y-coordinate of the wall sample
		 * @param res  The resolution of the wall sample
		 */
		wall_sample_t(double xx, double yy, double res);

		/**
		 * Initializes a wall sample at the given discretized
		 * position.
		 *
		 * Given an index in the sampling grid, will initialize
		 * the wall sample structure for that position.
		 *
		 * @param xxi  The x-index of the wall sample
		 * @param yyi  The y-index of the wall sample
		 */
		inline void init(int xxi, int yyi)
		{ 
			this->xi = xxi;
			this->yi = yyi;
		};
		
		/**
		 * Initializes the value of a wall sample based on
		 * a continuous position and a resolution
		 *
		 * Given the continuous position of this wall sample,
		 * and the resolution of wall sampling, will
		 * populate the fields of this structure
		 *
		 * @param xx   The x-coordinate of the wall sample
		 * @param yy   The y-coordinate of the wall sample
		 * @param res  The resolution of the wall sample
		 */
		void init(double xx, double yy, double res);

		/*-----------*/
		/* operators */
		/*-----------*/

		/**
		 * Copies the value from the other wall sample
		 * into this wall sample.
		 *
		 * Given another wall sample, will copy its value
		 * into this wall sample, and returned the modified
		 * structure.
		 *
		 * @param other   The wall sample to copy
		 *
		 * @return    The modified version of this wall sample
		 */
		inline wall_sample_t& operator = (
				const wall_sample_t& other)
		{
			this->xi = other.xi;
			this->yi = other.yi;
			return (*this);
		};

		/**
		 * Checks if two wall samples are equal
		 *
		 * Will check if the given wall sample is equal
		 * to the specified other wall sample
		 *
		 * @param other   The other wall sample to compare to
		 *
		 * @return    Returns true iff both wall samples are equal
		 */
		inline bool operator == (const wall_sample_t& other) const
		{
			return (this->xi == other.xi)
					&& (this->yi == other.yi);
		};

		/**
		 * Checks if this wall sample is less that the other
		 * wall sample.
		 *
		 * Will make a comparison operation between the two
		 * wall samples.
		 *
		 * @param other   The other wall sample to compare to
		 *
		 * @return    Returns true iff this wall sample is
		 *            strictly less than the other wall sample
		 */
		inline bool operator < (const wall_sample_t& other) const
		{
			if(this->xi < other.xi)
				return true;
			if(this->xi > other.xi)
				return false;
			return (this->yi < other.yi);
		};
};

class wall_sample_info_t
{
	/* parameters */
	private:

		/**
		 * The total weight of the wall samples at this location,
		 * based on all samples observed so far.
		 *
		 * You can think of this as the "num_points" field
		 * from previous wall sampling approaches, but allows
		 * for each point to have a different weight.
		 */
		double total_weight;

		/**
		 * The continuous, average 2D position of the wall samples
		 * at this location.
		 */
		double x_avg;
		double y_avg;

		/**
		 * The minimum and maximum height values observed
		 * so far for the wall samples at this location.
		 */
		double z_min;
		double z_max;

		// TODO

	/* functions */
	public:
		
		// TODO
};

#endif
