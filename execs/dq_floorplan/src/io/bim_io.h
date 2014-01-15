#ifndef BIM_IO_H
#define BIM_IO_H

/* bim_io.h:
 *
 * This file represents functions that will
 * generate Building Information Models (BIMs)
 * in various formats.
 *
 * The primary consumer of these functions is
 * EnergyPlus, a building energy simulation tool.
 */

#include "../rooms/tri_rep.h"

/* writeidf:
 *
 * 	Will write a .idf file, which is the
 * 	Input Data File for EnergyPlus.
 *
 * 	This assumes default building materials,
 * 	and does not include windows, doors,
 * 	or furniture.
 *
 * 	Will also include a default building schedule.
 *
 * arguments:
 *
 * 	filename -	Where to write the .idf file.
 *
 *	trirep -	The 2D triangulation used to
 *			create the extruded 3D model.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int writeidf(char* filename, tri_rep_t& trirep);

#endif
