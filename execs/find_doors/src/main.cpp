#include "io/find_doors_settings.h"
#include "process/door_finder.h"
#include <geometry/system_path.h>
#include <geometry/octree/octree.h>
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
	tictoc_t clk;
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

	/* initialize parameters */
	door_finder.init(args.door_min_width, args.door_max_width,
			args.door_max_height);

	/* perform analysis */
	ret = door_finder.analyze(tree, path, args.levelsfile);
	if(ret)
	{
		cerr << "[main]\tUnable to perform analysis, Error "
		     << ret << endl;
		return 4;
	}

	/* export data */
	ret = door_finder.writetxt(args.outfile);
	if(ret)
	{
		cerr << "[main]\tUnable to export output file, Error "
		     << ret << endl;
		return 5;
	}

	/* success */
	return 0;
}

