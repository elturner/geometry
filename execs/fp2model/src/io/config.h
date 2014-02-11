#ifndef CONFIG_H
#define CONFIG_H

/* config.h:
 *
 * 	Represents the functions used
 * 	to read command-line arguments.
 */

#include "filetypes.h"

using namespace std;

class config_t
{
	/*** parameters ***/
	public:

	char* prog_name; /* the name of the executable */

	/* the list of input files */
	char* fp_infile; /* provided fp file */
	char* windows_infile; /* provided windows file */

	/* the list of output files */
	filetype_t output_type; /* filetype of output file */
	char* outfile; /* location to write output file */

	/*** functions ***/
	public:

	/* parseargs:
	 *
	 *	Reads the input command-line arguments and
	 *	fills the config_t struct.
	 *
	 * arguments:
	 *
	 * 	argc, argv -	What was given to main()
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
