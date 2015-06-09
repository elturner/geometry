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

#include <image/camera.h>
#include <geometry/system_path.h>
#include <geometry/raytrace/OctTree.h>
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
		std::vector<std::shared_ptr<camera_t> > cameras;

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
		int add_camera(const std::string& metafile,
		               const std::string& calibfile,
		               const std::string& imgdir);

		// TODO functions to make scanoramas

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
