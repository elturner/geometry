#include "hia_floorplan_settings.h"
#include <util/tictoc.h>
#include <iostream>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Generates floorplan from .hia (Histogrammed Interior Area) file
 *
 * @section DESCRIPTION
 *
 * This program will take in the top-down 2D histogram represented by
 * a .hia file, which dictates the layout of a level of a building
 * environment, and will generate a floorplan from that information.
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
	hia_floorplan_settings_t args;
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

	// TODO

	/* success */
	return 0;
}
