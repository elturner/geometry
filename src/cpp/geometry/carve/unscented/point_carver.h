#ifndef POINT_CARVER_H
#define POINT_CARVER_H

/**
 * @file point_carver.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the point_carver_t class, which is used to carve
 * a single scan point into an octree.  The probability distribution
 * of the position of this point is used to generate a distribution of
 * the interior/exterior properties of intersected voxels.
 *
 * This class requires the Eigen framework.
 */

#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <Eigen/Dense>
#include <map>

/**
 * the point_carver_t class is used to model probabilistic carving
 *
 * The distribution of a single point's position can be imported, and
 * used to model the occupancy distribution of the intersected volume.
 */
class point_carver_t
{
	/* parameters */
	private:

		/* the following maps represents which voxels in space
		 * are intersected by the distribution of this point's
		 * ray, and how many times they are intersected. */
		std::map<octnode_t*, unsigned int> volume_map;

		/* this value indicates how many samples have been
		 * inserted so far */
		unsigned int num_samples;

	/* functions */
	public:

		/**
		 * Initializes empty object.
		 */
		point_carver_t();

		/**
		 * Clears all info from this object.
		 */
		void clear();

		/**
		 * Adds a random sample of the point position to this object
		 *
		 * Given the sampled position of the start and end points
		 * of the scanned ray (the start point being the location
		 * of the sensor, and the end point being the location
		 * of the point), will find the intersected volume within
		 * the octree and add to the built map.
		 *
		 * @param sensor_pos   The sampled sensor position
		 * @param scan_pos     The sampled scan point position
		 * @param tree         The tree to intersect
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int add_sample(const Eigen::Vector3d& sensor_pos,
		               const Eigen::Vector3d& scan_pos,
		               octree_t& tree);

		/**
		 * Updates the carved octree with the stored volume info
		 *
		 * After adding a sufficient number of samples, call this
		 * function to update the data stored in the inputted
		 * octree to add this intersected volume information to
		 * the tree.
		 *
		 * @return    Returns zero on success, non-zero on failure
		 */
		int update_tree() const;

		/* debugging */

		/**
		 * Exports contents of this structure to file
		 *
		 * Will export the volume map described by this object
		 * to a file, in the following format:
		 *
		 * Each voxel is a line in the ascii file, with:
		 *
		 * <x> <y> <z> <hw> <count>
		 *
		 * @param filename   The file to export to
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int write_to_file(const std::string& filename) const;
};

#endif
