#ifndef SIMPLIFY_GRAPH_H
#define SIMPLIFY_GRAPH_H

/* simplify_graph.h:
 *
 * 	This file defines functions used for
 * 	graph simplification.
 */

#include "../structs/cell_graph.h"
#include "../rooms/tri_rep.h"

/* simplify_graph:
 *
 *	Will simplify the graph using 2D error quadrics
 *	(should really be triprics?) as well as parallel
 *	thresholding.
 *
 *	The given threshold is used for the QEM simplification.
 *	If this threshold is negative, no simplification will
 *	be performed.
 *
 * arguments:
 *
 * 	graph	-	The graph to simplify
 * 	trirep -	The tri rep to simplify
 * 	thresh	-	The threshold to use.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int simplify_graph(cell_graph_t& graph, tri_rep_t& trirep, double thresh);

#endif
