#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "filetypes.h"
#include "../util/error_codes.h"
#include "../util/constants.h"

#define HELP_FLAG                "-h"
#define SIMPLIFY_THRESHOLD_FLAG  "-s"
#define EXPORT_2D_FLAG           "-2"
#define CARVE_THROUGH_FLAG       "-c"
#define SIMP_DOOR_FLAG           "-d"
#define NUM_POSES_FLAG           "-n"

int parseargs(int argc, char** argv, config_t& conf)
{
	int i;
	filetype_t ft;
	char* str;

	/* set default config */
	conf.dq_infile = NULL;
	conf.mad_infile = NULL;
	conf.xml_infile = NULL;
	conf.outfile = NULL;
	conf.output_type = unknown_file;
	conf.simplify_threshold = DEFAULT_SIMPLIFY_THRESHOLD;
	conf.export_2d = false;
	conf.carve_through = false;
	conf.simpdoor = false;
	conf.num_poses = -1;

	/* iterate through arguments */
	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], HELP_FLAG))
		{
			print_usage(argv[0]);
			exit(0);
		}
		else if(!strcmp(argv[i], SIMPLIFY_THRESHOLD_FLAG))
		{
			/* check record the simplification threshold */
			i++;
			str = NULL;
			conf.simplify_threshold = strtod(argv[i], &str);
			if(str == argv[i])
			{
				/* not a valid double */
				fprintf(stderr, "Could not parse "
						"argument: %s\n", str);
				return i;
			}
		}
		else if(!strcmp(argv[i], NUM_POSES_FLAG))
		{
			/* get specified number of poses to process */
			i++;
			str = NULL;
			conf.num_poses = strtol(argv[i], &str, 10);
			if(str == argv[i])
			{
				/* not a valid integer */
				fprintf(stderr, "Could not parse "
						"argument: %s\n", str);
				return i;
			}
		}
		else if(!strcmp(argv[i], EXPORT_2D_FLAG))
		{
			conf.export_2d = true;
		}
		else if(!strcmp(argv[i], CARVE_THROUGH_FLAG))
		{
			conf.carve_through = true;
		}
		else if(!strcmp(argv[i], SIMP_DOOR_FLAG))
		{
			conf.simpdoor = true;
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
				case dq_file:
					if(conf.dq_infile != NULL)
					{
						PRINT_WARNING("Multiple"
							" DQ files "
							"specified, "
							"using:");
						PRINT_WARNING(
							conf.dq_infile);
						PRINT_WARNING("");
					}
					else
					{
						conf.dq_infile = argv[i];
					}
					break;
				case xml_file:
					if(conf.xml_infile != NULL)
					{
						PRINT_WARNING("Multiple"
							" xml files "
							"specified, "
							"using:");
						PRINT_WARNING(
							conf.xml_infile);
						PRINT_WARNING("");
					}
					else
					{
						conf.xml_infile = argv[i];
					}
					break;

				/* check for outfiles */
				case obj_file:
				case fp_file:
				case ply_file:
				case edge_file:
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
						conf.output_type = ft;
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
	if(!conf.dq_infile)
	{
		PRINT_ERROR("Must specify an input DQ file!");
		return -1;
	}
	if(!conf.mad_infile)
	{
		PRINT_ERROR("Must specify an input mad file!");
		return -2;
	}
	if(!conf.outfile)
	{
		PRINT_ERROR("Must specify an outfile!");
		return -3;
	}

	/* success */
	return 0;
}

void print_usage(char* prog_name)
{
	printf("\n Usage:\n\n");
	printf("\t%s [flags] <file1> <file2> ...\n\n", prog_name);
	printf("\n Option flags:\n\n");
	printf("\t%s <float> Specifies simplification threshold to use\n"
	       "\t           for wall simplification.  A negative value\n"
	       "\t           denotes that no simplification will be\n"
	       "\t           performed.  This value roughly relates to\n"
	       "\t           distance vertices are moved from original\n"
	       "\t           2D mesh.  This flag is optional, and the\n"
	       "\t           default value if not specified is %.3f m\n\n",
	       	SIMPLIFY_THRESHOLD_FLAG, DEFAULT_SIMPLIFY_THRESHOLD);
	printf("\t%s         If specified, any output meshes will be 2D,\n"
	       "\t           instead of 3D extrusions.\n\n",
	       EXPORT_2D_FLAG);
	printf("\t%s         If specified, will not use any occlusion-\n"
	       "\t           checking when performing ray-tracing for\n"
	       "\t           geometry creation.\n\n", CARVE_THROUGH_FLAG);
	printf("\t%s         If specified, will simplify door geometry\n"
	       "\t           as well as the rest of the walls.  By\n"
	       "\t           default, door geometry is preserved and kept\n"
	       "\t           unsimplified, but this flag will force\n"
	       "\t           simplification of doors.\n\n", SIMP_DOOR_FLAG);
	printf("\t%s <int>   If specified, then will only use the first\n"
	       "\t           <int> poses specified. By default, all poses\n"
	       "\t           are used.\n\n", NUM_POSES_FLAG);
	printf("\n Valid input files:\n\n");
	printf("\t<dq_file>  The input *.dq file.  Exactly\n"
	       "\t           one must be specified.\n\n");
	printf("\t<madfile>  The input *.mad file.  Exactly\n"
	       "\t           one must be specified.\n\n");
	printf("\t<xmlfile>  An optional input *.xml file.  If provided,\n"
	       "\t           will use the sensor extrinsics specified to\n"
	       "\t           make starting point of ray tracing more\n"
	       "\t           accurate.\n\n");
	printf("\t<outfile>  The output file to write floorplan to.\n"
	       "\t           If multiple are specified, only the first\n"
	       "\t           will be used.  Valid formats are:\n\n"
	       "\t           *.obj, *.ply, *.fp, *.edge\n\n");
	printf("\n");
}

void print_usage_short(char* prog_name)
{
	printf("\n For help information, type:\t%s %s\n\n", 
				prog_name, HELP_FLAG);
}
