#ifndef MESH_IO_H
#define MESH_IO_H

/* mesh_io.h:
 *
 * 	This file defines functions used to export
 * 	meshes (triangulations) to disk.
 */

#include "../structs/triangulation.h"

/* writeobj:
 *
 * 	Writes triangulation to disk as a Wavefront Object File (*.obj).
 *
 * arguments:
 *
 * 	filename -	Path to file to write
 * 	tri -		The triangulation to write to disk
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int writeobj(char* filename, triangulation_t& tri);

/* writeobj_sorted:
 *
 * 	Writes triangulation to disk as a Wavefront Object File (*.obj).
 * 	The triangles are written out in an order that promotes sorting
 * 	by distance.  This is performed by a breadth-first search across
 * 	the mesh.
 *
 * 	The index fields in each triangle will be destroyed.
 *
 * arguments:
 *
 * 	filename -	Path to file to write
 * 	tri -		The triangulation to write to disk
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int writeobj_sorted(char* filename, triangulation_t& tri);

/* writeply:
 *
 *	Writes the triangulation as a Stanford Polygon File (*.ply).
 *
 * arguments:
 *
 * 	filename -	Path to file to write
 * 	tri -		The triangulation to write to disk
 *	ascii -		If true, will output in ascii format.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int writeply(char* filename, triangulation_t& tri, bool ascii);

#endif
