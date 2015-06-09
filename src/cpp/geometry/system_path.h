#ifndef SYSTEM_PATH_H
#define SYSTEM_PATH_H

/**
 * @file   system_path.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file contains structs and functions
 * used to manipulate poses of the scan acquisition
 * system.
 *
 * Typically the path information is retrieved from a *.mad
 * file (which is an outdated path format).
 *
 * The system transformations are read in from a hardware
 * xml configuration file, which allows conversion from
 * system common to the location of a specific sensor at
 * each timestep.
 */

#include <string>
#include <map>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <geometry/transform.h>
#include <util/range_list.h>

/* the following classes are defined in this file */
class system_path_t;
class pose_t;

/**
 * This class represents a complete system_path: a list of poses
 *
 * Using this list of poses, this classes represents a continuous
 * mapping between timestamps and the 6-degree-of-freedom pose of
 * the system.  From this mapping, the pose of a specific sensor
 * can be determined at any arbitrary timestamp within the
 * duration of a dataset.
 */
class system_path_t
{
	/*** parameters ***/
	protected:

		/* this pose list represents the poses in the system_path,
		 * in chronological order (dynamically allocated). */
		pose_t* pl;
		unsigned int pl_size; /* number of poses */

		/* the following map specifies the transforms from
		 * sensor coordinate systems to system common coordinates */
		std::map<std::string, transform_t*> transform_map;

		/* the following represents a blacklist of timestamps
		 * to ignore. If the path is imported from a .mad file,
		 * this list will contain the imported Zupt intervals */
		range_list_t timestamp_blacklist;

	/*** functions ***/
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/

		/**
		 * Constructs empty system path
		 */
		system_path_t();
		
		/**
		 * Frees all memory and resources
		 */
		~system_path_t();

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Reads the input file as a path file
		 *
		 * Determines the format of the specified file
		 * and attempts to read it.
		 *
		 * @param pathfile   The path to the file to read
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int read(const std::string& pathfile);
		
		/**
		 * Reads *.mad file and stores results.
		 *
		 * This function will destroy any pose information 
		 * currently in this object.
		 *
		 * @param filename    Local path to file to parse.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int readmad(const std::string& filename);

		/**
		 * Exports path to *.mad file on disk
		 *
		 * This function will export the path information
		 * that is stored in this object to the specified
		 * *.mad file on disk.
		 * 
		 * @param filename    Where to store the mad file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int writemad(const std::string& filename) const;

		/**
		 * Reads *.noisypath file and stores results.
		 *
		 * This function will destroy any pose information
		 * currently in this object.
		 *
		 * @param filename   The path to the .noisypath file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int readnoisypath(const std::string& filename);

		/**
		 * Reads in hardware transformations for each sensor
		 *
		 * Will parse the given xml hardware configuration file,
		 * and store the transformations for each sensor
		 * in this object.
		 *
		 * @param  xml     The hardware config xml file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int parse_hardware_config(const std::string& xml);

		/*-----------*/
		/* modifiers */
		/*-----------*/
		
		/**
		 * Clears all information in this structure.
		 *
		 * Resets the value of this structure to the state before
		 * any pose information was read.
		 */
		void clear();

		/**
		 * Applies a rigid transform to the entire path
		 *
		 * Will modify all poses in the path so reflect this
		 * new transform. For each pose, the new position will
		 * be:
		 *
		 * 	T_new = R * T_old + T
		 * 
		 * and the rotation of the pose will be:
		 *
		 * 	R_new = R * R_old
		 *
		 * @param R   The rotation to apply to each pose
		 * @param T   The translation to apply to each pose
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int apply_transform(const Eigen::Quaternion<double>& R,
					const Eigen::Vector3d& T);

		/*-----------*/
		/* accessors */
		/*-----------*/

		/**
		 * Generates the interpolated pose for specified timestamp.
		 *
		 * Given a timestamp, will compute the interpolate
		 * pose of the system for that time, based on Spherical 
		 * Linear intERPolation (SLERP).  See:
		 *
		 * 	http://en.wikipedia.org/wiki/Slerp
		 *
		 * @param p   Where to store the output pose
		 * @param t   The timestamp in question
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int compute_pose_at(pose_t& p, double t) const;

		/**
		 * Retrieves the rigid transform for the given sensor
		 *
		 * Will retrieve the sensor->system transform for
		 * the sensor with the specified name.
		 *
		 * @param t   Where to store the transform for this sensor
		 * @param s   The name of the sensor to retrieve
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int get_extrinsics_for(transform_t& t,
		                       const std::string& s) const;

		/**
		 * Computes transform of specified sensor at given timestamp
		 *
		 * Given a sensor and a timestamp, will generate an
		 * interpolated transform of the sensor's position and
		 * orientation at that time from sensor coordinates to
		 * world coordinates.  This is done by finding
		 * the interpolated pose of the system using
		 * compute_pose_at(), then pre-applying the sensor->system
		 * transform.
		 *
		 * @param p   Where to store the output transform
		 * @param t   The timestamp of the pose to compute
		 * @param s   The name of the sensor to use
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int compute_transform_for(transform_t& p, double t,
		                     const std::string& s) const;

		/**
		 * Checks if the specified timestamp is blacklisted
		 *
		 * Will check the specified timestamp against any imported
		 * blacklists (such as the zupt list in a .mad file), and
		 * will return true iff this timestamp falls within the
		 * blacklist.
		 *
		 * @param ts   The timestamp to check
		 *
		 * @return     Returns true iff ts is blacklisted.
		 */
		bool is_blacklisted(double ts) const;

		/**
		 * Retrieves stored zupt information
		 *
		 * Zupts (or Zero-velocity interrUPTS) specify when the
		 * system was not moving or when the information from
		 * the system path should be ignored.
		 *
		 * This function populates a list of pairs dictating the
		 * start and end times for each zupt.
		 *
		 * @param zupts    Where to store zupt info.
		 */
		inline void get_zupts(std::vector<
				std::pair<double, double> >& zupts) const
		{
			/* get zupts from black-list times */
			this->timestamp_blacklist.get_ranges(zupts);
		};

		/**
		 * Will return the timestamp of the first pose
		 */
		double starttime() const;

		/**
		 * Will return the timestamp of the last pose
		 */
		double endtime() const;

		/**
		 * Will return the total distance traveled along path
		 *
		 * The returned value will be in units of meters.
		 */
		double total_distance() const;

		/**
		 * Returns the total number of poses imported
		 */
		inline size_t num_poses() const
		{ return this->pl_size; };

		/**
		 * Retrieves the raw pose information for the i'th pose
		 *
		 * @param i   The pose index to retrieve
		 *
		 * @return    Returns a pointer to the i'th pose, or NULL
		 *            if i is not a valid index
		 */
		pose_t* get_pose(size_t i) const;

		/**
		 * Retrieves the beginning iterator to the map of transforms
		 *
		 * All transforms for the various sensors on the backpack
		 * are stored in a map in this structure.  This call will
		 * return the iterator to the first element in this map.
		 *
		 * The iterators reference a pair for each transform:
		 *
		 * 	string:                name of sensor
		 * 	transform_t pointer:   The transform from sensor
		 * 	                       to backpack center
		 *
		 * @return    The beginning iterator to the transforms.
		 */
		inline std::map<std::string, transform_t*>::const_iterator
				begin_transforms() const
		{ return this->transform_map.begin(); };

		/**
		 * Retrieves the ending iterator to the map of transforms
		 *
		 * All transforms for the various sensors on the backpack
		 * are stored in a map in this structure.  This call will
		 * return the iterator to the first element in this map.
		 *
		 * The iterators reference a pair for each transform:
		 *
		 * 	string:                name of sensor
		 * 	transform_t pointer:   The transform from sensor
		 * 	                       to backpack center
		 *
		 * @return    The beginning iterator to the transforms.
		 */
		inline std::map<std::string, transform_t*>::const_iterator
				end_transforms() const
		{ return this->transform_map.end(); };

	/* helper functions */
	public:

		/**
		 * Computes index of last pose at or before time t.
		 *
		 * Returns the index of the latest pose in pl
		 * that occurred at or before the specified time t.
		 *
		 * @param t  The timstamp to use to look-up pose
		 *
		 * @return   On success, returns non-negative index of 
		 * a pose.   On failure, returns negative value.
		 */
		int closest_index(double t) const;
};

/**
 * Represents a single pose: a location and orientation in space-time.
 */
class pose_t
{
	/* the path can modify poses */
	friend class system_path_t;

	/*** parameters ***/
	public:

		/**
		 * The timestamp associated with this pose.
		 *
		 * For a single pose, we are interested in both temporal
		 * and spatial positions.  This value is represented in
		 * units of seconds, in the synchronized system clock.
		 */
		double timestamp;

		/**
		 * Translation vector for this pose.
		 *
		 * Position of system common specified in ENU coordinates
		 * in units of meters.
		 */
		Eigen::Vector3d T;

		/**
		 * Covariance matrix for translation of pose
		 *
		 * If uncertainty values for the pose are provided, the
		 * covariance matrix of the fitted distribution of the
		 * translation is stored here.  If no such values are
		 * provided, then this matrix is Zero.
		 */
		Eigen::Matrix3d T_cov;

		/**
		 * Specifies the orientation of the system.
		 *
		 * A rotation from system coordinates to 
		 * world coordinates */
		Eigen::Quaternion<double> R;

		/**
		 * Covariance matrix for the rotation of pose
		 *
		 * If uncertainty values for the pose are provided,
		 * the covariance matrix of the rotation angles
		 * (roll, pitch, yaw) is stored here.  If no such
		 * values are provided, then this matrix is Zero.
		 */
		Eigen::Matrix3d R_cov;

		/**
		 * The linear velocity of the system at this pose.
		 * 
		 * This value is represented in WORLD COORDINATES, in
		 * units of meters / second.
		 */
		Eigen::Vector3d v;

		/** 
		 * The angular velocity of the system at this pose.
		 *
		 * This value is represented in SYSTEM COORDINATES, in
		 * units of radians/second.
		 */
		Eigen::Vector3d w;

	/*** functions ***/
	public:
	
		/* the pose list is an array that contains eigen 
		 * constructions, so it must be properly aligned */
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		/* constructors */
		
		/**
		 * Constructs pose at origin with default orientation
		 */
		pose_t();

		/**
		 * Clears all memory and resources
		 */
		~pose_t();

		/* operators */

		inline pose_t operator = (const pose_t& rhs)
		{
			/* copy params */
			this->timestamp = rhs.timestamp;
			this->T         = rhs.T;
			this->T_cov     = rhs.T_cov;
			this->R         = rhs.R;
			this->R_cov     = rhs.R_cov;
			this->v         = rhs.v;
			this->w         = rhs.w;

			/* return the value of this point */
			return (*this);
		};

		/* geometry */

		/**
		 * Computes the squared-distance between two poses
		 *
		 * Compues the squared-distance between this pose
		 * and the argument pose.  Only spatial translation
		 * is considered, not timestamps or rotations values.
		 *
		 * @param other   The other pose to analyze
		 *
		 * @return   Returns the square of the distance (spatial).
		 */
		double dist_sq(const pose_t& other) const;

		/* i/o */

		/**
		 * Prints the pose information for debugging purposes.
		 *
		 * Pose information is printed to stdout as a space-
		 * separated list of values <time> <T> <R>
		 */
		void print() const;

	/* helper functions */
	private:

		/**
		 * Computes rotation quaternion from NED roll, pitch, yaw
		 * 
		 * Assumes roll, pitch, yaw are in NED coordinates
		 * in units of radians.  The resulting quaternion will
		 * be stored in this->R.
		 *
		 * @param roll   The roll value  (rotation about +x)
		 * @param pitch  The pitch value (rotation about +y)
		 * @param yaw    The yaw value   (rotation about +z)
		 */
		void compute_transform_NED(double roll, double pitch,
		                           double yaw);

		/**
		 * Computes rotation quaternion from ENU roll, pitch, yaw
		 *
		 * Assumes roll, pitch, yaw are in ENU coordinates in
		 * units of radians.  The resulting quaternion will
		 * be stored in this->R.
		 *
		 * @param roll   The roll value  (rotation about +x)
		 * @param pitch  The pitch value (rotation about +y)
		 * @param yaw    The yaw value   (rotation about +z)
		 */
		void compute_transform_ENU(double roll, double pitch,
		                           double yaw);

		/**
		 * Computes the linear and angular velocity at this pose
		 *
		 * The velocity values will be stored in the corresponding
		 * fields in this object. The velocity is assumed to be 
		 * constant between poses.  The velocity of the system 
		 * at pose #i is the same as the velocity at poses [i,i+1).
		 *
		 * As such, to find the delta position, the successor pose
		 * must be given to compute velocities.
		 *
		 * @param next_pose  The next pose in path after this one
		 */
		void compute_velocity(const pose_t& next_pose);

};

#endif
