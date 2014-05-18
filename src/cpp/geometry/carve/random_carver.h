#ifndef RANDOM_CARVER_H
#define RANDOM_CARVER_H

/**
 * @file random_carver.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the random_carver_t class, which is used
 * to import probabilistic models of the position of scan points,
 * and insert those models into an octree.
 *
 * This class requires the Eigen framework.
 * This class requires the boost::threadpool framework.
 */

#include <io/carve/chunk_io.h>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
#include <geometry/octree/octree.h>
#include <boost/threadpool.hpp>
#include <string>
#include <vector>
#include <set>

/**
 * The random_carver_t class builds an octree using carve wedges.
 *
 * the random_carver_t class will populate an octree
 * in a probabilistic manner.  The parameters for our the probability
 * distributions are used to model a gaussian distribution for the
 * final position of each point in world coordinates, and to compute
 * the probability that the ray for each point passes through
 * a given voxel in the tree.
 */
class random_carver_t
{
	/* parameters */
	private:

		/* The octree holds the output representation of the
		 * space carving */
		octree_t tree;

		/* total number of rooms imported into this tree */
		unsigned int num_rooms;

		/* algorithm parameters */

		/* the number of threads to use when carving nodes from
		 * chunks.  By default, this value is the number of hardware
		 * cores detected. */
		unsigned int num_threads;

	/* functions */
	public:

		/**
		 * Constructs empty object
		 */
		random_carver_t();

		/**
		 * Initializes this object with specified data sources
		 *
		 * Given localization information and processing parameters,
		 * this function call will prepare this object to perform
		 * probabilistic carving of the scanned volume.
		 *
		 * @param res       The carve resolution, in meters
		 * @param nt        The number of threads to use
		 */
		void init(double res, unsigned int nt);

		/**
		 * Finds and exports all chunks to disk
		 *
		 * Given a wedge file with a list of wedges to process,
		 * will iterate through all carve wedges, find which ones
		 * intersects which chunks of the world volume, and export
		 * corresponding chunk files to the specified location on 
		 * disk.
		 *
		 * The size of the chunks is determined by the resolution
		 * passed to the init() funciton, which should be called
		 * before calling this function.
		 *
		 * NOTE: the chunk dir should be relative to the
		 * directory that contains chunklist.
		 *
		 * @param cmfile      The carvemap file to parse for input
		 * @param wedgefile   The wedge file to parse for input
		 * @param chunklist   File location to export chunklist
		 * @param chunk_dir   The directory to export chunks
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int export_chunks(const std::string& cmfile,
		                  const std::string& wedgefile,
		                  const std::string& chunklist,
		                  const std::string& chunk_dir);

		/**
		 * Carves all input scans in a chunk-by-chunk fashion
		 *
		 * Will parse specified chunk list, and read in all
		 * chunks from disk.  For each chunk, the referenced
		 * wedges will be carved into the tree, and that chunk
		 * will be simplified before the next chunk is imported.
		 *
		 * @param cmfile       The input carve map file for carving
		 * @param wedgefile    The reference wedge file for carving
		 * @param chunklist    File location to import chunks
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int carve_all_chunks(const std::string& cmfile,
		                     const std::string& wedgefile,
		                     const std::string& chunklist);

		/**
		 * Imports floor plan information into a carved tree
		 *
		 * After carving, calling this function will parse
		 * a floorplan and import its room information into
		 * the tree.
		 *
		 * @param fpfile   The input .fp file to parse and use
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int import_fp(const std::string& fpfile);

		/**
		 * Exports the stored octree to a .oct file
		 *
		 * Will serialize this object's octree to the specified
		 * .oct file.  This file will contain the computed
		 * volumetric information.
		 *
		 * @param octfile   The path to the .oct file to export
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int serialize(const std::string& octfile) const;

	/* helper functions */
	private:
		
		/**
		 * Carves a single chunk into the tree
		 *
		 * Will parse the specified chunk file, and carve it into
		 * the tree.  It is assumed that the fss_files list
		 * will be the same size and order as the sensors referenced
		 * in the corresponding chunklist file.
		 *
		 * @param carvemaps   The stream of carvemaps to use
		 * @param wedges      The stream of wedges to use
		 * @param chunkfile   The chunkfile to parse
		 * @param tp          The threadpool to use to carve fast
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int carve_chunk(cm_io::reader_t& carvemaps,
		                wedge::reader_t& wedges,
		                const std::string& chunkfile,
		                boost::threadpool::pool& tp);

		/**
		 * Will carve the given data into the given octnode
		 *
		 * This function will modify the given octnode based on
		 * the data given.  Any scans from the referenced indices
		 * will be attempted to be carved into the octnode.
		 *
		 * The inds structure will be copied, since that is
		 * the only structure that is not likely to be persistant
		 * between node calls.
		 *
		 * @param chunknode   The node to carve into
		 * @param inds        The scan indices to use
		 * @param carvemaps   The referenced input carve maps
		 * @param wedges      The referenced input carve wedges
		 * @param maxdepth    The relative max depth to carve
		 * @param verbose     If true, will print a progress bar
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		static int carve_node(octnode_t* chunknode,
			std::set<chunk::point_index_t> inds,
			cm_io::reader_t& carvemaps,
			wedge::reader_t& wedges,
			unsigned int maxdepth, bool verbose);
};

#endif
