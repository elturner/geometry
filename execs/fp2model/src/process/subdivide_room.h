#ifndef SUBDIVIDE_ROOM_H
#define SUBDIVIDE_ROOM_H

/**
 * @file subdivide_room.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Creates functions used to subdivide floorplan rooms
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to subdivide the geometry of
 * the rooms of floorplans.  These functions are necessary for exporting
 * to file formats that have a limit on the complexity of surfaces,
 * such as EnergyPlus IDF format.
 */

#include <mesh/floorplan/floorplan.h>

/**
 * Will bisect a room into two roughly equal subrooms
 *
 * This function will populate the subroom triangle assignments, so
 * that the original room's triangles are partitioned into two subrooms,
 * in an attempt to make the subrooms roughly equal area.
 *
 * This function is NOT optimized, is very heuristical, and is NOT meant
 * to be a representative example of the author's typical quality of work.
 *
 * @param a   The first subroom structure to populate
 * @param b   The second subroom structure to populate
 * @param r   The original room to subdivide.  Does not necessarily have to
 *            be an original room of f.
 * @param f   The floorplan to analyze.
 */
void bisect_room(fp::room_t& a, fp::room_t& b, const fp::room_t& r,
                 const fp::floorplan_t& f);

/*---------------- helper functions -------------------*/

/**
 * Will find the two triangles within a room that are farthest apart
 *
 * This triangles can be used as seeds for partitioning the room into
 * two sections.
 *
 * @param ai    Where to store the index of the first found triangle
 * @param bi    Where to store the index of the second found triangle
 * @param r     The room to analyze
 * @param f     The floorplan that is referenced by r
 */
void get_seeds(int& ai, int& bi, const fp::room_t& r,
               const fp::floorplan_t& f);

/**
 * Partition a set of triangles in two based on two seed triangles
 *
 * Given the indices of two 'seed' triangles, will attempt to partition
 * a given set of triangles from a floorplan into two, roughly equal
 * subsets.  This is done in a greedy fashion until all triangles are
 * sorted.
 *
 * @param a   The first subroom structure to populate
 * @param b   The second subroom structure to populate
 * @param r   The original room to subdivide.  Does not necessarily have to
 *            be an original room of f.
 * @param ai  The index of the first seed triangle
 * @param bi  The index of the second seed triangle
 * @param f   The floorplan to analyze.
 */
void partition_tri_sets(fp::room_t& a, fp::room_t& b, const fp::room_t& r,
                        int ai, int bi, const fp::floorplan_t& f);

#endif
