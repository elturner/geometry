#ifndef FP_IO_H
#define FP_IO_H

/* fp_io.h:
 *
 * 	This file contains functions used to generate
 * 	.fp files, which contain full information about
 * 	the floor plan.
 *
 * File Format for .fp (all units in meters)
 * -----------------------------------------
 *
 * 	<resolution>		// the resolution of model, in meters
 *	<num_verts>
 *	<num_tris>
 *	<num_rooms>
 *	<x1> <y1>		// first vertex position (two doubles)
 *	...
 *	<xn> <yn>		// n'th vertex position (two doubles)
 *	<i1> <j1> <k1>		// first triangle (three ints)
 *	...
 *	<im> <jm> <km>		// m'th triangle (three ints)
 *	<z1_min> <z1_max> <num_tris> <t_1> <t_2> ... <t_k1>	// room 1
 *	...
 *	<zr_min> <zr_max> <num_tris> <t_1> <t_2> ... <t_kr>	// room r
 *
 */

#include "../rooms/tri_rep.h"

/* write_fp:
 *
 * 	Will write a .fp file to the specified location, using
 * 	the specified triangle representation.
 *
 * arguments:
 *
 * 	filename -	Where to write the .fp file
 * 	trirep -	The information to write
 * 	res -		The resolution of this model
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int write_fp(char* filename, tri_rep_t& trirep, double res);

#endif
