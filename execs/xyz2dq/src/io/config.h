#ifndef CONFIG_H
#define CONFIG_H

/* config.h:
 *
 * 	Represents the functions used
 * 	to read command-line arguments.
 */

#include "filetypes.h"

#define MAX_POINTCLOUD_FILES 10

typedef struct config
{
	/*** input files configuration ***/
	
	/* the list of input point-cloud files */
	char* pc_infile[MAX_POINTCLOUD_FILES];
			/* location of input point-cloud file, *.xyz */
	unsigned int num_pc_files;

	char* mad_infile; /* location of input mad file */

	/*** processing paramters ***/

	double resolution; /* resolution of quadtree */
	
	int min_wall_num_points; /* min points per wall sample threshold */
	double min_wall_height; /* min wall height for this run */

	/*** output files configuration ***/

	char* outfile; /* location to write output */

} config_t;

/* parseargs:
 *
 *	Reads the input command-line arguments and
 *	fills the config_t struct.
 *
 * arguments:
 *
 * 	argc,argv -	What was given to main()
 * 	conf -		Where to store arguments
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int parseargs(int argc, char** argv, config_t& conf);

/* print_usage:
 *
 * 	Prints the usage of this program to screen.
 */
void print_usage(char* prog_name);

/* print_usage_short:
 *
 *	Prints a very short message about the program.
 */
void print_usage_short(char* prog_name);

#endif
