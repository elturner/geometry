#ifndef GENERATE_BOUNDARY_H
#define GENERATE_BOUNDARY_H

/* generate_boundary.h:
 *
 * This file contains functions used to construct a
 * Delaunay Triangulation from the specified cell graph's vertices,
 * determine which triangles are interior and exterior, find the
 * boundary edges between these labelings, and
 * then store the resulting edges back into the graph structure.
 */

#include <set>
#include "../structs/cell_graph.h"
#include "../structs/quadtree.h"
#include "../structs/path.h"
#include "../structs/triple.h"
#include "../rooms/tri_rep.h"
#include "../delaunay/triangulation/triangulation.h"

using namespace std;

/* generate_boundary:
 *
 *	Will populate the specified graph with cells and edges that form
 *	a watertight boundary of the geometry described by the specified
 *	quadtree.
 *
 *	The tree should contain samples consistant with the specified path.
 *	
 * arguments:
 *
 * 	graph -		The graph to populate and modify.
 *	trirep -	The triangulation to populate with room-labeled
 *			interior triangles.
 *	tree -		The tree to use to populate graph.
 *	path -		The path used to generate tree.
 *	carve_through -	Boolean indicates whether to use occlusion testing
 *			when raytracing (if false) or not to use occlusion
 *			testing (if true)
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int generate_boundary(cell_graph_t& graph, tri_rep_t& trirep,
				quadtree_t& tree, path_t& path,
				bool carve_through);

/************* HELPER FUNCTIONS ******************/

/* triangulate_graph:
 *
 * 	Will use the existing vertices in the graph to
 * 	form a delaunay triangulation.  The edges in the
 * 	graph will be ignored.
 *
 * 	The resulting triangles will be stored in the graph
 * 	structure.
 *
 * arguments:
 *
 * 	tri -	The triangulation that will be initialized and computed.
 *
 * 	graph -	The graph whose cells will form the vertices of the
 * 		triangulation.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int triangulate_graph(triangulation_t& tri, cell_graph_t& graph);

/* label_triangulation:
 *
 *	Will determine which triangles of the specified triangulation
 *	are interior, and which are exterior.
 *	
 *	This triangulation should be formed from the cells of the
 *	specified graph, with the corresponding path.
 *
 * arguments:
 *
 * 	interior -	The set in which the interior triangles will
 * 			be stored as triples of indices into tri.
 *
 *	visited -	The set of triangles that are intersected by
 *			path of the scanner.
 *
 * 	path -		The path traversed during scanning
 *
 * 	graph -		The graph, populated with cells.  These cells
 * 			should be the vertices of the triangulation.
 *
 * 	tri -		The triangulation to label.
 *
 *	tree -		the tree is needed to help with ray-tracing
 *
 *	carve_through -	Boolean indicates whether to use occlusion testing
 *			when raytracing (if false) or not to use occlusion
 *			testing (if true)
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int label_triangulation(set<triple_t>& interior, set<triple_t>& visited,
				path_t& path,
				cell_graph_t& graph, triangulation_t& tri,
				quadtree_t& tree, bool carve_through); 

/* raytrace_triangulation:
 *
 *	Will find all triangles in tri that intersect the line segment
 *	denoted by start,end.
 *
 *	Note:  for all triple_t's used in this call, the values within
 *		them should be indices into tri's vertex list.  In other
 *		words, they should be unsigned ints cast to void*.
 *
 * arguments:
 *
 * 	found_tris -	Any intersecting triangles will be added to this set
 * 	tri -		The triangulation to analyze.
 *	start, end -	The endpoints of the line segment to trace
 *	st -		An optionally defined triangle containing the
 *			start point.  Specifying the correct triangle
 *			will save on computation.  If invalid triangle
 *			given, the correct starting triangle will be
 *			stored here.
 *	et -		This will be set to the triangle that contains
 *			the point 'end'
 *
 * return value:
 *
 *	Returns zero on success, non-zero on failure.
 */
int raytrace_triangulation(set<triple_t>& found_tris, triangulation_t& tri,
					vertex_t& start, vertex_t& end,
					triple_t& st, triple_t& et);

/* add_boundary_edges_to_graph:
 *
 *	Given a valid partitioning of inside/outside triangles,
 *	will add the boundary edges of this partitioning to the
 *	specified graph, which should have as cells the vertices of
 *	the triangulation.
 *
 * arguments:
 *
 * 	trirep		-	The representation of interior
 * 				triangles in this triangulation.
 * 	tri		-	The triangulation to analyze
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int add_boundary_edges_to_graph(tri_rep_t& trirep,
					triangulation_t& tri);

#endif
