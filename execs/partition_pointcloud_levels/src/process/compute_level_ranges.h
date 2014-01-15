#ifndef COMPUTE_LEVEL_RANGES_H
#define COMPUTE_LEVEL_RANGES_H

/* compute_level_ranges.h:
 *
 * 	Given histograms of building scans,
 * 	will determine how many stories are
 * 	in the building, and what the ranges
 * 	are for these stories.
 */

#include <vector>
#include "../io/config.h"
#include "../structs/histogram.h"

using namespace std;

/* compute_level_ranges:
 *
 * 	By specifying a floor and ceiling vertical
 * 	histogram generated from building scans,
 * 	will populate the floor and ceiling lists
 * 	that denote the number of levels and the
 * 	floor and ceiling height of each level.
 *
 * arguments:
 *
 * 	floor_heights -	The output list of floor heights
 * 	ceil_heights -	The output list of ceiling heights
 * 	floor_hist -	The input histogram for the floor
 * 	ceil_hist -	The input histogram for the ceiling
 *	conf -		The configuration to use
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int compute_level_ranges(vector<double>& floor_heights, 
			vector<double>& ceil_heights, 
			histogram_t& floor_hist, 
			histogram_t& ceil_hist,
			config_t& conf);

#endif
