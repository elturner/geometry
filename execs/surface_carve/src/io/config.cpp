#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "filetypes.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

#define HELP_FLAG             "-h"
#define RESOLUTION_FLAG       "-r"
#define BEGIN_POSE_FLAG       "-b"
#define NUM_POSES_FLAG        "-n"
#define READ_VOX_FLAG         "-v"
#define POINT_OCCLUSIONS_FLAG "-p"
#define DOWNSAMPLE_FLAG       "-d"
#define SIMPLIFY_FLAG         "-s"
#define UNIFORM_FLAG          "-u"
#define RANGE_LIMIT_FLAG      "-m"
#define NO_CHUNK_FILES_FLAG   "-f"
#define OUTPUT_ASCII_FLAG     "-a"
#define COALESCE_REGIONS_FLAG "-c"

int parseargs(int argc, char** argv, config_t& conf)
{
	int i;
	char* str;
	filetype_t ft;

	/* set default config */
	conf.num_pc_files = 0;
	conf.chunk_pc_files = true;
	conf.resolution = DEFAULT_VOXEL_RESOLUTION;
	conf.mad_infile = NULL;
	conf.bcfg_infile = NULL;
	conf.outfile = NULL;
	conf.output_type = unknown_file;
	conf.output_ascii = false;
	conf.point_occlusions = false;
	conf.voxfile = NULL;
	conf.readvox = false;
	conf.begin_pose = 0;
	conf.num_poses = -1;
	conf.downsample_rate = 1;
	conf.range_limit_sq = DEFAULT_MAX_SCAN_DISTANCE_SQ;
	conf.uniform = false;
	conf.simplify = false;
	conf.min_region_area = -1;
	
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
				/* not a valid double */
				fprintf(stderr, "Could not parse: %s\n",
								str);
				return i;
			}
		}
		else if(!strcmp(argv[i], NUM_POSES_FLAG))
		{
			i++;
			str = NULL;
			conf.num_poses = strtol(argv[i], &str, 10);
			if(str == argv[i])
			{
				/* not a valid int */
				fprintf(stderr, "Could not parse: %s\n",
								str);
				return i;
			}
		}
		else if(!strcmp(argv[i], BEGIN_POSE_FLAG))
		{
			i++;
			str = NULL;
			conf.begin_pose = strtol(argv[i], &str, 10);
			if(str == argv[i])
			{
				/* not a valid int */
				fprintf(stderr, "Could not parse: %s\n",
								str);
				return i;
			}
		}
		else if(!strcmp(argv[i], POINT_OCCLUSIONS_FLAG))
		{
			/* check for point occlusions */
			conf.point_occlusions = true;
		}
		else if(!strcmp(argv[i], READ_VOX_FLAG))
		{
			/* read from voxel file */
			conf.readvox = true;
		}
		else if(!strcmp(argv[i], NO_CHUNK_FILES_FLAG))
		{
			/* don't attempt to chunk pointcloud files,
			 * read each all in at once */
			conf.chunk_pc_files = false;
		}
		else if(!strcmp(argv[i], DOWNSAMPLE_FLAG))
		{
			i++;
			str = NULL;
			conf.downsample_rate = strtol(argv[i], &str, 10);
			if(str == argv[i] || conf.downsample_rate <= 0)
			{
				/* not a valid int */
				fprintf(stderr, "Could not parse: %s\n",
								str);
				return i;
			}
		}
		else if(!strcmp(argv[i], RANGE_LIMIT_FLAG))
		{
			i++;
			str = NULL;
			conf.range_limit_sq = strtod(argv[i], &str);
			if(str == argv[i] || conf.range_limit_sq <= 0)
			{
				/* not valid range limit */
				fprintf(stderr, "Could not parse: %s\n",
								str);
				return i;
			}

			/* human inputs as meters, we want to store as
			 * meters^2 */
			conf.range_limit_sq *= conf.range_limit_sq;
		}
		else if(!strcmp(argv[i], UNIFORM_FLAG))
		{
			/* make uniform mesh */
			conf.uniform = true;
		}
		else if(!strcmp(argv[i], SIMPLIFY_FLAG))
		{
			/* simplify mesh */
			conf.simplify = true;
		}
		else if(!strcmp(argv[i], OUTPUT_ASCII_FLAG))
		{
			/* attempt to write output in ascii, if possible */
			conf.output_ascii = true;
		}
		else if(!strcmp(argv[i], COALESCE_REGIONS_FLAG))
		{
			i++;
			str = NULL;
			conf.min_region_area = strtod(argv[i], &str);
			if(str == argv[i] || conf.min_region_area <= 0)
			{
				/* not valid range limit */
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
				case bcfg_file:
					if(conf.bcfg_infile != NULL)
					{
						PRINT_WARNING("Multiple"
							" bcfg files "
							"specified, "
							"using:");
						PRINT_WARNING(
							conf.bcfg_infile);
						PRINT_WARNING("");
					}
					else
					{
						conf.bcfg_infile = argv[i];
					}
					break;

				/* check for outfiles */
				case obj_file:
				case ply_file:
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
		
				/* check for voxel files */
				case vox_file:
					if(conf.voxfile != NULL)
					{
						/* vox file already
						 * specified, ignore
						 * this arg */
						PRINT_WARNING("Multiple"
							" vox files "
							"specified, "
							"using:");
						PRINT_WARNING(conf.voxfile);
						PRINT_WARNING("");
					}
					else
					{
						/* use this voxfile */
						conf.voxfile = argv[i];
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
	if(conf.resolution < 0)
	{
		PRINT_ERROR("Must specify a valid resolution!");
		return -1;
	}
	if(conf.num_pc_files == 0 && !conf.readvox)
	{
		PRINT_ERROR("Must specify an input point-cloud!");
		return -2;
	}
	if(!conf.mad_infile && !conf.readvox)
	{
		PRINT_ERROR("Must specify an input mad file!");
		return -3;
	}
	if(conf.readvox && !conf.voxfile)
	{
		PRINT_ERROR("Must specify an input vox file!");
		return -4;
	}
	if(!conf.outfile)
	{
		PRINT_ERROR("Must specify an outfile!");
		return -5;
	}

	/* success */
	return 0;
}

void print_usage(char* prog_name)
{
	printf("\n Usage:\n\n");
	printf("\t%s %s <res> [...] <file1> <file2> ...\n\n", 
			prog_name, RESOLUTION_FLAG);
	printf("\tThis program generates a water-tight 3D surface\n");
	printf("\tfrom input point-clouds and the corresponding path\n");
	printf("\tof the mobile scanner.\n");
	printf("\n Where:\n\n");
	printf("\t%s <res>   Resolution of voxels, measured in\n"
	       "\t           meters (decimal value).  Default value\n"
	       "\t           is %0.3f m.\n\n", RESOLUTION_FLAG, 
	       					DEFAULT_VOXEL_RESOLUTION);
	printf("\t%s <int>   Optional.  The pose number at which to\n"
	       "\t           begin carving.  By default, carving will\n"
	       "\t           begin at the 0'th pose.\n\n", BEGIN_POSE_FLAG);
	printf("\t%s <int>   Number of poses to process.  Default\n"
	       "\t           computes all poses.\n\n", NUM_POSES_FLAG);
	printf("\t%s         Optionally denotes to perform carving while\n"
	       "\t           checking for scan point occlusions.  If flag\n"
	       "\t           is present, then any voxel carving will be\n"
	       "\t           truncated as to not carve through voxels\n"
	       "\t           which contain elements of the input point-\n"
	       "\t           cloud.  This helps to preserve structures\n"
	       "\t           such as walls that are seen from both sides,\n"
	       "\t           even in the presence of registration error,\n"
	       "\t           though may result in artifacts from objects\n"
	       "\t           in the scene that are temporary, such as\n"
	       "\t           people walking who were briefly scanned.\n\n"
	       "\t           Highly recommended for point-clouds with\n"
	       "\t           minimal error and lots of small details.\n\n",
	       				POINT_OCCLUSIONS_FLAG);
	printf("\t%s         If present, will use the specified *.vox\n"
	       "\t           file to populate the carved voxels.\n\n",
	       					READ_VOX_FLAG);
	printf("\t%s         If present, will force the program to read\n"
	       "\t           each input point-cloud file in its entirety\n"
	       "\t           at once.  This will crash the program if the\n"
	       "\t           file is more than can be stored in memory,\n"
	       "\t           but has the advantage of being faster for\n"
	       "\t           small data sets.  By default, the program\n"
	       "\t           reads each file in chunks of %d scans.\n\n"
	       "\t           NOTE: if the input point-cloud files are not\n"
	       "\t           ordered, then this flag MUST be used.\n\n",
	       		NO_CHUNK_FILES_FLAG, NUM_SCANS_PER_FILE_CHUNK);
	printf("\t%s <int>   Optionally denotes the downsample rate.\n"
	       "\t           If file is large, will only read in 1/<int>\n"
	       "\t           of the points, and use that subset to\n"
	       "\t           perform the carving.  Default uses all\n"
	       "\t           points in the file.\n\n", DOWNSAMPLE_FLAG);
	printf("\t%s <len>   Optionally specifies the maximum range\n"
	       "\t           a laser scan sample can be from the scanner.\n"
	       "\t           The parameter is measured in meters.  The\n"
	       "\t           default value is %.2f m.  If you trust\n"
	       "\t           localization of the point-cloud, make this\n"
	       "\t           value large.  Limiting this value may cause\n"
	       "\t           some scanned features not to appear in the\n"
	       "\t           the final output, but features close to the\n"
	       "\t           scanner may look nicer.\n\n", 
	       			RANGE_LIMIT_FLAG,
	       			sqrt(DEFAULT_MAX_SCAN_DISTANCE_SQ));
	printf("\t%s         Optional.  If present, and the output file\n"
	       "\t           specified is a format that can be in either\n"
	       "\t           ascii or binary, will write output in ascii.\n"
	       "\t           By default, will output any such formats in\n"
	       "\t           binary.\n\n", OUTPUT_ASCII_FLAG);
	printf("\t%s         Optional.  Will triangulate using marching\n"
	       "\t           cubes, resulting in uniform-sized elements.\n"
	       "\t           Note that this mesh will be less accurate,\n"
	       "\t           but also have fewer self-intersections.\n\n",
	       					UNIFORM_FLAG);
	printf("\t%s         Optionally simplifies triangular mesh.  The\n"
	       "\t           number of triangles in the mesh will be\n"
	       "\t           reduced by using edge-contraction within\n"
	       "\t           planar regions.  Ignored if %s is not also\n"
	       "\t           present.\n", SIMPLIFY_FLAG, UNIFORM_FLAG);
	printf("\t           WARNING: This feature may result in self-\n"
	       "\t           overlapping meshes.\n\n");
	printf("\t%s <area>  Optionally specifies a minimum surface area\n"
	       "\t           for planar regions defined in ouput models.\n"
	       "\t           Changing this number will not affect the\n"
	       "\t           geometry of triangles, but the groupings of\n"
	       "\t           triangles.  A larger number will result in\n"
	       "\t           fewer total regions.  Units:  square meters\n",
	       COALESCE_REGIONS_FLAG);
	printf("\n Valid input files:\n\n");
	printf("\t<xyzfile>  The input ascii *.xyz file that\n"
	       "\t           specifies the input pointcloud.\n"
	       "\t           At least one must be specified.\n"
	       "\t           Each file is processed separately\n"
	       "\t           and only one is stored in memory\n"
	       "\t           at a time.\n\n");
	printf("\t<madfile>  The input *.mad file.  Exactly\n"
	       "\t           one must be specified.\n\n");
	printf("\t<bcfgfile> This denotes a configuration file for the\n"
	       "\t           scanner hardware used during the data\n"
	       "\t           collection.  If specified, will check the\n"
	       "\t           input *.xyz filenames for laser serial\n"
	       "\t           numbers.  If those are found, carving will\n"
	       "\t           be performed from the laser's position at\n"
	       "\t           each pose.  If the *.xyz filenames do not\n"
	       "\t           contain these serial numbers, or no config\n"
	       "\t           file is specified, then carving is performed\n"
	       "\t           from the pose position.\n\n");
	printf("\t<outfile>  The *.obj or *.ply file to write surface to.\n"
	       "\t           If multiple are specified, only the first\n"
	       "\t           will be used.\n\n");
	printf("\t<voxfile>  Optional. A *.vox file can be used\n"
	       "\t           either to specify where to store the\n"
	       "\t           carved voxel grid or where to read\n"
	       "\t           the grid from.  Reading in an existing\n"
	       "\t           grid skips the carving process, and is\n"
	       "\t           useful for debugging.\n");
	printf("\n Example:\n\n");
	printf("\t%s -r 0.01 -n 5000 "
		"example.mad example.xyz example.obj\n\n", prog_name);
	printf("\tThis runs the program on the input point-cloud\n");
	printf("\texample.xyz, which was generated from the path\n");
	printf("\texample.mad.  The output surface will be saved\n");
	printf("\tto the file example.obj.  The surface will use\n");
	printf("\ta resolution of 1 centimeter.  Only the points\n");
	printf("\tassociated with the first 5000 poses are used.\n");
	printf("\n Trouble-shooting:\n\n");
	printf("\tIf you get the following error:\n\n");
	printf("\tterminate called after throwing an instance of"
						" 'std::bad_alloc'\n\n");
	printf("\tIt is likely because the input xyz file is too big\n");
	printf("\tto fit in memory.  Either split it into multiple\n");
	printf("\t*.xyz files, with some overlap, or use the %s flag\n",
						DOWNSAMPLE_FLAG);
	printf("\tto decimate the point-cloud.\n");
	printf("\n References:\n\n");
	printf("\tC. Holenstein, R. Zlot, and M. Bosse. \"Watertight\n");
	printf("\tSurface Reconstruction of Caves from 3D Laser Data\"\n");
	printf("\tIntelligent Robots and Systems, Sept. 2011.\n");
	printf("\n");
}

void print_usage_short(char* prog_name)
{
	printf("\n For help information, type:\t%s %s\n\n", 
				prog_name, HELP_FLAG);
}
