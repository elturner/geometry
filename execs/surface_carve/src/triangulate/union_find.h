#ifndef UNION_FIND_H
#define UNION_FIND_H

/* union_find.h:
 *
 * This file defines functions that are used
 * to perform union find on a triangulation.
 */

#include <vector>
#include "../structs/triangulation.h"

using namespace std;

/* remove_small_unions:
 *
 * 	Removes triangles that are in small, isolated clusters.
 *
 * arguments:
 *
 * 	tri -	The triangulation to modify
 * 	tus -	Threshold union size.  Any unions with fewer than
 * 		this many triangles will be removed.
 */
void remove_small_unions(triangulation_t& tri, int tus);

/* color_by_union:
 *
 *	Defines the color field in each vertex by
 *	the union its triangles are associated with.
 */
void color_by_union(triangulation_t& tri);

/* union_find:
 *
 *	Performs union-find on the given triangulation.
 *
 * arguments:
 *
 * 	unions -	Where to store the result.  Each element is
 * 			a list of triangle indices.
 *
 * 	tri -		The triangulation to analyze.
 */
void union_find(vector<vector<int> >& unions, triangulation_t& tri);

/* get_root:
 *
 *	Helper function for union-find.  Gets the root of i, and simplifies
 *	forest.
 */
int get_root(vector<int>& forest, int i);

#endif
