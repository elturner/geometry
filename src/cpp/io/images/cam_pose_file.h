#ifndef CAM_POSE_FILE_H
#define CAM_POSE_FILE_H

/**
 * @file    cam_pose_file.h
 * @author  Nick Corso, edited by Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   I/O functionality for camera pose files
 *
 * @section DESCRIPTION
 *
 * Has a representation of camera pose files.  Contains the cam_pose_file_t
 * class, which can read camera pose files.
 */

#include <string>
#include <vector>
#include <util/binary_search.h>

/**
 * Camera pose file class 
 */
class cam_pose_file_t
{
	/* helper classes */
	public:

		/**
		 * Class to store the Poses 
		 *
		 * Each instance of this class contains
		 * a single sensor pose.
		 */
		class Pose
		{
			/* parameters */
			private:
				
				/* store the time and 6DOF pose */
				double _time;
				double _p[6];

			/* functions */
			public:
	
				/* Constructor */
				Pose(double timestamp,
					double roll,
					double pitch,
					double yaw,
					double x,
					double y,
					double z)
				{
					_time = timestamp;
					_p[0] = roll;
					_p[1] = pitch;
					_p[2] = yaw;
					_p[3] = x;
					_p[4] = y;
					_p[5] = z;
				}
			
				/* getters for the data */
				inline const double * p() const 
				{return _p;};
				
				inline double time() const 
				{return _time;};
				
				inline double roll() const 
				{return _p[0];};
				
				inline double pitch() const 
				{return _p[1];};
				
				inline double yaw() const 
				{return _p[2];};
				
				inline double x() const
				{return _p[3];};
				
				inline double y() const
				{return _p[4];};
				
				inline double z() const
				{return _p[5];};
		};
	
	/* parameters */
	private:

		/**
		 * Holds the timestamps
		 */
		std::vector<double> _timestamps;

		/**
		 * Holds the poses
		 */
		std::vector<Pose> _poses;
	
	/* functions */
	public:

		/* constructor */
		cam_pose_file_t() {};
		cam_pose_file_t(const std::string& filename)
		{ read(filename); };

		/* Reader */
		bool read(const std::string& filename);
	
		/* inline functions for accessing certain things */
	
		/**
		 * Retrieves number of poses stored in file
		 */
		inline size_t num_poses() const 
		{return _poses.size();};
		
		/**
		 * Retrieves number of timestamps stored in file.
		 */
		inline size_t num_times() const 
		{return _timestamps.size();};
		
		/**
		 * Retrieves the i'th pose.
		 */
		inline const Pose& pose(size_t i) const 
		{return _poses[i];};
		
		/**
		 * Retrieves the i'th timestamp
		 */
		inline double timestamp(size_t i) const 
		{return _timestamps[i];};
		
		/**
		 * Retrieves the closest index to the given timestamp
		 */
		inline size_t get_nearest_idx(double timestamp) const 
		{ 
			return binary_search::get_closest_index(
					_timestamps, timestamp);
		};
};

#endif
