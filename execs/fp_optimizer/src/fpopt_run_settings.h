#ifndef FPOPT_RUN_SETTINGS_H
#define FPOPT_RUN_SETTINGS_H

/**
 * @file   fpopt_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for fpopt program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * fpopt program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the fpopt program
 */
class fpopt_run_settings_t
{
	/* parameters */
	public:

		/* the following files are necessary for
		 * this program */

		/**
		 * Location of the input .oct file
		 */
		std::string octfile;

		/**
		 * Locations of input floorplan files
		 */
		std::vector<std::string> input_fpfiles;

		/**
		 * Location of output floorplan files
		 */
		std::vector<std::string> output_fpfiles;

		/**
		 * The number of iterations to run
		 *
		 * This vlaue represents the number of iterations
		 * to perform when attempting to align the floorplan
		 * to the carving via gradient descent.
		 */
		unsigned int num_iterations;

		/**
		 * The search range for moving a floorplan surface
		 *
		 * This value indicates the distance, in meters,
		 * that a floorplan surface can be perturbed in a
		 * single iteration of the optimization process.
		 */
		double search_range;

		/**
		 * Indicates the step siuze of the offset alignment
		 *
		 * This value indicates the step size, in units of
		 * tree.get_resolution(), of how to iterate over the
		 * different possible offsets for a given surface of
		 * the floorplan.  This value is typically less than
		 * 1, but not extremely so.
		 */
		double offset_step_coeff;

		/**
		 * If true, specifies that horizontal wall positions
		 * should be optimized for each floorplan.
		 *
		 * If false, horizontal positions will not be modified
		 */
		bool opt_walls;

		/**
		 * If true, specifies that vertical floor and ceiling
		 * heights should be optimized for each floorplan.
		 *
		 * If flase, vertical positions will not be modified.
		 */
		bool opt_heights;

		/**
		 * This value represents the bonus given to each
		 * surface offset based on its delta from the 
		 * previous offset.
		 *
		 * The purpose of awarding a delta-cost bonus is
		 * to favor the first offset that reached a particular
		 * cost, in order to limit shrinkage.
		 */
		double delta_cost_bonus;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		fpopt_run_settings_t();

		/**
		 * Parses settings from command-line
		 *
		 * Will parse the command-line arguments to get all
		 * the necessary settings.  This may also include
		 * parsing xml settings files that were passed on
		 * the command-line.
		 *
		 * @param argc  Number of command-line arguments
		 * @param argv  The command-line arguments given to main()
		 *
		 * @return      Returns zero on success, non-zero on failure
		 */
		int parse(int argc, char** argv);
};

#endif
