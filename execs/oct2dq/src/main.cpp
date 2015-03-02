#include "oct2dq_run_settings.h"
#include "process.h" 
#include <iostream>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Will form wall samples from an octree
 *
 * @section DESCRIPTION
 *
 * This is the main file for the wall sampling program.  This program
 * (oct2dq) will form wall samples using the geometry specified
 * in an octree.
 *
 * This program also generates the level split elevations and records
 * the vertical surfaces found in this model.
 */

using namespace std;

/* function implementations */

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	oct2dq_run_settings_t args;
	process_t process;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* initialize the data to process 
	 *
	 * This will produce planar regions that define surfaces
	 * in the model.
	 */
	ret = process.init(args);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to initialize data" << endl;
		return 2;
	}

	/* sort the planar regions into representative floors, walls, 
	 * and ceilings */
	ret = process.identify_surfaces(args);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to identify floors, walls, and ceilings" 
		     << endl;
		return 3;
	}

	/* estimate how many levels were scanned, and where to split them */
	ret = process.compute_level_splits(args);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to compute level splits" << endl;
		return 4;
	}

	/* compute the wall samples from the discovered planar regions */
	ret = process.compute_wall_samples(args);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to compute wall samples" << endl;
		return 5;
	}

	/* add pose information to the wall samples */
	ret = process.compute_pose_inds(args);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to compute pose indices" << endl;
		return 6;
	}

	/* export the samples */
	ret = process.export_data(args);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to export data." << endl;
		return 7;
	}

	/* success */
	return 0;
}
