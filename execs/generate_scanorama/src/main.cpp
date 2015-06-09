#include "generate_scanorama_run_settings.h"
#include <image/scanorama/scanorama_maker.h>
#include <iostream>
#include <string>

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
	scanorama_maker_t maker;
	size_t i, n;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* initialize the maker object */
	ret = maker.init(args.pathfile, args.xml_config, args.modelfile);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not initialize" << endl;
		return 2;
	}

	/* import all cameras that are given */
	n = args.cam_metafiles.size();
	for(i = 0; i < n; i++)
	{
		/* add this camera */
		ret = maker.add_camera(
				args.cam_metafiles[i],
				args.cam_calibfiles[i],
				args.cam_imgdirs[i]);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Could not add camera #" << i << endl;
			return 3;
		}
	}

	// TODO actually make some scanoramas

	/* success */
	return 0;
}
