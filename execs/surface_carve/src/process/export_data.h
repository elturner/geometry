#ifndef EXPORT_DATA_H
#define EXPORT_DATA_H

/* export_data.h:
 *
 * This file defines functions that will export the computed
 * data of this program to a file. */

#include <vector>
#include "../io/config.h"
#include "../structs/triangulation.h"
#include "../triangulate/region_growing.h"

using namespace std;

/* export_data:
 *
 *	Exports the data of this program to file.
 *
 * arguments:
 *
 * 	tri -	The triangulation that resulted from computation
 * 	rl -	The region list to use when exporting
 * 	conf -	The configuration settings to use
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int export_data(triangulation_t& tri, vector<planar_region_t>& rl, 
						config_t& conf);

#endif
