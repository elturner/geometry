#ifndef SIMPLIFY_H
#define SIMPLIFY_H

/* simplify.h:
 *
 * This file contains functions used to simplify a
 * triangular mesh in 3D space.  Reduction of elements
 * is performed while still attempting to preserve the
 * overall shape of the mesh.
 */

#include <vector>
#include "region_growing.h"
#include "../structs/triangulation.h"

using namespace std;

/* simplify_triangulation:
 *
 * 	Simplifies all regions in the given triangulation.
 *
 * arguments:
 *
 * 	tri -	The triangulation to modify.
 * 	rl -	The list of regions in this triangulation.
 */
void simplify_triangulation(triangulation_t& tri, 
			vector<planar_region_t>& rl);

/* simplify_region:
 *
 * 	Simplifies the given region of a mesh by iterating over the 
 * 	triangles.
 * 	
 * 	If there exist two triangles ta,tb such that:
 * 		- ta,tb share an edge
 * 		- ta,tb are part of the given region
 * 		- all neighbors of both ta,tb are part of the
 * 		  given region
 * 	Then these two triangles are collapsed.  This process
 * 	is performed for as long as such pairs exist.
 */
void simplify_region(triangulation_t& tri, planar_region_t& reg);

/* simplify_set_edge_center:
 *
 * 	Will reset the position of vc to be between
 * 	the current positions of vc and vc, based on
 * 	the the average of the edge's link-ring positions.
 */
void simplify_set_edge_center(vertex_t* vc, vertex_t* vd);

#endif
