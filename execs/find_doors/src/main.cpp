#include "io/find_doors_settings.h"
#include "process/door_finder.h"
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
#include <geometry/hist/hia_analyzer.h>
#include <util/tictoc.h>
#include <iostream>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Estimates positions of doors in octree models
 *
 * @section DESCRIPTION
 *
 * Given an octree representation of a model (including room id's),
 * will estimate the position of doors by following the provided
 * localization path and determining where rooms were entered and exited.
 */

using namespace std;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	find_doors_settings_t args;
	door_finder_t door_finder;
	system_path_t path;
	octree_t tree;
	hia_analyzer_t hia;
	tictoc_t clk;
	size_t hia_index, num_hia;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* read in the octree */
	tic(clk);
	ret = tree.parse(args.octfile);
	if(ret)
	{
		cerr << "[main]\tUnable to parse octree file: \""
		     << args.octfile << "\", Error " << ret << endl;
		return 2;
	}

	/* read in the path information */
	ret = path.read(args.pathfile);
	if(ret)
	{
		cerr << "[main]\tUnable to parse path file: \""
		     << args.pathfile << "\", Error " << ret << endl;
		return 3;
	}
	toc(clk, "Importing files");

	/* find doors for each hia file specified */
	num_hia = args.hiafiles.size();
	for(hia_index = 0; hia_index < num_hia; hia_index++)
	{
		/* read in the hia information */
		ret = hia.readhia(args.hiafiles[hia_index]);
		if(ret)
		{
			cerr << "[main]\tUnable to read .hia file: \""
			     << args.hiafiles[hia_index] << "\"" << endl;
			return 4;
		}

		/* initialize parameters */
		door_finder.init(args.door_min_width, args.door_max_width,
				args.door_min_height, args.door_max_height,
				args.angle_stepsize);

		/* perform analysis */
		ret = door_finder.analyze(tree, hia, path);
		if(ret)
		{
			cerr << "[main]\tUnable to perform analysis, Error "
			     << ret << endl;
			return 5;
		}

		/* export data */
		ret = door_finder.writedoors(args.outfile_prefix,
						hia.get_level(),
						args.outfile_xyz);
		if(ret)
		{
			cerr << "[main]\tUnable to export output file, "
			     << "Error " << ret << endl;
			return 6;
		}
	}

	/* success */
	return 0;
}

