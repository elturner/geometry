#ifndef SCANORAMA_MAKER_H
#define SCANORAMA_MAKER_H

/**
 * @file    scanorama_maker.h
 * @author  Eric Turner <elturner@indoorreality.com>
 * @brief   Generates scanorama_t objects based on dataset products
 *
 * @section DESCRIPTION
 *
 * Combines imagery and models in order to raytrace a new set of point
 * clouds from specified positions, color those point clouds appropriately,
 * then export those as gridded scanoramas.
 *
 * Created on June 8, 2015
 */

#include "scanorama.h"
#include <image/camera.h>
#include <geometry/system_path.h>
#include <geometry/raytrace/OctTree.h>
#include <Eigen/Dense>
#include <string>
#include <vector>
#include <memory>

/**
 * Generates scanoramas from dataset products.
 *
 * The scanorama_maker_t class contains the common dataset elements
 * used to make scanoramas, including the path, imagery, and geometry
 * information.
 */
class scanorama_maker_t
{
	/* parameters */
	private:

		/**
		 * The path of the system over time
		 *
		 * The system path indicates the trajectory of the
		 * system during the data acquisition
		 */
		system_path_t path;

		/**
		 * The list of all cameras used in this dataset
		 */
		std::vector<camera_t*> cameras;

		/**
		 * The model geometry, represented as a triangulated mesh.
		 *
		 * units: meters
		 */
		OctTree<float> model;

	/* functions */
	public:

		/*--------------*/
		/* constructors */
		/*--------------*/
		
		/**
		 * Default constructor 
		 *
		 * Creates empty object. 
		 *
		 * This octree is constructed with a maximum depth of 12.
		 * By default, the tree has a maximum depth of 10, which
		 * tends to not be enough for some of our larger models.
		 */
		scanorama_maker_t() : path(), cameras(), model(12)
		{};

		/**
		 * Frees all memory and resources
		 */
		~scanorama_maker_t()
		{ this->clear(); };

		/*----------------*/
		/* initialization */
		/*----------------*/

		/**
		 * Clears all information from this object
		 */
		void clear();

		/**
		 * Initializes this object based on the specified files
		 *
		 * @param pathfile    The file to the path data 
		 *                    (.mad or .noisypath)
		 * @param configfile  The .xml hardware configuration file
		 * @param modelfile   The model geometry 
		 *                    (either .obj or .ply)
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int init(const std::string& pathfile,
				const std::string& configfile,
				const std::string& modelfile);

		/**
		 * Adds a fisheye camera to be used to color 
		 * the output scanoramas.
		 *
		 * Fisheye cameras are defined with three pieces of 
		 * information:  a metadata file, a calibration file,
		 * and a folder containing the actual images.
		 *
		 * @param metafile    The metadata file (.txt) for this
		 *                    camera
		 * @param calibfile   The calibration file (.dat) for this
		 *                    camera
		 * @param imgdir      The folder containing imagery
		 *
		 * @return    Return zero on success, non-zero on failure.
		 */
		int add_fisheye_camera(const std::string& metafile,
		                       const std::string& calibfile,
		                       const std::string& imgdir);

		/**
		 * Adds a rectilinear camera to be used to color
		 * the output scanoramas.
		 *
		 * These cameras are defined by a metadata file (which
		 * includes the list of imagery), a calibration file
		 * (which includes the K-matrix), and the directory
		 * containing the imagery.
		 *
		 * @param metafile    The metadata file (.txt) for this
		 *                    camera
		 * @param calibfile   The calibration file (.dat) for this
		 *                    camera
		 * @param imgdir      The folder containing imagery
		 *
		 * @return    Return zero on success, non-zero on failure.
		 */
		int add_rectilinear_camera(const std::string& metafile,
		                           const std::string& calibfile,
		                           const std::string& imgdir);

		/*------------*/
		/* Generation */
		/*------------*/

		/**
		 * Populates a scanorama with the given info
		 *
		 * @param scan     The scanorama to populate
		 * @param t        The timestamp to set for this scanorama
		 * @param r        Number of rows to use
		 * @param c        Number of columns to use
		 * @param bw       The blendwidth to use (range [0,1])
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int populate_scanorama(scanorama_t& scan,
				double t, size_t r, size_t c, double bw);

		/**
		 * Generates and exports scanoramas for list of timestamps.
		 *
		 * Given a list of timestamps and an output destination,
		 * will export scanoramas for each of the specified
		 * timestamps.
		 *
		 * NOTE:  The scanoramas will be centered at the system 
		 * common coordiantes.  It may be worthwhile in the future
		 * to adjust this to center the scanorama at one of the
		 * cameras instead.
		 *
		 * @param prefix_out  The prefix for the output path
		 * @param meta_out    The output metadata file
		 * @param times       The input list of timestamps
		 * @param r           Number of rows to use
		 * @param c           Number of columns to use
		 * @param bw          The blendwidth to use (range [0,1])
		 * @param begin_idx   The index within times to start
		 *                    exporting, inclusive (default = 0)
		 * @param end_idx     The index within times to end
		 *                    exporting, exclusive (default = -1,
		 *                    which indicates that all values
		 *                    should be used)
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int generate_all(const std::string& prefix_out,
				const std::string& meta_out,
				const std::vector<double>& times,
				size_t r, size_t c, double bw,
				int begin_idx = 0, int end_idx = -1);

		/**
		 * Generates and exports scanoramas along the path at
		 * the specified spacing.
		 *
		 * Given a distance spacing parameter, will determine
		 * the list of times on which to generate scanoramas.
		 * These scanoramas will be generated and exported to
		 * the specified output destination.
		 *
		 * This function will internally call the generate_all()
		 * function.
		 *
		 * @param prefix_out   The prefix for the output path
		 * @param meta_out     The output metadata file
		 * @param minspacedist The min spacing distance (in meters)
		 * @param maxspacedist The max spacing distance (in meters)
		 * @param r            Number of rows to use
		 * @param c            Number of columns to use
		 * @param bw           The blendwidth to use (range [0,1])
		 * @param begin_idx    The index within the generated poses
		 *                     to start exporting, inclusive 
		 *                     (default = 0)
		 * @param end_idx      The index within the generated poses
		 *                     to end exporting, exclusive 
		 *                     (default = -1,
		 *                     which indicates that all values
		 *                     should be used)
		 *
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int generate_along_path(const std::string& prefix_out,
			const std::string& meta_out,
			double minspacedist, double maxspacedist,
			size_t r, size_t c, double bw,
			int begin_idx = 0, int end_idx = -1);

	/* helper functions */
	private:

		/**
		 * Populates the octree structure that stores the model
		 * for efficient raytracing operations.
		 *
		 * @param modelfile   Where to read the model from
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int populate_octree(const std::string& modelfile);

		/**
		 * Finds the first index that is at least min_dist away
		 * from reference posiiton.
		 *
		 * Given a list of positions and reference point, will
		 * iterate through the list (starting with the index
		 * i_start) and will determine the first index that
		 * is at least min_dist away from poses[i_ref].  If no
		 * elements meet that criteria, then poses.size() is
		 * returned.
		 *
		 * @param poses     The list of poses to search
		 * @param i_start   The first index to start searching
		 * @param i_ref     The index of reference point 
		 *                  to compare distances to
		 * @param min_dist  The minimum distance away from
		 *                  refpoint a returned pose must be.
		 *
		 * @return    Returns the index of the first pose >= i_start
		 *            that is at least min_dist away from refpoint.
		 */
		size_t index_jump_by_dist(
			const std::vector<transform_t,
	        	    Eigen::aligned_allocator<transform_t> >& poses,
			size_t i_start, size_t i_ref, 
			double min_dist) const;

		/**
		 * Retrieves the best candidate pose in a list of timestamps
		 *
		 * For the specified list of possible timestamps for a
		 * camera pose, will chose the one where the system had
		 * the minimal rotational velocity.
		 *
		 * @parma times    The list of timestamps to check
		 * @param i_start  The index to start searching (inclusive)
		 * @param i_end    The index to end searching (exclusive)
		 *
		 * @return    Returns i in [i_start,i_end] where the system
		 *            had the smallest rotational speed.
		 */
		size_t get_best_index(const std::vector<double>& times,
				size_t i_start, size_t i_end) const;
};

#endif
