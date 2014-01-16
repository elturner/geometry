#ifndef EXPORT_DATA_H
#define EXPORT_DATA_H

/* export_data.h:
 *
 * This file contains functions
 * used to export the floorplan
 * data to various formats.
 */

#include "../io/config.h"
#include "../structs/building_model.h"

/* export_data:
 *
 * 	Will export the given data to the specified files
 * 	from conf.
 *
 * arguments:
 *
 *	bim -		The building model to export
 * 	conf -		The configuration file
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int export_data(building_model_t& bim, config_t& conf);

#endif
