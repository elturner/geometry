#include "export_data.h"
#include "../io/config.h"
#include "../io/idf_io.h"
#include "../io/csv_io.h"
#include "../io/ply_io.h"
#include "../structs/building_model.h"
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <iostream>

/**
 * @file export_data.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Exports building models to output files
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to export the floorplan
 * data to various formats.
 */

using namespace std;

/* function implementations */

int export_data(const building_model_t& bim, const config_t& conf)
{
	tictoc_t clk;
	int ret;
	unsigned int i, n;

	/* start the clock */
	tic(clk);

	/* export specified files */

	/* export obj files */
	n = conf.outfile_obj.size();
	for(i = 0; i < n; i++)
	{
		/* export to this obj file */
		ret = bim.export_obj(conf.outfile_obj[i]);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[export_data]\tError " << ret << ": "
			     << "Unable to export obj file #" << i << ": "
			     << conf.outfile_obj[i] << endl;
			return ret;
		}
	}
	
	/* export wrl files */
	n = conf.outfile_wrl.size();
	for(i = 0; i < n; i++)
	{
		/* export to this wrl file */
		ret = bim.export_wrl(conf.outfile_wrl[i]);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[export_data]\tError " << ret << ": "
			     << "Unable to export wrl file #" << i << ": "
			     << conf.outfile_wrl[i] << endl;
			return ret;
		}
	}

	/* export idf files */
	n = conf.outfile_idf.size();
	for(i = 0; i < n; i++)
	{	
		/* export to this idf file */
		ret = writeidf(conf.outfile_idf[i], bim);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[export_data]\tError " << ret << ": "
			     << "Unable to export idf file #" << i << ": "
			     << conf.outfile_idf[i] << endl;
			return ret;
		}
	}

	/* export csv files */
	n = conf.outfile_csv.size();
	for(i = 0; i < n; i++)
	{
		/* export to csv file */
		ret = writecsv(conf.outfile_csv[i], bim);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-4, ret);
			cerr << "[export_data]\tError " << ret << ": "
			     << "Unable to export csv file #" << i << ": "
			     << conf.outfile_csv[i] << endl;
			return ret;
		}
	}

	/* export ply files */
	n = conf.outfile_ply.size();
	for(i = 0; i < n; i++)
	{
		/* export each ply file given */
		ret = writeply(conf.outfile_ply[i], bim);
		if(ret)
		{
			/* error occurred */
			ret = PROPEGATE_ERROR(-5, ret);
			cerr << "[export_data]\tError " << ret << ": "
			     << "Unable to export ply file #" << i << ": "
			     << conf.outfile_ply[i] << endl;
			return ret;
		}
	}

	/* success */
	toc(clk, "Exporting all data");
	return 0;
}
