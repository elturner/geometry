#include "procarve_run_settings.h"
#include <geometry/carve/random_carver.h>
#include <iostream>
#include <string>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the main file for the probability-carving program (procarve),
 * which will generate a surface reconstruction of a building interior
 * environment from range scans from a mobile scanning system.
 */

using namespace std;

/* function implementations */

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	procarve_run_settings_t settings;
	random_carver_t carver;
	unsigned int i, n;
	int ret;
	
	/* set input files */
	ret = settings.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to parse command-line args." << endl;
		return 1;
	}

	/* initialize */
	ret = carver.init(settings.madfile, settings.confile,
			settings.timefile, settings.resolution, 
			settings.default_clock_uncertainty,
			settings.carvebuf, settings.num_threads);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to init carver." << endl;
		return 2;
	}

	/* process */
	ret = carver.carve_all_chunks(settings.fssfiles,
	                              settings.chunklist);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to process chunks." << endl;
		return 3;
	}

	/* import some floorplans */
	n = settings.fpfiles.size();
	for(i = 0; i < n; i++)
	{
		/* import current floorplan */
		ret = carver.import_fp(settings.fpfiles[i]);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to import floorplan "
			     << settings.fpfiles[i] << endl;
			return 4;
		}
	}

	/* export */
	ret = carver.serialize(settings.octfile);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to export tree to " 
		     << settings.octfile << endl;
		return 5;
	}

	/* success */
	return 0;
}
