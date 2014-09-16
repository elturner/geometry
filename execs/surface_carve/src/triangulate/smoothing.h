#ifndef SMOOTHING_H
#define SMOOTHING_H

/* smoothing.h:
 *
 * This file defines functions to be called on 
 * triangulations.
 *
 * All functions assume that triangulation_map_neighbors()
 * has already been called on the input triangulation.
 */

#include "../structs/triangulation.h"

/* smoothing_laplace:
 *
 *	Performs one round of laplacian smoothing 
 *	on the input triangulation.
 *
 *	Current implementation does in-place smoothing,
 *	which is dependent on the ordering of vertices,
 *	and is faster but not as ideal.
 */
void smoothing_laplace(triangulation_t& tri);

/* smoothing_laplace_in_region:
 *
 *	Performs one round of laplacian smoothing 
 *	on the input triangulation.  Will only smooth
 *	vertices that are fully contained in a region,
 *	marked by triangle indices.
 *
 *	Current implementation does in-place smoothing,
 *	which is dependent on the ordering of vertices,
 *	and is faster but not as ideal.
 */
void smoothing_laplace_in_region(triangulation_t& tri);

/* smoothing_laplace_region_edges:
 *
 *	Performs one round of laplacian smoothing only
 *	on the vertices that occupy the edges of regions
 *	(i.e. are incident with triangles from multiple
 *	regions).
 */
void smoothing_laplace_region_edges(triangulation_t& tri);

#endif
