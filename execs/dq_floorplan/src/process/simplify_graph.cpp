#include "simplify_graph.h"
#include "../structs/cell_graph.h"
#include "../rooms/tri_rep.h"
#include "../util/tictoc.h"
#include "../util/room_parameters.h"
#include "../util/error_codes.h"

int simplify_graph(cell_graph_t& graph, tri_rep_t& trirep, double thresh,
				bool simpdoor)
{
	tictoc_t clk;
	int ret;

	/* check valid threshold */
	tic(clk);
	if(thresh < 0)
		return 0; /* do nothing */

	/* remove sharps first, because we don't wnat them to
	 * be mistaken for valid features by QEM */
	ret = graph.remove_sharps(trirep, 
			DEFAULT_SHARPS_REMOVAL_THRESHOLD, simpdoor);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* simplify graph using error quadrics */
	ret = graph.simplify(trirep, thresh, simpdoor);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* straighten walls */
	ret = graph.simplify_straights(trirep, simpdoor);
	if(ret)
		return PROPEGATE_ERROR(-3, ret);

	/* remove unnecessary unions */
	// graph.remove_unions_below(MIN_ROOM_PERIMETER);
	ret = trirep.remove_interroom_columns(MIN_ROOM_PERIMETER);
	if(ret)
		return PROPEGATE_ERROR(-4, ret);
	toc(clk, "Simplifying model");

	/* verify that triangulation topology still valid */
	tic(clk);
	ret = trirep.verify();
	if(ret)
		return PROPEGATE_ERROR(-5, ret);
	toc(clk, "Verifying topology");

	/* success */
	return 0;
}
