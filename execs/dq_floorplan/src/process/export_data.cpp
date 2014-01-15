#include "export_data.h"
#include <fstream>
#include <set>
#include <vector>
#include "../structs/cell_graph.h"
#include "../rooms/tri_rep.h"
#include "../io/config.h"
#include "../io/mesh_io.h"
#include "../io/bim_io.h"
#include "../io/fp_io.h"
#include "../util/error_codes.h"

using namespace std;

int export_data(cell_graph_t& graph, tri_rep_t& trirep, config_t& conf)
{
	int ret;

	/* make sure cells are indexed */
	graph.remove_outliers();
	graph.index_cells();

	/* how we export depends on the file format specifieid */
	switch(conf.output_type)
	{
		case obj_file:
			/* write the mesh */
			if(conf.export_2d)
				ret = writeobj_2d(conf.outfile, trirep);
			else
				ret = writeobj(conf.outfile, trirep);
			
			/* check error */
			if(ret)
				return PROPEGATE_ERROR(-1, ret);
			break;

		case ply_file:
			/* write the mesh with region info */
			ret = writeply(conf.outfile, trirep);
			if(ret)
				return PROPEGATE_ERROR(-2, ret);
			break;

		case idf_file:
			/* write energy plus idf file */
			ret = writeidf(conf.outfile, trirep);
			if(ret)
				return PROPEGATE_ERROR(-3, ret);
			break;

		case fp_file:
			/* write this floor-plan file */
			ret = write_fp(conf.outfile, trirep, 
					conf.simplify_threshold);
			if(ret)
				return PROPEGATE_ERROR(-4, ret);
			break;

		case edge_file:
			/* write edge file */
			ret = writeedge(conf.outfile, trirep);
			if(ret)
				return PROPEGATE_ERROR(-5, ret);
			break;

		default:
			/* don't know how to export to this format */
			PRINT_ERROR("Output format not implemented for:");
			PRINT_ERROR(conf.outfile);
			PRINT_ERROR("");
			return -6;
	}

	/* success */
	return 0;
}
