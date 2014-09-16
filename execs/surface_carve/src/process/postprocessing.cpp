#include "postprocessing.h"
#include <vector>
#include "../io/config.h"
#include "../structs/triangulation.h"
#include "../triangulate/smoothing.h"
#include "../triangulate/union_find.h"
#include "../triangulate/region_growing.h"
#include "../triangulate/simplify.h"
#include "../util/tictoc.h"
#include "../util/error_codes.h"
#include "../test/verify.h"
#include "../test/stats.h"

using namespace std;

int post_processing(triangulation_t& tri, vector<planar_region_t>& regions,
						config_t& conf)
{
	tictoc_t clk;

	/* Remove disjoint unions */
	tic(clk);
	remove_small_unions(tri, MIN_MESH_UNION_SIZE);
	toc(clk, "Reducing mesh");

	/* smoothing */
	tic(clk);
	smoothing_laplace(tri);
	smoothing_laplace(tri);
	smoothing_laplace(tri);
	smoothing_laplace(tri);
	toc(clk, "Smoothing mesh");

	/* perform region growing */
	tic(clk);
	region_grow_all(regions, tri);
	region_grow_coalesce_small(regions);
	region_grow_snap(regions);
	region_grow_coalesce(regions, MIN_SNAP_REGION_SIZE);
	color_by_region(regions);
	toc(clk, "Region growing");
	
	/* simplify triangulation */
	if(conf.simplify)
	{
		tic(clk);
		simplify_triangulation(tri, regions);
		toc(clk, "Simplification");
	}

	/* verify triangulation structure */
	tic(clk);
	if(!verify_triangulation(tri))
		return -1;
	toc(clk, "Verifying");

	/* print statistics -- only run code if want to print statistics */
	#ifdef PRINT_STATISTICS
	print_planar_region_stats(regions);
	#endif

	/* success */
	return 0;
}
