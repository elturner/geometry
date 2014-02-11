#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "filetypes.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

#define HELP_FLAG                "-h"

int config_t::parseargs(int argc, char** argv)
{
	int i;
	filetype_t ft;

	/* set default config */
	this->fp_infile = NULL;
	this->windows_infile = NULL;
	this->outfile = NULL;
	this->output_type = unknown_file;

	/* save program name */
	this->prog_name = argv[0];

	/* iterate through arguments */
	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], HELP_FLAG))
		{
			this->print_usage();
			exit(0);
		}
		else
		{
			/* this argument is assumed to be a filename,
			 * figure out which filetype it is */
			ft = filetype_of(argv[i]);
			switch(ft)
			{
				/* check for outfiles */
				case obj_file:
				case wrl_file:
					if(this->outfile != NULL)
					{
						PRINT_WARNING("Multiple"
							" output files "
							"specified, "
							"using:");
						PRINT_WARNING(
							this->outfile);
						PRINT_WARNING("");
					}
					else
					{
						/* save output file */
						this->outfile = argv[i];
						this->output_type = ft;
					}
					break;

				/* check for input files */
				case fp_file:
					if(this->fp_infile != NULL)
					{
						PRINT_WARNING("Multiple"
							" fp files "
							"specified, "
							"using:");
						PRINT_WARNING(
							this->fp_infile);
						PRINT_WARNING("");
					}
					else
					{
						this->fp_infile = argv[i];
					}
					break;
				case windows_file:
					if(this->windows_infile != NULL)
					{
						PRINT_WARNING("Multiple"
							" windows files "
							"specified, "
							"using:");
						PRINT_WARNING(
						this->windows_infile);
						PRINT_WARNING("");
					}
					else
					{
						this->windows_infile 
							= argv[i];
					}
					break;

				/* return error if unknown file */
				case unknown_file:
				default:
					PRINT_WARNING("Ignoring arg:");
					PRINT_WARNING(argv[i]);
					PRINT_WARNING("");
					break;
			}
		}
	}

	/* check that we were given sufficient arguments */
	if(!this->fp_infile)
	{
		PRINT_ERROR("Must specify input floorplan file!");
		return -1;
	}
	if(!this->outfile)
	{
		PRINT_ERROR("Must specify an outfile!");
		return -3;
	}

	/* success */
	return 0;
}

void config_t::print_usage()
{
	printf("\n Usage:\n\n");
	printf("\t%s [flags] <file1> <file2> ...\n\n", this->prog_name);
	printf("\n Valid input files:\n\n");
	printf("\t<fp>       The input *.fp files specify floorplans to\n"
	       "\t           parse and convert to models.\n\n");
	printf("\t<obj>      Specifies an output *.obj file to create\n"
	       "\t           that models the input floorplans.\n\n");
	printf("\t<windows>  Specifies an input *.windows file that\n"
	       "\t           specifies where windows exist in relation\n"
	       "\t           to the input floorplan.\n\n");
	printf("\n");
}

void config_t::print_usage_short()
{
	printf("\n For help information, type:\t%s %s\n\n", 
				this->prog_name, HELP_FLAG);
}
