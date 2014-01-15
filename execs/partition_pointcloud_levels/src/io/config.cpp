#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "filetypes.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

#define HELP_FLAG                "-h"
#define OUTFILE_FLAG             "-o"
#define RESOLUTION_FLAG          "-r"
#define MIN_FLOOR_HEIGHT_FLAG    "-H"

int config_t::parseargs(int argc, char** argv)
{
	int i;
	filetype_t ft;
	char* str;

	/* set default config */
	this->mad_infile = NULL;
	this->xyz_infiles.clear();
	this->msd_infiles.clear();
	this->outfile = NULL;
	this->matlab_outfile = NULL;
	this->res = DEFAULT_RESOLUTION;
	this->min_floor_height = DEFAULT_MIN_FLOOR_HEIGHT;

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
		else if(!strcmp(argv[i], RESOLUTION_FLAG))
		{
			/* the next value is the resolution */
			i++;
			if(i >= argc)
			{
				PRINT_ERROR("Must specify a resolution "
					"(double) after the " 
					RESOLUTION_FLAG " flag");
				return i;
			}

			/* parse the resolution from command-line */
			str = NULL;
			this->res = strtod(argv[i], &str);
			if(str == argv[i] || this->res <= 0)
			{
				/* not valid double */
				PRINT_ERROR("resolution specified "
					"is not valid");
				return i;
			}
		}
		else if(!strcmp(argv[i], MIN_FLOOR_HEIGHT_FLAG))
		{
			/* the next value is the min floor height */
			i++;
			if(i >= argc)
			{
				PRINT_ERROR("Must specify a value (double)"
					" after the " MIN_FLOOR_HEIGHT_FLAG
					" flag");
				return i;
			}

			/* parse the floor height */
			str = NULL;
			this->min_floor_height = strtod(argv[i], &str);
			if(str == argv[i] || this->min_floor_height <= 0)
			{
				/* not valid */
				PRINT_ERROR("min floor height specified"
					" is not valid");
				return i;
			}
		}
		else if(!strcmp(argv[i], OUTFILE_FLAG))
		{
			/* the next value will be the outfile location */
			i++;

			/* check if unique */
			if(this->outfile != NULL)
			{
				/* The output file was already specified,
				 * so ignore this arg */
				PRINT_WARNING("Multiple output files "
					"specified, using:");
				PRINT_WARNING(this->outfile);
				PRINT_WARNING("");
			}
			else if(i < argc)
			{
				/* use this as the output file */
				this->outfile = argv[i];
			}
		}
		else
		{
			/* this argument is assumed to be a filename,
			 * figure out which filetype it is */
			ft = filetype_of(argv[i]);
			switch(ft)
			{
				/* check for infiles */
				case mad_file:
					if(this->mad_infile != NULL)
					{
						PRINT_WARNING("Multiple"
							" mad files "
							"specified, "
							"using:");
						PRINT_WARNING(
							this->mad_infile);
						PRINT_WARNING("");
					}
					else
					{
						this->mad_infile = argv[i];
					}
					break;

				/* check for input xyz files */
				case xyz_file:
					this->xyz_infiles.push_back(
							argv[i]);
					break;

				/* check for input msd files */
				case msd_file:
					this->msd_infiles.push_back(
							argv[i]);
					break;
				
				/* check for matlab output file */
				case m_file:
					if(this->matlab_outfile != NULL)
					{
						PRINT_WARNING("Multiple"
							" output matlab "
							"scripts "
							"specified, "
							"using:");
						PRINT_WARNING(
							this->matlab_outfile
							);
						PRINT_WARNING("");
					}
					else
					{
						this->matlab_outfile 
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
	if(this->xyz_infiles.empty() && this->msd_infiles.empty())
	{
		PRINT_ERROR("Must specify input scans!");
		return -1;
	}
	if(!this->mad_infile)
	{
		PRINT_ERROR("Must specify input mad file!");
		return -2;
	}
	if(!this->outfile && !this->matlab_outfile)
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
	printf("\n Option flags:\n\n");
	printf("\t%s <file>  Specifies the location to write the output.\n"
	       "\t           The file specifies should not have a suffix,\n"
	       "\t           since that will be appended to the file name\n"
	       "\t           given, so that each floor exported will have\n"
	       "\t           a unique output.  Output will be in xyz\n"
	       "\t           format.\n\n", OUTFILE_FLAG);
	printf("\t%s <flt>   Specifies the resolution of histogram. If\n"
	       "\t           none specified, a default value of %fm is\n"
	       "\t           used.\n\n", RESOLUTION_FLAG, 
	       DEFAULT_RESOLUTION);
	printf("\t%s <flt>   Specifies the minimum distance between the\n"
	       "\t           floors in a building, so no two ceilings\n"
	       "\t           are within this distance of one another. If\n"
	       "\t           not specified, the default value of %f m is\n"
	       "\t           used.\n\n", MIN_FLOOR_HEIGHT_FLAG,
	       DEFAULT_MIN_FLOOR_HEIGHT);
	printf("\n Valid input files:\n\n");
	printf("\t<madfile>  The input *.mad file.  Exactly\n"
	       "\t           one must be specified.\n\n");
	printf("\t<msdfile>  The scan points in sensor coordinates.  Any\n"
	       "\t           number of these can be specified.\n\n");
	printf("\t<xyzfile>  The scan points in world coordinates.  Any\n"
	       "\t           number of these can be specified.\n\n");
	printf("\n");
}

void config_t::print_usage_short()
{
	printf("\n For help information, type:\t%s %s\n\n", 
				this->prog_name, HELP_FLAG);
}
