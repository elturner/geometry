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

#include <iostream>
#include <string>
#include <set>
#include <map>

/* the following classes are defined in this file */
class wall_sample_t;
class wall_sample_info_t;
class wall_sampling_t;

/* the following typedefs are used in these classes */
typedef std::map<wall_sample_t, wall_sample_info_t> wall_sample_map_t;

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
		 * @param res  The resolution of the wall samples
		 * @param cx   The x-coordinate of the center of the map
		 * @param cy   The y-coordinate of the center of the map
		 */
		wall_sample_t(double xx, double yy,
			double res, double cx, double cy);

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
		 * @param cx   The x-coordinate of the center of the map
		 * @param cy   The y-coordinate of the center of the map
		 */
		void init(double xx, double yy,
				double res, double cx, double cy);

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
	/* security */
	friend class wall_sampling_t;

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

		/**
		 * The normal vector for this sample
		 *
		 * The values given will not necessarily be normalized
		 */
		double x_norm;
		double y_norm;

		/**
		 * The list of poses for this sample
		 *
		 * The poses are represented by indices that reference
		 * the original path file for the system trajectory.
		 * Each sample can be seen by a subset of all poses,
		 * which is designated here.
		 */
		std::set<size_t> poses;

	/* functions */
	public:

		/**
		 * Constructs empty info structure
		 */
		wall_sample_info_t();

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Clears values from this info structure
		 */
		void clear();

		/**
		 * Adds an xy sample to this structure with the given weight
		 *
		 * @param x   The x-component to add
		 * @param y   The y-component to add
		 * @param nx   The normal x-component to add
		 * @param ny   The normal y-component to add
		 * @param w   The weight to add
		 */
		void add(double x,double y, double nx,double ny, double w);

		/**
		 * Adds a z-range to this structure
		 *
		 * @param z0  The min z value of the range to add
		 * @param z1  The max z value of the range to add
		 */
		void add_zs(double z0, double z1);

		/**
		 * Adds a pose to this structure
		 *
		 * @param ind   The pose index to add
		 */
		inline void add_pose(size_t ind)
		{ this->poses.insert(ind); };

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Get the weight for this wall sample
		 *
		 * @return   Returns the total weight for the wall sample
		 */
		inline double get_weight() const
		{ return this->total_weight; };

		/**
		 * Get the normal vector for this wall sample
		 *
		 * The normal vector will be normalized before being
		 * returned.
		 *
		 * @param nx   Where to store the x-component
		 * @param ny   Where to store the y-component
		 */
		void get_normal(double& nx, double& ny) const;

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports this info to a line in the body of a DQ
		 * file.
		 *
		 * A Dynamic Quadree (DQ) file represents a list of
		 * wall samples in ascii-format.  This function will
		 * write a single wall sample to the file stream.
		 *
		 * @param os   The output stream to write to
		 */
		void writedq(std::ostream& os) const;
};

/**
 * The wall sampling class represents a set of wall samples
 */
class wall_sampling_t
{
	/* parameters */
	private:

		/**
		 * The wall samples are stored in a map
		 *
		 * Each wall sample associates a position in
		 * space with the properties of the samples in
		 * that position.
		 *
		 * The samples are divided into a 2D grid along
		 * the XY plane, so each sample can be looked up
		 * by referencing the index of its appropriate
		 * grid cell.
		 */
		wall_sample_map_t samples;

		/**
		 * This value represents the bounding area of the
		 * samples.
		 *
		 * The wall samples can be stored in a quadtree, which
		 * requires us to keep track of the center position
		 * and halfwidth of the tree's root, as well as the
		 * resolution of each sample.
		 *
		 * Note that as samples get added to this map,
		 * the halfwidth will be updated appropriately.  The
		 * halfwidth can also be set at the outset to a larger
		 * value.
		 *
		 * units: meters
		 */
		double halfwidth;

		/**
		 * The x-coordinate of the center of the environment
		 *
		 * units: meters
		 */
		double center_x;

		/**
		 * The y-coordinate of the center of the environment
		 *
		 * units: meters
		 */
		double center_y;

		/**
		 * This represents the size of each grid cell
		 *
		 * Each sample is stored in a grid cell at a particular
		 * resolution.  The resolution denotes the width of
		 * each cell.
		 *
		 * units: meters
		 */
		double resolution;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs an empty sampling with default resolution,
		 * centered at the origin.
		 */
		wall_sampling_t();

		/**
		 * Constructs an empty sampling with the specified
		 * resolution, centered at the origin.
		 *
		 * @param res   The resolution to use for each gridcell
		 */
		wall_sampling_t(double res);

		/**
		 * Constructs an empty sampling with the specified
		 * resolution and center.
		 *
		 * @param res   The resolution to use for each gridcell
		 * @param x     The x-coordinate of the center of the map
		 * @param y     The y-coordinate of the center of the map
		 */
		wall_sampling_t(double res, double x, double y);

		/**
		 * Constructs an empty sampling with the specified
		 * resolution, center, and halfwidth.
		 *
		 * @param res   The resolution to use for each gridcell
		 * @param x     The x-coordinate of the center of the map
		 * @param y     The y-coordinate of the center of the map
		 * @param hw    The halfwidth of the map
		 */
		wall_sampling_t(double res, double x, double y, double hw);

		/**
		 * Initializes this map with the specified values.
		 *
		 * Note that this call will clear all wall samples
		 * from the map as well as change the parameters.
		 *
		 * @param res   The resolution to use for each gridcell
		 * @param x     The x-coordinate of the center of the map
		 * @param y     The y-coordinate of the center of the map
		 * @param hw    The halfwidth of the map
		 */
		void init(double res, double x, double y, double hw);

		/*-----------*/
		/* modifiers */
		/*-----------*/

		/**
		 * Clears all samples from this map
		 */
		inline void clear()
		{ this->samples.clear(); };

		/**
		 * Sets the halfwidth of this map
		 *
		 * Note that the halfwidth will be automatically updated
		 * with each wall sample insertion, but this function allows
		 * the user to set the halfwidth from the offset.
		 *
		 * @param hw   The halfwidth to use.
		 */
		void set_halfwidth(double hw);
		
		/**
		 * Adds a wall sample to this map
		 *
		 * A wall sample is represented by a continuous
		 * point in 2D space and a weighting.  By default,
		 * each sample is weighted equally, but samples can
		 * be weighted differently based on prior knowledge.
		 *
		 * For each sample, we can also add a range of z-values,
		 * which will be incorporated in the sample position's
		 * total z-range.
		 *
		 * @param x      The x-position of the sample to add
		 * @param y      The y-position of the sample to add
		 * @param nx     The x-component of the normal vector 
		 *               of this sample
		 * @param ny     The y-component of the normal vector
		 *               of this sample
		 * @param z_min  The minimum z-position of this sample
		 * @param z_max  The maximum z-position of this sample
		 * @param w      The weight of this sample
		 *
		 * @return    Returns the wall sample that was made/edited.
		 */
		wall_sample_t add(double x, double y, double nx, double ny, 
				double z_min, double z_max, double w=1.0);

		/**
		 * Adds pose information to the specified sample
		 *
		 * Will add the specified pose index to the sample
		 * located at the given position.  Note that adding
		 * a pose index indicates that a sensor from that
		 * particular system pose has line-of-sight to the
		 * given sample.
		 *
		 * @param x      The x-position of the sample to modify
		 * @param y      The y-position of the sample to modify
		 * @param ind    The pose index to add to this sample
		 *
		 * @return    Returns the wall sample that was made/edited.
		 */
		 wall_sample_t add(double x, double y, size_t ind);

		/**
		 * Adds pose information to the specified sample
		 *
		 * Will add the specified pose index to the sample
		 * located at the given position.  Note that adding
		 * a pose index indicates that a sensor from that
		 * particular system pose has line-of-sight to the
		 * given sample.
		 *
		 * @param ws     The wall sample to modify
		 * @param ind    The pose index to add to this sample
		 */
		 void add(const wall_sample_t& ws, size_t ind);

		/**
		 * Removes all wall samples that have no associated poses
		 *
		 * Will iterate through the wall samples.  If any have
		 * no pose information, will remove them.
		 */
		 void remove_without_pose();

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Retrieves an iterator to the specified wall sample
		 *
		 * If not found, will return this->end().
		 *
		 * @param ws  The wall sample to find
		 *
		 * @return   Returns iterator to given wall sample info
		 */
		inline wall_sample_map_t::const_iterator 
			 	find(const wall_sample_t& ws) const
		{ return this->samples.find(ws); };

		/**
		 * Retrieves an iterator to the wall sample at the
		 * specified location.
		 *
		 * If not found, will return this->end()
		 *
		 * @param x   The x-coordinate of the location to check
		 * @param y   The y-coordinate of the location to check
		 *
		 * @return    Returns iterator to wall sample info at (x,y)
		 */
		wall_sample_map_t::const_iterator
					find(double x, double y) const;

		/**
		 * Retrieves iterator to the first wall sample
		 *
		 * @return   Returns the beginning iterator
		 */
		inline wall_sample_map_t::const_iterator begin() const
		{ return this->samples.begin(); };

		/**
		 * Retrieves iterator to past the end of wall samples
		 *
		 * @return   Returns the end iterator
		 */
		 inline wall_sample_map_t::const_iterator end() const
		 { return this->samples.end(); };

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Exports these samples to a dq file
		 *
		 * Will export the values stored in this sample
		 * map to the specified .dq file.  DQ files are
		 * ascii-formatted files that represent the
		 * wall samples in the form of a Dynamic Quadtree (DQ).
		 *
		 * @param filename   Where to write the file
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int writedq(const std::string& filename) const;

	/* helper functions */
	private:

		/**
		 * Computes the maximum depth required to represent
		 * these samples in a dynamic quadtree.
		 *
		 * @return   Returns the max depth of a quadree for
		 *           these samples.
		 */
		size_t get_max_depth() const;
};

#endif
