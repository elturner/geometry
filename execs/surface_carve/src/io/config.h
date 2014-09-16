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
	bool chunk_pc_files; /* if true, will only read subsets of
				pointcloud files at a time */

	char* mad_infile; /* location of input mad file */
	char* bcfg_infile; /* optional: location of scanner config file */

	/*** output files configuration ***/

	char* outfile; /* location to write output */
	filetype_t output_type; /* what format to output in */
	bool output_ascii; /* if true, will attempt to output ascii */

	/*** carving configuration ***/

	double resolution; /* meters */
	
	bool point_occlusions; /* if true, scan points occlude carving */

	char* voxfile; /* optional file that specifies voxel values */

	/* if a voxel file is specified, should it be read from
	 * or written to? */
	bool readvox;

	/* how much of the dataset to process */
	int begin_pose;
	int num_poses;
	int downsample_rate; /* only read one point in every this many */
	double range_limit_sq; /* the distance to range-limit scanners,
	                          in units of meters^2 */

	/*** post-processing configuration ***/

	/* processing choices */
	bool uniform; /* if true, will use uniformly sized triangles
				in the output */
	bool simplify; /* if true, will attempt to reduce mesh */

	/* region sizing parameters */
	double min_region_area; /* if non-negative, then any defined
					regions must be at least this
					surface area (in meters) */

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
