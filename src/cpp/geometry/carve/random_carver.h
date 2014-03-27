#ifndef RANDOM_CARVER_H
#define RANDOM_CARVER_H

/**
 * @file random_carver.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the random_carver_t class, which is used
 * to create probabilistic models of the position of scan points,
 * and insert those models into an octree.
 *
 * This class requires the Eigen framework.
 * This class requires the boost::threadpool framework.
 */

#include <io/carve/chunk_io.h>
#include <io/data/fss/fss_io.h>
#include <timestamp/sync_xml.h>
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
#include <boost/threadpool.hpp>
#include <string>
#include <vector>
#include <set>

/**
 * The random_carver_t class builds an octree from range scans.
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

		/* the path of the system through space. */
		system_path_t path;

		/* The octree holds the output representation of the
		 * space carving */
		octree_t tree;

		/* total number of rooms imported into this tree */
		unsigned int num_rooms;

		/* algorithm parameters */

		/* The clock error represents the uncertainty (std. dev.)
		 * of the system clock when timestamping hardware sensors.
		 * It is expressed in units of seconds. This error can
		 * be different for different sensors, and is described
		 * in the time synchronization output file. */
		SyncXml timesync;

		/* if unable to compute the timestamp uncertainty for a
		 * particular sensor, use this value */
		double default_clock_uncertainty;

		/* the carving buffer, in units of standard deviations,
		 * dictates how far past each point will be carved. A
		 * non-zero buffer allows for the exterior-mapped portions
		 * of the probability distributions to be explicity
		 * represented in the octree. */
		double carving_buffer;

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
		 * @param madfile   The path file for this dataset
		 * @param confile   The xml hardware config file
		 * @param tsfile    The time synchronization output xml file
		 * @param res       The carve resolution, in meters
		 * @param dcu       The default clock uncertainty
		 * @param carvebuf  The carving buffer, units of std. dev.
		 * @param nt        The number of threads to use
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int init(const std::string& madfile,
		         const std::string& confile,
		         const std::string& tsfile,
		         double res, double dcu,
		         double carvebuf, unsigned int nt);

		/**
		 * Finds and exports all chunks to disk
		 *
		 * Given a list of scan files to process, will iterate
		 * through all scans, find which scans intersects which
		 * chunks of the world volume, and export corresponding
		 * chunk files to the specified location on disk.
		 *
		 * The size of the chunks is determined by the resolution
		 * passed to the init() funciton, which should be called
		 * before calling this function.
		 *
		 * NOTE: the chunk dir should be relative to the
		 * directory that contains chunklist.
		 *
		 * @param fss_files   A list of scan filenames to use
		 * @param chunklist   File location to export chunklist
		 * @param chunk_dir   The directory to export chunks
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int export_chunks(const std::vector<std::string>& fss_files,
		                  const std::string& chunklist,
		                  const std::string& chunk_dir);

		/**
		 * Carves all input scans in a chunk-by-chunk fashion
		 *
		 * Will parse specified chunk list, and read in all
		 * chunks from disk.  For each chunk, the referenced
		 * scans will be carved into the tree, and that chunk
		 * will be simplified before the next chunk is imported.
		 *
		 * @param fss_files    A list of the scan filenames to use
		 * @param chunklist    File location to import chunks
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int carve_all_chunks(
			const std::vector<std::string>& fss_files,
			const std::string& chunklist);

		/**
		 * Carves a single chunk into the tree
		 *
		 * Will parse the specified chunk file, and carve it into
		 * the tree.  It is assumed that the fss_files list
		 * will be the same size and order as the sensors referenced
		 * in the corresponding chunklist file.
		 *
		 * @param fss_files   A list of fss file streams
		 * @param ts_uncerts  A list of sensor clock uncertainties
		 * @param chunkfile   The chunkfile to parse
		 * @param tp          The threadpool to use to carve fast
		 *
		 * @return   Returns zero on success, non-zero on failure.
		 */
		int carve_chunk(
			const std::vector<fss::reader_t*>& fss_files,
			const std::vector<double>& ts_uncerts,
			const std::string& chunkfile,
			boost::threadpool::pool& tp);

		/**
		 * Carves all input scan points into the octree
		 *
		 * Given the path to a .fss scan file, will parse this
		 * file and import all scan points into the octree, using
		 * each scan to define which portions of the volume are
		 * interior and which are exterior.
		 *
		 * @param fssfile   The input scan file to parse and use
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int carve_direct(const std::string& fssfile);

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
		 * Gets the timestamp uncertainty for a specific sensor
		 *
		 * Given the sensor name, will look up the timestamp
		 * uncertainty for that sensor, and return the desired
		 * value.
		 *
		 * @return   Returns the std. dev. of the clock error for
		 *           the specified sensor.
		 */
		double get_clock_uncertainty_for_sensor(
				const std::string& sensor_name) const;

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
		 * @param fss_files   The scan files to use as input
		 * @param ts_uncerts  Each sensor's timestamp uncertainty
		 * @param path        The system path to use
		 * @param maxdepth    The relative max depth to carve
		 * @param carvebuf    The carving buffer parameter
		 */
		static void carve_node(octnode_t* chunknode,
			std::set<chunk::point_index_t> inds,
			const std::vector<fss::reader_t*>& fss_files,
			const std::vector<double>& ts_uncerts,
			const system_path_t& path,
			unsigned int maxdepth, double carvebuf);
};

#endif
