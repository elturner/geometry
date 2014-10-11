#ifndef CONFIG_H
#define CONFIG_H

/* config.h:
 *
 * 	Represents the functions used
 * 	to read command-line arguments.
 */

#include "filetypes.h"

typedef struct config
{
	/*** input files configuration ***/
	
	/* the list of input point-cloud files */
	char* dq_infile; /* the location of the input dq file */
	char* mad_infile; /* location of input mad file */
	char* xml_infile; /* optional backpack extrinsics */

	/*** output files configuration ***/

	char* outfile; /* location to write output */
	filetype_t output_type; /* what format to output in */

	/*** options ***/

	/* indicates the number of poses to process.  A negative
	 * value indicates that all poses will be used. */
	int num_poses;

	/* This threshold denotes how aggresively to simplify the mesh.
	 * A negative threshold denotes no simplification should be
	 * performed. */
	double simplify_threshold;

	/* this boolean specifies to output a 2D mesh instead
	 * of a 3D extruded mesh */
	bool export_2d;

	/* determines if ray-tracing will use occlusion-check (if false),
	 * or will fully carve through any existing cells (if true) */
	bool carve_through;

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
