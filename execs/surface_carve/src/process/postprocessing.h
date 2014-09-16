#ifndef POST_PROCESSING_H
#define POST_PROCESSING_H

/* postprocessing.h:
 *
 * 	This file contains functions that encapsulate
 * 	the post-processing steps after triangulation
 * 	is performed to the carved space.
 */

#include "../io/config.h"
#include "../structs/triangulation.h"
#include "../triangulate/region_growing.h"

/* post_processing:
 *
 *	This function performs the desired post-processing
 *	steps on the given triangulation.
 *
 *	Included are smoothing, region-growing, plane fitting,
 *	and optionally simplification of the mesh.
 *
 * arguments:
 *
 * 	tri -		The mesh to process
 * 	regions -	The list to store the planar regions found
 *	conf -		The configuration to use.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int post_processing(triangulation_t& tri, vector<planar_region_t>& regions,
						config_t& conf);

#endif
