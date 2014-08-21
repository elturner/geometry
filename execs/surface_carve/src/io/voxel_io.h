#ifndef VOXEL_IO_H
#define VOXEL_IO_H

/* voxel_io.h:
 *
 * The file defines functions that read and write voxel formatted files.
 *
 * Voxel files have the following ASCII format:
 *
 * 	<vs>
 * 	<x1> <y1> <z1> <s1>
 * 	<x2> <y2> <z2> <s2>
 * 	...
 * 	...
 *
 * Where:
 *
 * 	<vs> is the voxel size, as a double, in meters
 * 	
 * 	<xi>,<yi>,<zi> specify the integer grid positions of this voxel
 * 	
 * 	<si> specifies the state of the voxel.
 */

#include "../structs/dgrid.h"

/* readvox:
 *
 * 	Reads the specified file, and stores the information
 * 	in the provided grid struct.  Any information in g
 * 	before this call will be lost.
 *
 * arguments:
 *
 * 	filename -	Where to read the grid information from.
 * 	g -		Where to store the information from the
 * 			file.
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int readvox(char* filename, dgrid_t& g);

/* writevox:
 *
 *	Writes the information stored in the provided
 *	grid to the specified file.
 *
 * arguments:
 *
 * 	filename -	Where to write to disk
 * 	g -		The grid whose information to write.
 * 
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int writevox(char* filename, dgrid_t& g);

#endif
