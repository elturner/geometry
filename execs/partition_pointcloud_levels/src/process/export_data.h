#ifndef EXPORT_DATA_H
#define EXPORT_DATA_H

/* export_data.h:
 *
 * This file contains functions
 * used to export the level partitioning
 * data, including partitioning the point
 * cloud and exporting the level values
 */

#include <vector>
#include "../io/config.h"
#include "../structs/point.h"
#include "../structs/histogram.h"

using namespace std;

/* export_data:
 *
 * 	Will export the given data to the specified files,
 * 	which includes partitioning scans into xyz files
 * 	by floor, which requires the input scans files to
 * 	be read.
 *
 * arguments:
 *
 *	floor_heights -	The list of floors and their heights
 *	ceil_heights -	The list of ceilings and their heights,
 *			must be the same length as floor_heights
 *	floor_hist -	The floors histogram
 *	ceil_hist -	The ceilings histogram
 *	conf -		The configuration file
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int export_data(vector<double>& floor_heights, vector<double>& ceil_heights,
		histogram_t& floor_hist, histogram_t& ceil_hist,
		config_t& conf);

/********************** Helper Functions ****************************/

/* export_matlab_script:
 *
 * 	Will attempt to export a matlab script that contains all
 * 	relevant information about the level partitioning computed.
 *
 * arguments:
 *
 *	floor_heights -	The list of floors and their heights
 *	ceil_heights -	The list of ceilings and their heights,
 *			must be the same length as floor_heights
 *	floor_hist -	The floors histogram
 *	ceil_hist -	The ceilings histogram
 *	conf -		The configuration file
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int export_matlab_script(vector<double>& floor_heights, 
			vector<double>& ceil_heights,
			histogram_t& floor_hist, histogram_t& ceil_hist,
			config_t& conf);

/* partition_scans:
 *
 * 	Will read in the scans, and rewrite the points to the specified
 * 	output locations, partitioning the points based on which level
 * 	they are in.
 *
 * arguments:
 *
 *	floor_heights -	The list of floors and their heights
 *	ceil_heights -	The list of ceilings and their heights,
 *			must be the same length as floor_heights
 *	conf -		The configuration file
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int partition_scans(vector<double>& floor_heights, 
			vector<double>& ceil_heights,
			config_t& conf);

/* level_of_point:
 *
 * 	Will specify the level index for the given point.
 *
 * arguments:
 *
 * 	p -				The point to analyze
 * 	floor_heights, ceil_heights -	The level heights to use
 *
 * return value:
 *
 * 	Returns the level of the point p.
 */
int level_of_point(point_t& p, vector<double>& floor_heights, 
				vector<double>& ceil_heights);

#endif
