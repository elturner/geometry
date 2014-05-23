#ifndef FP_OPTIMIZER_H
#define FP_OPTIMIZER_H

/**
 * @file fp_optimizer.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This file contains functions to optmize floorplans from octrees
 *
 * @section DESCRIPTION
 *
 * This file contains the fp_optimizer_t class, which is used to modify
 * the geometry of a floorplan in order to align it with the geometry
 * described in an octree.
 */

#include <mesh/floorplan/floorplan.h>
#include <geometry/octree/octree.h>
#include <vector>
#include <string>

/**
 * The fp_optimizer_t class will import, modify, and export a floorplan
 *
 * This class will modify a floorplan in order to optimize its geometry
 * to match the carving contain the octree provided.
 */
class fp_optimizer_t
{
	/* parameters */
	private:

		/**
		 * The octree to use in order to improve floorplan geometry
		 */
		octree_t tree;

		/**
		 * This denotes the floorplan to modify
		 */
		fp::floorplan_t floorplan;

	/* functions */
	public:

		/*------------*/
		/* processing */
		/*------------*/

		/**
		 * Will optimize all floorplans based on the given octree
		 *
		 * Performs the primary function of this class, which is
		 * to optimze one or more floorplans based on the carving
		 * data from an octree.
		 *
		 * @param octfile    The file containing the octree to use
		 * @param infiles    The input .fp files to optimize
		 * @param outfiles   The output .fp files to export
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int process_all(const std::string& octfile,
		                const std::vector<std::string>& infiles,
		                const std::vector<std::string>& outfiles);

		/**
		 * Will optimize the given floorplan based on the octree
		 *
		 * Will import the referenced floorplan information,
		 * adjust the geometry of the floorplan to align with
		 * the carving stored in the octree, and export the
		 * modified floorplan geometry to the given output file.
		 *
		 * @param octfile    The file containing the octree to use
		 * @param infile     The input .fp floorplan file to use
		 * @param outfile    The output .fp file to export
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int process(const std::string& octfile,
		            const std::string& infile,
		            const std::string& outfile);

		/*-----*/
		/* i/o */
		/*-----*/

		/**
		 * Loads an octree from file
		 *
		 * Given a specified .oct file, will load the information
		 * into the octree of this structure.  This information
		 * will be used to align the stored floorplan to the
		 * carved geometry.
		 *
		 * @param filename   The .oct file to import
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int load_oct(const std::string& filename);

		/**
		 * Loads a floorplan from file
		 *
		 * Given a speified .fp file, will load the information
		 * into the floorplan structure of this object.  The
		 * input floorplan can then be modified to align with the
		 * stored octree in this object.
		 *
		 * @param filename   The .fp file to import
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int load_fp(const std::string& filename);

		/**
		 * Will export the stored floorplan to the given file
		 *
		 * Assuming that the floorplan parameter of this object
		 * is initialized, will export this floorplan to the
		 * specified .fp file.
		 *
		 * @param filename   Where to write the .fp file
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int export_fp(const std::string& filename) const;

	/* helper functions */
	private:

		/**
		 * Will attempt to optimize the floorplan to the octree
		 *
		 * Calling this function will modify the geometry of the
		 * stored floorplan, attempting to align it with the
		 * carving information stored in the octree.  This must
		 * be called after both the octree and floorplan have
		 * been loaded.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int optimize();

		/**
		 * Will run a single iteration of the wall optimization
		 *
		 * This function is called by optimize().  This will
		 * run a single iteration of the floorplan optimization
		 * process, which will modify the vertices slightly to
		 * align to the carved octree.  Several calls to this
		 * function may be required.
		 *
		 * This iteration will only modify the wall positions,
		 * which means only the (x,y) positions of the vertices
		 * will be changed, and the elevation (z) of the vertices
		 * will be unmodified.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int run_iteration_walls();

		/**
		 * Will run a single iteration of the height optimization
		 *
		 * This function is called by optimize().  This will
		 * run a single iteration of the floorplan optimization.
		 *
		 * This will only modify the heights (z) of vertices to
		 * attempt to align the floor and ceiling positions to the
		 * information stored in the octree.  The horizontal (x,y)
		 * positions of the vertices will remain unchanged.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int run_iteration_heights();
};

#endif
