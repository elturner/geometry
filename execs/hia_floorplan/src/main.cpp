#include "hia_floorplan_settings.h"
#include <geometry/hist/hia_analyzer.h>
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
	hia_analyzer_t analyzer;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* import the hia file */
	ret = analyzer.readhia(args.hiafile);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to import hia file: "
		     << args.hiafile << endl;
		return 2;
	}

	// TODO
	cout << "summing: " << analyzer.populate_neighborhood_sums(0.8) << endl;
	cout << "localmax: " << analyzer.label_local_maxima(0.8) << endl;
	cout << "rooms: " << analyzer.propegate_room_labels() << endl;
	analyzer.write_localmax(cerr);

	/* success */
	return 0;
}
