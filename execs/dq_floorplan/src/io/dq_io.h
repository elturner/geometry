#ifndef DQ_IO_H
#define DQ_IO_H

/* This file defines functions to read and write from *.dq files */

#include "../structs/quadtree.h"

/* read_dq:
 *
 * 	Reads a *.dq file, and stores it in the
 * 	specified quadtree struct.
 *
 * 	Any information in the struct will be destroyed.
 * 
 * arguments:
 *
 * 	filename -	The location of the file to read.
 * 	tree -		The tree into which the data will be read.
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int read_dq(char* filename, quadtree_t& tree);

/* write_dq:
 *
 * 	Writes the quadtree to the specified *.dq file.
 *
 * arguments:
 *
 * 	filename -	Where to write the data
 * 	tree -		The data to write
 *
 * return value:
 *
 * 	Returns zero on success, non-zero on failure.
 */
int write_dq(char* filename, quadtree_t& tree);

#endif
