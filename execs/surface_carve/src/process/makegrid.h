#ifndef MAKE_GRID_H
#define MAKE_GRID_H

/* makegrid.h:
 *
 * This file defines the high-level functions
 * used to carve a grid from a series of laser
 * scans and pose information.
 */

#include "../structs/dgrid.h"
#include "../io/config.h"

/* make_grid:
 *
 * 	Will populate the given grid with appropriate voxel
 * 	values based on the files provided in the configuration
 * 	structure.
 *
 * arguments:
 *
 * 	grid -	The grid to create and populate
 * 	conf -	The configuration settings to use (filenames
 * 		included).
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int make_grid(dgrid_t& grid, config_t& conf);

#endif
