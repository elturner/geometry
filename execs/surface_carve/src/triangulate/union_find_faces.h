#ifndef UNION_FIND_FACES_H
#define UNION_FIND_FACES_H

/* union_find_faces.h:
 *
 * This file defines functions that are used
 * to perform union find on a mesh of voxel faces.
 */

#include <vector>
#include "../structs/mesher.h"

using namespace std;

/* remove_small_unions_faces:
 *
 * 	Removes voxel faces that are in small, isolated clusters.
 *
 * arguments:
 *
 * 	mesh -	The mesh to modify
 * 	tus -	Threshold union size.  Any unions with fewer than
 * 		this many triangles will be removed.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int remove_small_unions_faces(mesher_t& mesh, int tus);

/* union_find_faces:
 *
 *	Performs union-find on the given mesh of voxel faces.
 *
 * arguments:
 *
 * 	unions -	Where to store the result.  Each element is
 * 			a list of faces.
 *
 * 	mesh -		The mesh to analyze.
 * 
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int union_find_faces(vector<vector<face_t> >& unions, mesher_t& mesh);

/* get_root_faces:
 *
 *	Helper function for union-find.  Gets the root of i, and simplifies
 *	forest.
 */
int get_root_faces(vector<int>& forest, int i);

#endif
