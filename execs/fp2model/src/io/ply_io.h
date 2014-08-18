#ifndef PLY_IO_H
#define PLY_IO_H

/**
 * @file   ply_io.h
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Export floorplan information to a PLY file
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to export floorplan information
 * to a PLY file.  This file type is the Stanford Polygon Format,
 * which is used to represented 3D meshes.  This file type is
 * also necessary for Peter Cheng's texture-mapping code.
 */

#include "../structs/building_model.h"
#include <string>

/**
 * Export floorplan to PLY
 *
 * Writes a Stanford PLY file of the 3D extruded mesh to
 * the specified location.  Will also included extra properties
 * that indicate planar region information.
 *
 * @param filename      The location of the file to write
 * @param bim           The building information model
 *
 * @return Returns zero on success, non-zero on failure.
 */
int writeply(const std::string& filename, const building_model_t& bim);

#endif
