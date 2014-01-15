#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "filetypes.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

#define HELP_FLAG                "-h"
#define RESOLUTION_FLAG          "-r"
#define MIN_WALL_NUM_POINTS_FLAG "-n"
#define MIN_WALL_HEIGHT_FLAG     "-H"

int parseargs(int argc, char** argv, config_t& conf)
{
	int i;
	char* str;
	filetype_t ft;

	/* set default config */
	conf.num_pc_files = 0;
	conf.mad_infile = NULL;
	conf.resolution = DEFAULT_QUADTREE_RESOLUTION;
	conf.min_wall_num_points = DEFAULT_MIN_NUM_POINTS_PER_WALL_SAMPLE;
	conf.min_wall_height = DEFAULT_MIN_WALL_HEIGHT; 
	conf.outfile = NULL;
	
	/* iterate through arguments */
	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], HELP_FLAG))
		{
			print_usage(argv[0]);
			exit(0);
		}
		else if(!strcmp(argv[i], RESOLUTION_FLAG))
		{
			i++;
			str = NULL;
			conf.resolution = strtod(argv[i], &str);
			if(str == argv[i])
			{
				/* not valid double */
				fprintf(stderr, "Could not parse: %s\n",
							str);
				return i;
			}
		}
		else if(!strcmp(argv[i], MIN_WALL_HEIGHT_FLAG))
		{
			i++;
			str = NULL;
			conf.min_wall_height = strtod(argv[i], &str);
			if(str == argv[i])
			{
				/* not valid double */
				fprintf(stderr, "Could not parse: %s\n",
							str);
				return i;
			}
		}
		else if(!strcmp(argv[i], MIN_WALL_NUM_POINTS_FLAG))
		{
			i++;
			str = NULL;
			conf.min_wall_num_points = strtol(argv[i],&str,10);
			if(str == argv[i])
			{
				/* not valid double */
				fprintf(stderr, "Could not parse: %s\n",
							str);
				return i;
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
				case xyz_file:
					if(conf.num_pc_files 
						< MAX_POINTCLOUD_FILES)
						conf.pc_infile[
						conf.num_pc_files++] 
							= argv[i];
					else
					{
						PRINT_WARNING(
							"[parseargs]\t"
							"too many input"
							" files, "
							"ignoring:");
						PRINT_WARNING(argv[i]);
						PRINT_WARNING("");
					}
					break;
				case mad_file:
					if(conf.mad_infile != NULL)
					{
						PRINT_WARNING("Multiple"
							" mad files "
							"specified, "
							"using:");
						PRINT_WARNING(
							conf.mad_infile);
						PRINT_WARNING("");
					}
					else
					{
						conf.mad_infile = argv[i];
					}
					break;

				/* check for outfiles */
				case dq_file:
					if(conf.outfile != NULL)
					{
						/* The output file was
						 * already specified, so
						 * ignore this arg */
						PRINT_WARNING("Multiple"
							" output files "
							"specified, "
							"using:");
						PRINT_WARNING(conf.outfile);
						PRINT_WARNING("");
					}
					else
					{
						/* use this as the output
						 * file */
						conf.outfile = argv[i];
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
	if(conf.num_pc_files == 0)
	{
		PRINT_ERROR("Must specify an input point-cloud!");
		return -2;
	}
	if(!conf.mad_infile)
	{
		PRINT_ERROR("Must specify an input mad file!");
		return -3;
	}
	if(!conf.outfile)
	{
		PRINT_ERROR("Must specify an outfile!");
		return -5;
	}
	if(conf.resolution <= 0)
	{
		PRINT_ERROR("Must specify positive resolution!");
		return -6;
	}
	if(conf.min_wall_height <= 0)
	{
		PRINT_ERROR("Must specify positive wall threshold height!");
		return -7;
	}

	/* success */
	return 0;
}

void print_usage(char* prog_name)
{
	printf("\n Usage:\n\n");
	printf("\t%s <file1> <file2> ...\n\n", prog_name);
	printf("\tThis program generates a Dynamic Quadtree (DQ) file\n");
	printf("\tfrom wall samples of the input point-clouds using\n");
	printf("\tthe corresponding path of the mobile scanner.\n");
	printf("\n Where:\n\n");
	printf("\t%s <float> Specifies the resolution of output tree.\n"
	       "\t           The default resolution is %f meters.\n\n",
					RESOLUTION_FLAG,
					DEFAULT_QUADTREE_RESOLUTION);
	printf("\t%s <int>   Specifies the minimum threshold of a wall's\n"
	       "\t           number of points for it to be captured in\n"
	       "\t           the output.  The default is %d points.\n\n",
				MIN_WALL_NUM_POINTS_FLAG,
				DEFAULT_MIN_NUM_POINTS_PER_WALL_SAMPLE);
	printf("\t%s <float> Specifies the minimum threshold of a wall\n"
	       "\t           height for it to be captured in the output.\n"
	       "\t           The default resolution is %f meters.\n\n",
					MIN_WALL_HEIGHT_FLAG,
					DEFAULT_MIN_WALL_HEIGHT);
	printf("\n Valid input files:\n\n");
	printf("\t<xyzfile>  The input ascii *.xyz file that\n"
	       "\t           specifies the input pointcloud.\n"
	       "\t           At least one must be specified.\n"
	       "\t           Each file is processed separately\n"
	       "\t           and only one is stored in memory\n"
	       "\t           at a time.\n\n");
	printf("\t<madfile>  The input *.mad file.  Exactly\n"
	       "\t           one must be specified.\n\n");
	printf("\t<outfile>  The *.dq file to write surface to.\n"
	       "\t           If multiple are specified, only the first\n"
	       "\t           will be used.\n\n");
}

void print_usage_short(char* prog_name)
{
	printf("\n For help information, type:\t%s %s\n\n", 
				prog_name, HELP_FLAG);
}
