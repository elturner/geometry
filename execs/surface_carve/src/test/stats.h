#ifndef STATS_H
#define STATS_H

/* stats.h:
 *
 * This file contains functions which analyze and print
 * various statistics about the data structures passed to
 * them.
 */

#include <vector>
#include "../triangulate/region_growing.h"

using namespace std;

/* print_planar_region_stats:
 *
 * 	Prints the statistics of the given planar regions.
 */
void print_planar_region_stats(vector<planar_region_t>& regions);

#endif
