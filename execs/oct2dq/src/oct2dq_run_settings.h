#ifndef OCT2DQ_RUN_SETTINGS_H
#define OCT2DQ_RUN_SETTINGS_H

/**
 * @file   oct2dq_run_settings.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for oct2dq program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the 
 * oct2dq program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

#include <string>
#include <vector>

/**
 * This class is used to store run settings for the oct2dq program
 */
class oct2dq_run_settings_t
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
		 * Location of the input .noisypath file
		 */
		std::string pathfile;

		/**
		 * Location of the input xml hardware config file
		 */
		std::string configfile;

		/**
		 * Locations of the input scan files
		 */
		std::vector<std::string> fssfiles;

		/**
		 * Location of the output .dq file
		 */
		std::string dqfile;

		/**
		 * Location of the (optional) output .levels file
		 */
		std::string levelsfile;

		/* the following parameters are used for this program */

		/**
		 * The distance threshold to use for region coalescing
		 *
		 * Node faces that are farther than this many std. devs.
		 * away from the fitting plane will be considered outliers
		 * and prevent regions from being merged.
		 */
		double coalesce_distthresh;

		/**
		 * The planarity threshold to use for region coalescing
		 *
		 * Node faces with a planarity less than this value
		 * will not be coalesced into larger regions.
		 */
		double coalesce_planethresh;

		/**
		 * Specifies whether to use isosurface positions
		 *
		 * If true, will use isosurface positions for each
		 * node face when computing fitting planes for regions
		 */
		bool use_isosurface_pos;

		/**
		 * Specifies the verticality threshold to use
		 * when determining if a surface is vertically aligned.
		 *
		 * This value is a threshold for the dot-product of the
		 * normal of a surface with the z-vector.  Surfaces will
		 * only be considered vertical if the following inequality
		 * is met:
		 *
		 * abs(surface_normal.dot(<0,0,1>)) < verticalitythresh
		 *
		 * NOTE:  this same threshold is used for determining
		 * if planar regions are horizontal, as well.
		 */
		double verticalitythresh;

		/**
		 * Specifies the surface area threshold to use
		 * to filter regions for candidacy for wall samples.
		 *
		 * Only regions that have at least this much surface
		 * area are considered to donate wall samples.
		 *
		 * NOTE:  This threshold is only used for vertical
		 * surfaces.  A separate threshold is used for
		 * horizontal surfaces.
		 *
		 * units: meters squared
		 */
		double surfaceareathresh;

		/**
		 * Specifies the minimum wall height allowed for
		 * regions to be used to contribute to wall samples.
		 *
		 * Note that this is measured on the bounding box
		 * of the region, not the region itself, so any
		 * occlusions should not have to be worried about.
		 *
		 * units: meters
		 */
		double wallheightthresh;

		/**
		 * Specifies the surface area threshold to use
		 * to filter regions for candidacy as floors or ceilings.
		 *
		 * Only regions that have this much surface area are
		 * considered to be possible floor or ceiling areas.
		 *
		 * NOTE:  this threshold is only used for horizontal
		 * surfaces.  A separate threshold is used for vertical
		 * surfaces.
		 *
		 * units:  meters squared
		 */
		double floorceilsurfareathresh;

		/**
		 * Minimum height between building levels
		 *
		 * This value indicates the minimum possible height
		 * of a building story, going from the floor surface
		 * on one level to the floor surface on the next level.
		 *
		 * This value should be a loose lower bound, since no
		 * levels will be detected smaller than this.
		 *
		 * units: meters
		 */
		double minlevelheight;

		/**
		 * Minimum room feature length
		 *
		 * This value indicates the range to check across for
		 * the best wall sample along a given ray.  This value
		 * should be no larger than the samllest width of a room,
		 * so that we don't accidently choose another valid wall
		 * behind a given wall.
		 *
		 * units: meters
		 */
		double minroomsize;

		/**
		 * The minimum percentage of poses that choose a valid
		 * wall sample
		 *
		 * Each wall sample is chosen based on how many poses see it
		 * as the best wall smaple for a given direction.  This
		 * value indicates the percentage of poses that chose it
		 * over other possible samples.  This threshold is meant
		 * to select the best samples.
		 *
		 * units:  unitless, range [0,1]
		 */
		double choiceratiothresh;

		/**
		 * Specifies the output DQ resolution to export
		 *
		 * units: meters
		 */
		double dq_resolution;

	/* functions */
	public:

		/**
		 * Creates an empty object
		 */
		oct2dq_run_settings_t();

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
