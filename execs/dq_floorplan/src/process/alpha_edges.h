#ifndef ALPHA_EDGES_H
#define ALPHA_EDGES_H

/* This file defines functions which will compute
 * the alpha edges of an input model, stored as
 * a dynamic quadtree. */

#include "../structs/quadtree.h"
#include "../structs/path.h"
#include "../structs/cell_graph.h"

/* process_alpha_edges:
 *
 *	Initialize and performs computation on a cell graph to
 *	yield a set of alpha edges.
 *
 * arguments:
 *
 * 	graph -	The graph to create.
 * 	tree -	The tree to reference in creating the graph.
 *	path -	The path of the scanner through the geometry.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int process_alpha_edges(cell_graph_t& graph, quadtree_t& tree,
						path_t& path);

#endif
