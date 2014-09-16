#include "makegrid.h"
#include <vector>
#include <iostream>
#include "../structs/dgrid.h"
#include "../structs/pose.h"
#include "../structs/point.h"
#include "../io/config.h"
#include "../io/scanner_config_io.h"
#include "../io/pose_io.h"
#include "../io/point_io.h"
#include "../io/voxel_io.h"
#include "../carve/carve_dgrid.h"
#include "../util/tictoc.h"
#include "../util/error_codes.h"
#include "../test/stats.h"

using namespace std;

int make_grid(dgrid_t& grid, config_t& conf)
{
	scanner_config_t bcfg;
	boundingbox_t bbox;
	vector<pose_t> poselist;
	vector<streampos> sssp;
	point_t p;
	tictoc_t clk;
	unsigned int i, j, k, n;
	int ret, r;

	/* initialize bounding box */
	boundingbox_init(bbox);

	/* check if we need to compute carved grid */
	if(conf.readvox)
	{
		/* import grid from voxel file */
		tic(clk);
		ret = readvox(conf.voxfile, grid);
		if(ret)
		{
			PRINT_ERROR("[make_grid]"
				"\tCould not read from voxel file:");
			PRINT_ERROR(conf.voxfile);
			return PROPEGATE_ERROR(-1, ret);
		}
		toc(clk, "Reading voxel file");
	}
	else
	{
		/* read input mad file */
		tic(clk);
		ret = readmad(conf.mad_infile, poselist);
		if(ret)
		{
			PRINT_ERROR("[make_grid]\tCould not read:");
			PRINT_ERROR(conf.mad_infile);
			return PROPEGATE_ERROR(-2, ret);
		}
		toc(clk, "Reading poses");

		/* read in scanner configuration file, if available */
		if(conf.bcfg_infile)
		{
			/* parse scanner configuration file */
			tic(clk);
			ret = bcfg.import(conf.bcfg_infile);
			if(ret)
			{
				PRINT_ERROR("[make_grid]\tCould not read:");
				PRINT_ERROR(conf.bcfg_infile);
				return PROPEGATE_ERROR(-3, ret);
			}
			toc(clk, "Reading scanner config file");
		}

		/* verify that we are given a valid region to process */
		if(conf.begin_pose >= (int) poselist.size())
		{
			PRINT_ERROR("[make_grid]\tUser-specified "
					"beginning pose not valid.");
			return PROPEGATE_ERROR(-4, ret);
		}

		/* initialize voxel grid */
		grid.init(conf.resolution);
	
		/* if point-occlusions are active, then we must
		 * populate the point voxel set in the dgrid structure,
		 * using all of the input pointclouds. */
		if(conf.point_occlusions)
		{
			tic(clk);
			for(i = 0; i < conf.num_pc_files; i++)
				grid.populate_points_from_xyz(
						conf.pc_infile[i], 
						poselist, 
						conf.range_limit_sq);
			toc(clk, "Populating scan-point voxels");
		}

		/* read and carve from input point-cloud files */
		for(i = 0; i < conf.num_pc_files; i++)
		{
			/* check to see if we have
			 * additional information about
			 * the scans */
			r = bcfg.index_of_laser(conf.pc_infile[i]);
			if(r < 0)
			{
				PRINT_WARNING("No scanner configuration "
					"given for point cloud:");
				PRINT_WARNING(conf.pc_infile[i]);
				PRINT_WARNING("");
				p.x = p.y = p.z = 0;
			}
			else
			{
				/* change laser_pos field based on scanner 
				 * config file */
				p.x = bcfg.lasers[r].pos.x;
				p.y = bcfg.lasers[r].pos.y;
				p.z = bcfg.lasers[r].pos.z;
			}

			/* check whether we should read the next file
			 * in all at once, or if we should partition it into
			 * more managable chunks */
			if(conf.chunk_pc_files)
			{
				/* get scan line numbers for the next
				 * point-cloud */
				tic(clk);
				ret = readxyz_index_scans(conf.pc_infile[i],
							sssp);
				if(ret)
				{
					PRINT_ERROR("[make_grid]\t"
							"BAD FILE:");
					PRINT_ERROR(conf.pc_infile[i]);
					PRINT_ERROR("");
					PRINT_ERROR("The input point-cloud"
					     " specified is not valid."
					     "  Please make sure that "
					     "all fields are specified "
					     "and that the points"
					     " are stored in order."); 
					return PROPEGATE_ERROR(-5, ret);
				}
				toc(clk, "Indexing pointcloud file");

				/* determine how many poses to process */
				n = (conf.num_poses < 0) ? sssp.size()
					: (conf.begin_pose 
						+ conf.num_poses);
				if(n > sssp.size())
					n = sssp.size();
				/* n represents the ending pose (exclusive)
				 * to process */

				/* process each chunk of the file 
				 * separately */
				for(j = conf.begin_pose; j < n; j += 
						(NUM_SCANS_PER_FILE_CHUNK 
						- OVERLAP_PER_FILE_CHUNK))
				{
					/* reared from start to end */
					k = j + NUM_SCANS_PER_FILE_CHUNK;
					if(k >= n)
						k = n-1;
				
					/* get next chunk of points, which
					 * starts at the j'th pose and ends
					 * at the k'th pose, inclusive */
					tic(clk);
					ret = readxyz_subset_to_pose(
						conf.pc_infile[i],
						sssp[j], sssp[k], poselist,
						bbox, p, 
						conf.downsample_rate,
						conf.range_limit_sq);	
					if(ret)
					{
						PRINT_ERROR(
							"[make_grid]\tCould"
							" not read chunk "
							" of:"); 
						PRINT_ERROR(
							conf.pc_infile[i]);
						return PROPEGATE_ERROR(-6, 
								ret);
					}
					toc(clk,"Reading pointcloud chunk");

					/* carve tets out of grid */
					tic(clk);
					carve_path(grid,poselist,j,k-j+1);
					toc(clk, "Carving voxels");
				
					/* we are now done with 
					 * current chunk of scans */
					poselist_clear_points(poselist);
				}
	
				/* for user convenience, add a space 
				 * between input pointcloud files */
				LOG("\n");
			}
			else
			{
				/* read next point-cloud */
				tic(clk);
				ret = readxyz_to_pose(conf.pc_infile[i], 
						poselist, bbox, p,
						conf.downsample_rate,
						conf.range_limit_sq);
				if(ret)
				{
					PRINT_ERROR("[make_grid]\t"
							"Could not read:"); 
					PRINT_ERROR(conf.pc_infile[i]);
					return PROPEGATE_ERROR(-7, ret);
				}
				toc(clk, "Reading pointcloud");
		
				/* carve tets out of grid */
				tic(clk);
				carve_path(grid, poselist, conf.begin_pose,
							conf.num_poses);
				toc(clk, "Carving voxels");
	
				/* we are now done with current scans */
				poselist_clear_points(poselist);
			}
		}

		/* Perform post-processing on grid */
		tic(clk);
		grid.remove_outliers();
		toc(clk, "Voxel cleanup");

		/* write voxels if applicable */
		if(conf.voxfile)
		{
			tic(clk);
			ret = writevox(conf.voxfile, grid);
			if(ret)
			{
				PRINT_ERROR("[make_grid]\t"
						"Could not write:");
				PRINT_ERROR(conf.voxfile);
				return PROPEGATE_ERROR(-8, ret);
			}
			toc(clk, "Writing voxels");
		}
	}

	/* success */
	return 0;
}
