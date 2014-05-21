#include "wedge_run_settings.h"
#include <geometry/carve/wedge_generator.h>
#include <iostream>
#include <string>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The wedge-generation program, outputs carve wedges to disk
 *
 * @section DESCRIPTION
 *
 * This is the main file for the wedge generation program, which is used to
 * generate probabilistic carve_wedge_t structures from input scans
 */

using namespace std;

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	wedge_run_settings_t settings;
	wedge_generator_t wedgen;
	int ret;

	/* get command-line arguments */
	ret = settings.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tUnable to import settings" << endl;
		return 1;
	}

	/* initialize the generator */
	ret = wedgen.init(settings.pathfile, settings.confile,
			settings.timefile,
			settings.default_clock_uncertainty,
			settings.carvebuf, settings.linefit_dist);
	if(ret)
	{
		/* an error occurred */
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to initialize wedge generator" << endl;
		return 2;
	}

	/* generate probabilistic wedge structures for all input scans,
	 * and export these wedges to file */
	ret = wedgen.process(settings.fssfiles,
	                     settings.carvemapfile, settings.wedgefile);
	if(ret)
	{
		/* an error occurred */
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to process wedges from input scans" << endl;
		return 3;
	}

	/* success */
	return 0;
}
