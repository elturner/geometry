#ifndef CONFIG_H
#define CONFIG_H

/* config.h:
 *
 * 	Represents the functions used
 * 	to read command-line arguments.
 */

#include "filetypes.h"
#include <vector>

using namespace std;

class config_t
{
	/*** parameters ***/
	public:

	char* prog_name; /* the name of the executable */

	/* the list of input files */
	char*         mad_infile;  /* provided input mad file */
	vector<char*> xyz_infiles; /* provided xyz files */
	vector<char*> msd_infiles; /* provided msd files */

	/* the list of output files */
	char* outfile; /* location to write output */
	char* matlab_outfile; /* optional output matlab script */

	/* run parameters */
	double res; /* the resolution to use, in meters */
	double min_floor_height; /* min floor height to use, in meters */

	/*** functions ***/
	public:

	/* parseargs:
	 *
	 *	Reads the input command-line arguments and
	 *	fills the config_t struct.
	 *
	 * arguments:
	 *
	 * 	argc,argv -	What was given to main()
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int parseargs(int argc, char** argv);

	/* print_usage:
	 *
	 * 	Prints the usage of this program to screen.
	 */
	void print_usage();

	/* print_usage_short:
	 *
	 *	Prints a very short message about the program.
	 */
	void print_usage_short();

};

#endif
