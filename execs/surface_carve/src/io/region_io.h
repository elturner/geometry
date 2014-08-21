#ifndef REGION_IO_H
#define REGION_IO_H

/* the following functions are used to
 * output planar region information to disk. */

#include <vector>
#include "../structs/triangulation.h"
#include "../triangulate/region_growing.h"

using namespace std;

/* writeply_with_regions:
 *
 *	Writes the full mesh to the output PLY file,
 *	including region information.
 *
 * arguments:
 * 	
 * 	filename -	Path to file to write
 * 	tri -		The triangulation to write to disk
 * 	rl -		The region list to use
 *	ascii -		If true, will output in ascii format.
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int writeply_with_regions(char* filename, triangulation_t& tri, 
				vector<planar_region_t>& rl, bool ascii);

#endif
