#ifndef POPULATE_HISTOGRAM_H
#define POPULATE_HISTOGRAM_H

/* populate_histogram.h:
 *
 * 	This file contains functions used to
 * 	generate a height histogram from an input
 * 	set of scans.
 */

#include "../io/config.h"
#include "../structs/histogram.h"

/* populate_histogram:
 *
 * 	Will parse the specified scan files in the config
 * 	struct, and will initialize and populate the specified
 * 	histogram with their heights.
 *
 * arguments:
 *
 * 	floors -	The histogram to generate for floor estimates
 * 	ceilings -	The histogram to generate for ceiling estimates
 * 	conf -		The config file that specifies input scans
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int populate_histogram(histogram_t& floors, histogram_t& ceilings,
				config_t& conf);

#endif
