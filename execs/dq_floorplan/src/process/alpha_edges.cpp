#include "alpha_edges.h"
#include "../structs/quadtree.h"
#include "../structs/path.h"
#include "../structs/cell_graph.h"
#include "../util/error_codes.h"

#include <fstream>
using namespace std;

int process_alpha_edges(cell_graph_t& graph, quadtree_t& tree, path_t& path)
{
	int ret;

	/* create the graph */
	ret = graph.populate(tree);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	
	/* erode cells */
	ret = graph.erode(path, tree);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* create initial edges */
	graph.map_neighbors(tree);
	graph.remove_time_spanning_edges();
	graph.reduce_cliques();
	
	/* simplify straight walls */
	graph.simplify_extraordinary_cells();
	graph.remove_extraordinary_paths_below(0.2);
	graph.simplify_straights();
	graph.remove_interior_cells();

	/* success */
	return 0;
}
