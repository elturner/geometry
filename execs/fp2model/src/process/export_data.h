#ifndef EXPORT_DATA_H
#define EXPORT_DATA_H

/**
 * @file export_data.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Exports floorplan geometry to various file formats
 *
 * @section DESCRIPTION
 *
 * This file contains functions
 * used to export the floorplan
 * data to various formats.
 */

#include "../io/config.h"
#include "../structs/building_model.h"

/**
 * Will export the given data to the specified files from conf.
 *
 * @param bim           The building model to export
 * @param conf          The configuration file
 *
 * @returns             Returns zero on success, non-zero on failure.
 */
int export_data(const building_model_t& bim, const config_t& conf);

#endif
