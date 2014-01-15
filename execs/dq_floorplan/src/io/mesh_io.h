#ifndef MESH_IO_H
#define MESH_IO_H

/* mesh_io.h:
 *
 * This file defines functions that export 3D meshes.
 */

#include "../rooms/tri_rep.h"

/* writeobj:
 *
 * 	Writes a Wavefront OBJ file to the specified location.
 *
 * arguments:
 *
 * 	filename -	The location to write the file
 * 	trirep -	The triangulation of the floors and
 * 			ceilings to write.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int writeobj(char* filename, tri_rep_t& trirep);

/* writeobj_2d:
 *
 * 	Writes a Wavefront OBJ file to the specified location.
 * 	Only writes floor triangles
 *
 * arguments:
 *
 * 	filename -	The location to write the file
 * 	trirep -	The triangulation of the floors and
 * 			ceilings to write.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int writeobj_2d(char* filename, tri_rep_t& trirep);

/* writeedge:
 *
 * 	Writes a .edge file from the specified triangle representation.
 *
 * arguments:
 *
 * 	filename -	The location to write the file
 * 	trirep -	The triangulation of floors to write.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int writeedge(char* filename, tri_rep_t& trirep);

/* writeply:
 *
 * 	Writes a Stanford PLY file of the 3D extruded mesh to
 * 	the specified location.  Will also included extra properties
 * 	that indicate planar region information.
 *
 * arguments:
 *
 * 	filename -	The location of the file to write
 * 	trirep -	The triangulation of the floors and ceilings
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int writeply(char* filename, tri_rep_t& trirep);

#endif
