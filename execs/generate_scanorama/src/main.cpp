#include "generate_scanorama_run_settings.h"
#include <image/scanorama/scanorama.h>
#include <iostream>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Will generate scanorama (.ptx) files from this dataset
 *
 * @section DESCRIPTION
 *
 * This is the main file for the scanorama program.  This program
 * (generate_scanorama) will form scanorama products 
 * using the imported imagery and geometry.
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
	generate_scanorama_run_settings_t args;
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
	scanorama_t scan;
	scan.init_sphere();
	scan.writeptx(cerr);

	/* success */
	return 0;
}
