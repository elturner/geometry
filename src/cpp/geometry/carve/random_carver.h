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
 * This is performed as an Unscented Transform, where samples of
 * the scan points' positions are generated from concatenating each
 * known source of error, and these samples are used to represent
 * a model of the probability that each intersected voxel is interior.
 */

#include <geometry/octree/octree.h>
#include <geometry/probability/noise_model.h>
#include <string>

/**
 * The random_carver_t class builds an octree from range scans.
 *
 * the random_carver_t class will insert range scans into an octree
 * in a probabilistic manner.  The parameters for our the probability
 * distributions are sampled and used are controlled by this class.
 */
class random_carver_t
{
	/* parameters */
	private:

		/* probabilistic structures */

		/* The noise model for the system encompasses all positional
		 * uncertainties in modeling the location of scan points.
		 * 
		 * This value includes all localization data.
		 */
		noise_model_t model;

		/* The octree holds the output representation of the
		 * space carving */
		octree_t tree;

		/* algorithm parameters */

		/* the number of samples per scan indicates how complex
		 * each scan point's random model is.  Increasing this
		 * number will make the output carving more accurate,
		 * but will increase run-time */
		unsigned int num_samples;

		/* The clock error represents the uncertainty (std. dev.)
		 * of the system clock when timestamping hardware sensors.
		 * It is expressed in units of seconds. */
		double clock_uncertainty;

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
		 * @param res       The carve resolution, in meters
		 * @param num_samps The number of samples per point to use
		 * @param clk_err   The system clock uncertainty, in secs
		 *
		 * @return     Returns zero on success, non-zero on failure.
		 */
		int init(const std::string& madfile,
		         const std::string& confile,
		         double res, unsigned int num_samps,
		         double clk_err);

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
		int carve(const std::string& fssfile);

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
		 * Performs carving for a single scan point
		 *
		 * Will use the scan point statistical model currently
		 * loaded into this->model to generate a distribution
		 * of the interesected volume of this scan point and
		 * add it to this->tree.
		 *
		 * @return    Returns zero on success, non-zero on failure.
		 */
		int carve_current_point();
};

#endif
