#ifndef POSE_IO_H
#define POSE_IO_H

/* pose_io.h:
 *
 * Defines functions for reading/writing pose
 * information from files */

#include <vector>
#include "../structs/pose.h"

using namespace std;

/* readmad:
 *
 * 	Reads a *.mad file, and stores results in
 * 	specified pose list.
 *
 * arguments:
 *
 * 	filename -	Local path to file to parse
 * 	pl -		Where to store file contents
 *
 * return value:
 *
 *	Returns 0 on success, non-zero on failure.
 */
int readmad(char* filename, vector<pose_t>& pl);

#endif
