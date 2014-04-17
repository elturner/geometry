#ifndef VOX_WRITER_H
#define VOX_WRITER_H

/**
 * @file vox_writer.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file defines classes used to export .vox files from octrees
 *
 * @section DESCRIPTION
 *
 * This file contains the vox_writer_t class, which is used to take an
 * octree, and export its contents to a .vox file.  This file format was
 * originally used for the voxel carving program specified in the following
 * paper:
 *
 * Eric Turner and Avideh Zakhor, "Watertight Planar Surface Meshing of 
 * Indoor Point-Clouds with Voxel Carving," Third Joint 3DIM/3DPVT 
 * Conference, Seattle, WA, June 29-July 1, 2013.
 *
 * This file format specifies the location of occupied voxels by explicitly
 * storing the voxel positions that occur on the boundary between connected
 * components of interior and exterior voxels.  Each voxel defined in the
 * .vox file is an exterior voxel that borders one or more interior voxels.
 *
 * This file requires the Eigen framework
 */

#include <geometry/octree/octree.h>
#include <Eigen/Dense>
#include <string>

/* a voxel_state_t is a value in the mapping at each voxel.  This value
 * is non-zero only at boundary solid voxels.  If non-zero, this value
 * denotes which faces of the boundary voxel are connected to interior
 * voxels.  Boundary voxels themselves are labeled exterior. */
typedef unsigned char voxel_state_t;

/**
 * The vox_writer_t class is used to convert octree_t to vox files
 *
 * All the functions for this class are static, so objects of this class
 * do not have to be instantiated.
 */
class vox_writer_t
{
	/* user-accessible functions */
	public:

		/**
		 * Exports a vox file to the specified location
		 *
		 * Will interpret the referenced octree and export it
		 * to the specified vox file.  Note that the output file
		 * will contain strictly less information than the octree
		 * representation, since none of the probabily information
		 * or adaptive sizing information will be stored.
		 *
		 * @param voxfile   Where to write the output vox file
		 * @param tree      The octree to parse and export
		 *
		 * @return     Returns zero on success, non-zero on failure
		 */
		static int write(const std::string& voxfile,
		                 const octree_t& tree);

	/* helper functions */
	private:

		/**
		 * Retrieves the voxel state for a location within a tree
		 *
		 * Will analyze the tree at the specified location, and
		 * determine what the state is of the voxel at the 
		 * deepest depth of the tree.  Note that the tree may
		 * not branch as far as this voxel depth, but it will
		 * still analyze what the voxel state would be if such
		 * a voxel existed.
		 *
		 * @param tree   The tree to analyze
		 * @param p      The position contained within the voxel
		 * @param r      The resolution of the voxel to analyze
		 * 
		 * @return       Returns the voxel state at p
		 */
		static voxel_state_t retrieve_state(const octree_t& tree,
					const Eigen::Vector3d& p, double r);
};

#endif
