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
#include <image/fisheye/fisheye_camera.h>
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
		 * @param prefix_out  The prefix for the output path
		 * @param spacingdist The spacing distance (in meters)
		 * @param r           Number of rows to use
		 * @param c           Number of columns to use
		 * @param bw          The blendwidth to use (range [0,1])
		 * @param begin_idx   The index within the generated poses
		 *                    to start exporting, inclusive 
		 *                    (default = 0)
		 * @param end_idx     The index within the generated poses
		 *                    to end exporting, exclusive 
		 *                    (default = -1,
		 *                    which indicates that all values
		 *                    should be used)
		 *
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int generate_along_path(const std::string& prefix_out,
			double spacingdist, size_t r, size_t c, double bw,
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
};

#endif
