#ifndef CSV_IO_H
#define CSV_IO_H

/**
 * @file   csv_io.h
 * @author Eric Turner
 * @brief  Contains functions to export building models to .csv files
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to export building models to
 * .csv files.  These files will contain statistics about the
 * various rooms recovered from the represented floorplan.
 */

#include "../structs/building_model.h"
#include <string>

/**
 * Will export the building model to the specified .csv
 *
 * Given the path to a .csv file, will write out the building
 * information to that file.
 *
 * @param filename  Where to export the .csv file
 *
 * @return     Returns zero on success, non-zero on failure
 */
int writecsv(const std::string& filename, const building_model_t& bm);

#endif
