#ifndef SHP_IO_H
#define SHP_IO_H

/**
 * @file   shp_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Export model to .shp file
 *
 * @section DESCRIPTION
 *
 * This file contains functions to export floorplans to .shp files.
 * This file format is defined here:
 *
 * http://en.wikipedia.org/wiki/Shapefile#Shapefile_shape_format_.28.shp.29
 */

#include "../structs/building_model.h"
#include <string>

/**
 * Will export building model to .shp file
 *
 * Will write to the specified .shp file, including all building
 * information.  This file is represented by a binary format, but
 * does not include any topology information.
 *
 * @param filename    Where to write the .shp file
 * @param bim         The building information model to export
 *
 * @return            Returns zero on success, non-zero on failure.
 */
int writeshp(const std::string& filename, const building_model_t& bim);

#endif
