#include "chunker_run_settings.h"
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
		return 1;

	/* initialize */
	ret = carver.init(settings.madfile, settings.confile,
	                  settings.chunk_size, 0.003, // TODO use timesync
	                  settings.carvebuf);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ":  "
		     << "Unable to init carver" << endl;
		return 2;
	}

	/* process */
	ret = carver.export_chunks(settings.fssfiles,
	                           settings.chunklist_outfile,
	                           settings.chunkdir); 
	if(ret)
	{
		cerr << "[main]\tError " << ret << ":  "
		     << "Unable to export chunks" << endl;
		return 3;
	}

	/* success */
	return 0;
}
