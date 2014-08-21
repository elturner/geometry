#include "export_data.h"
#include <vector>
#include <iostream>
#include "../io/mesh_io.h"
#include "../io/region_io.h"
#include "../structs/triangulation.h"
#include "../triangulate/region_growing.h"
#include "../util/tictoc.h"
#include "../util/error_codes.h"

using namespace std;

int export_data(triangulation_t& tri, vector<planar_region_t>& rl,
							config_t& conf)
{
	tictoc_t clk;
	int ret;

	/* determine what format to output in */
	switch(conf.output_type)
	{
		case obj_file:
			/* write to Wavefront Object File */
			tic(clk);
			ret = writeobj(conf.outfile, tri);
			if(ret)
			{
				cerr << "Could not write "
				     << "Wavefront Object File: "
				     << conf.outfile << endl;
				return PROPEGATE_ERROR(-1, ret);
			}
			toc(clk, "Exporting surface to OBJ");
			break;
		case ply_file:
			/* write to Stanford Polygon File */
			tic(clk);
			ret = (conf.output_ascii) ?
				writeply_with_regions(conf.outfile, 
					tri, rl, conf.output_ascii)
				: writeply(conf.outfile, tri, 
					conf.output_ascii);
			if(ret)
			{
				cerr << "Could not write "
				     << "Stanford Polygon File: "
				     << conf.outfile << endl;
				return PROPEGATE_ERROR(-2, ret);
			}
			toc(clk, "Exporting surface to PLY");
			break;
		default:
			/* unknown file type */
			cerr << "[export_data]\tUnknown output file type."
			     << endl;
			return -3;
	}

	/* success */
	return 0;
}
