#include "chunker_run_settings.h"
#include <geometry/carve/wedge_generator.h>
#include <geometry/carve/random_carver.h>
#include <iostream>
#include <string>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The scan-chunking program, outputs scan index chunks to disk
 *
 * @section DESCRIPTION
 *
 * This is the main file for the scan-chunking program, which is used to
 * generate spatially-oriented input for the probability-carving program
 * (procarve).
 */

using namespace std;

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	chunker_run_settings_t settings;
	random_carver_t carver;
	int ret;
	
	/* get command-line arguments */
	ret = settings.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tUnable to import settings" << endl;
		return 1;
	}

	/* initialize */
	carver.init(settings.chunk_size, 1);

	/* process */
	ret = carver.export_chunks(settings.carvemapfile,
	                           settings.wedgefile,
	                           settings.chunklist_outfile,
	                           settings.chunkdir); 
	if(ret)
	{
		cerr << "[main]\tError " << ret << ":  "
		     << "Unable to export chunks" << endl;
		return 2;
	}

	/* success */
	return 0;
}
