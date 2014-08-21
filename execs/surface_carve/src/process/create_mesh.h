#ifndef CREATE_MESH_H
#define CREATE_MESH_H

/* create_mesh.h:
 *
 * This file contains function(s) to generate
 * a triangulated mesh from a voxel grid.
 */

#include <vector>
#include "../io/config.h"
#include "../structs/dgrid.h"
#include "../structs/triangulation.h"
#include "../triangulate/region_growing.h"

using namespace std;

/* create_mesh:
 *
 * 	Given a grid, will perform planar region growing
 * 	on the voxel faces, triangulate these regions, and
 * 	store the resulting triangles in the specified triangulation.
 *
 * arguments:
 *
 * 	tri -		Where to store the triangulation
 *	regions -	Where to store the resulting planar regions
 * 	grid -		The grid to process
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int create_mesh(triangulation_t& tri, vector<planar_region_t>& regions,
					dgrid_t& grid, config_t& conf);

/************** HELPER FUNCTIONS *******************/

#include "../structs/mesher.h"

/* triangulate_regions:
 *
 *	This is a helper function.
 *
 * 	Given the currently defined regions of faces, and the
 * 	currently defined vertices, will create a triangulation 
 * 	on these vertices that represents these faces.
 *
 * arguments:
 *
 * 	tri -	Where to store the resulting triangulation
 * 	mesh -	The mesh to use to triangulate.  The mesh should
 * 		have well-defined planar regions.
 * 	grid -	The original voxel grid
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int triangulate_regions(triangulation_t& tri, 
			vector<planar_region_t>& regions, mesher_t& mesh, 
						dgrid_t& grid);

/* verts_are_degenerate:
 *
 * This function will return true if the given
 * list of three vertices contain duplicates,
 * or any nulls.
 */
bool verts_are_degenerate(vertex_t** verts);

/* organize_boundary_face_verts:
 *
 * 	Given the vertices at the corners of a boundary
 * 	face, will rotate the vertices so that the diagonal
 * 	edge across the face is always consistant in world
 * 	coordinates for all faces.
 *
 * arguments:
 *
 * 	verts -	A pointer to the length-four array of vertex_t pointers.
 */
void organize_boundary_face_verts(vertex_t** verts);

/* are_tris_duplicate:
 *
 *	Checks if the two triangles provided have the same vertices,
 *	just possibly in a different ordering.
 *
 * arguments:
 *
 * 	a,b -	The triangles to analyze
 *
 * return value:
 *
 * 	Returns true if a and b have the same vertices.
 */
bool are_tris_duplicate(triangle_t* a, triangle_t* b);

/* remove_double_surfacing:
 *
 *	Removes any triangles from the specified triangulation
 *	that exhibit double-surfacing behavior.  WARNING, this
 *	may cause gaps in the triangulation!
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int remove_double_surfacing(triangulation_t& tri);

/* coalesce_small_regions:
 *
 *	Performs the same operation as 
 *	triangulate/region_growing.cpp::region_grow_coalesce
 *
 *	The difference is that this function does not rely on the
 *	referenced triangles to have valid neighbor pointers.
 *
 *	The other difference is that this function depicts "small"
 *	regions by surface area, not triangle count, so it is appropriate
 *	for regions whose triangles are not uniformly sized.
 *
 *	This implementation is less efficient than the other, so
 *	if these pointers are valid, it is recommended that the other
 *	is used.
 */
void coalesce_small_regions(vector<planar_region_t>& rl, 
					double min_surface_area);

#endif
