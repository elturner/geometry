#include "generate_hia_settings.h"
#include <io/levels/building_levels_io.h>
#include <geometry/octree/octree.h>
#include <geometry/hist/octhist_2d.h>
#include <util/tictoc.h>
#include <iostream>
#include <sstream>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Generates .hia file (Histogrammed Interior Area) from an octree
 *
 * @section DESCRIPTION
 *
 * This program will take in an octree and optionally a levels file,
 * and define a 2D top-down histogram for each level in the imported
 * volume.
 */

using namespace std;

/* the following defines are used to indicate the output file suffix */
#define HIAFILE_EXT "hia"

/*--------------------------*/
/* function implementations */
/*--------------------------*/

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	generate_hia_settings_t args;
	building_levels::file_t levels;
	octree_t tree;
	octhist_2d_t hist;
	stringstream ss;
	tictoc_t clk;
	size_t curr_level;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* import the octree and levels files */
	tic(clk);
	ret = tree.parse(args.octree_file);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not import octree." << endl;
		return 2;
	}
	ret = levels.parse(args.levels_file);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not import levels file." << endl;
		return 3;
	}
	toc(clk, "Importing data");

	/* iterate over the levels, performing the histogram */
	for(curr_level = 0; curr_level < levels.num_levels(); curr_level++)
	{
		/* initialize the histogram for this level */
		ret = hist.init(tree, levels.get_level(curr_level));
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Could not initialize histogram." << endl;
			return 4;
		}

		/* prepare output file name */
		ss.clear();
		ss.str("");
		ss << args.hia_prefix << curr_level << "." << HIAFILE_EXT;

		/* export the output file */
		ret = hist.writehia(ss.str());
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Could not export output hia file: " 
			     << ss.str() << endl;
			return 5;
		}
	}

	/* success */
	return 0;
}
