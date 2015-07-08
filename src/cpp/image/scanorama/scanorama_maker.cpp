#include "scanorama_maker.h"
#include "scanorama.h"
#include <io/scanorama/scanolist_io.h>
#include <io/mesh/mesh_io.h>
#include <image/camera.h>
#include <image/fisheye/fisheye_camera.h>
#include <image/rectilinear/rectilinear_camera.h>
#include <geometry/system_path.h>
#include <geometry/raytrace/OctTree.h>
#include <geometry/raytrace/Triangle3.h>
#include <util/progress_bar.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <Eigen/Dense>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

/**
 * @file    scanorama_maker.cpp
 * @author  Eric Turner <elturner@indoorreality.com>
 * @brief   Generates scanorama_t objects based on dataset products
 *
 * @section DESCRIPTION
 *
 * Combines imagery and models in order to raytrace a new set of point
 * clouds from specified positions, color those point clouds appropriately,
 * then export those as gridded scanoramas.
 *
 * Created on June 8, 2015
 */

using namespace std;
using namespace Eigen;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void scanorama_maker_t::clear()
{
	size_t i, n;

	/* free memory for the path */
	this->path.clear();
	
	/* the cameras are dynamically allocated, and
	 * need to be explicitly deleted */
	n = this->cameras.size();
	for(i = 0; i < n; i++)
		delete (this->cameras[i]);

	/* clear the list */
	this->cameras.clear();
}
		
int scanorama_maker_t::init(const std::string& pathfile,
				const std::string& configfile,
				const std::string& modelfile)
{
	int ret;

	/* first, clear any existing data */
	this->clear();

	/* read the path data */
	ret = this->path.read(pathfile);
	if(ret)
	{
		/* unable to parse */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[scanorama_maker_t::init]\tERROR " << ret
		     << ": Unable to read path information" << endl;
		return ret;
	}
	
	/* read the hardware config xml file */
	ret = this->path.parse_hardware_config(configfile);
	if(ret)
	{
		/* unable to parse */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[scanorama_maker_t::init]\tERROR " << ret
		     << ": Unable to read xml config file" << endl;
		return ret;
	}

	/* parse the model file */
	ret = this->populate_octree(modelfile);
	if(ret)
	{
		/* unable to parse */
		ret = PROPEGATE_ERROR(-3, ret);
		cerr << "[scanorama_maker_t::init]\tERROR " << ret
		     << ": Unable to import model info" << endl;
		return ret;
	}

	/* success */
	return 0;
}
		
int scanorama_maker_t::add_fisheye_camera(const std::string& metafile,
		               const std::string& calibfile,
		               const std::string& imgdir)
{
	fisheye_camera_t* cam;
	int ret;

	/* make a new fisheye camera object and initialize */
	cam = new fisheye_camera_t();
	ret = cam->init(calibfile, metafile, imgdir, this->path);
	if(ret)
	{
		/* unable to init */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[scanorama_maker_t::add_fisheye_camera]\t"
		     << "ERROR " << ret
		     << ": Unable to initialize camera" << endl;
		return ret;
	}

	/* success */
	this->cameras.push_back(cam);
	return 0;
}
		
int scanorama_maker_t::add_rectilinear_camera(const std::string& metafile,
                                              const std::string& calibfile,
                                              const std::string& imgdir)
{
	rectilinear_camera_t* cam;
	int ret;

	/* make a new rectilinear camera object and initialize */
	cam = new rectilinear_camera_t();
	ret = cam->init(calibfile, metafile, imgdir, this->path);
	if(ret)
	{
		/* unable to init */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[scanorama_maker_t::add_rectilinear_camera]\t"
		     << "ERROR " << ret
		     << ": Unable to initialize camera" << endl;
		return ret;
	}

	/* success */
	this->cameras.push_back(cam);
	return 0;
}
		
int scanorama_maker_t::populate_scanorama(scanorama_t& scan,
				double t, size_t r, size_t c, double bw)
{
	pose_t p;
	size_t i, n;
	int ret;

	/* get the position of the system at this timestamp */
	ret = this->path.compute_pose_at(p, t);
	if(ret)
	{
		/* error occurred */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[scanorama_maker_t::populate_scanorama]\t"
		     << "ERROR " << ret << ": could not get pose."
		     << endl;
		return ret;
	}

	/* prepare the point geometry for this scanorama */
	ret = scan.init_geometry(this->model, t, p.T, r, c, bw);
	if(ret)
	{
		/* error occurred */
		ret = PROPEGATE_ERROR(-2, ret);
		cerr << "[scanorama_maker_t::populate_scanorama]\t"
		     << "ERROR " << ret << ": could not init "
		     << "geometry." << endl;
		return ret;
	}

	/* attempt to color this geometry with the camera imagery */
	n = this->cameras.size();
	for(i = 0; i < n; i++)
	{
		/* for each camera, attempt to color as many points
		 * as possible. */
		ret = scan.apply(this->cameras[i]);
		if(ret)
		{
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[scanorama_maker_t::populate_scanorama]\t"
			     << "ERROR " << ret << ": could not color "
			     << "with imagery from camera #" << i 
			     << "." << endl;
			return ret;
		}
	}

	/* success */
	return 0;
}
		
int scanorama_maker_t::generate_all(const std::string& prefix_out,
				const std::string& meta_out,
				const std::vector<double>& times,
				size_t r, size_t c, double bw,
				int begin_idx, int end_idx)
{
	progress_bar_t progbar;
	scanolist_io::scanolist_t metaoutfile;
	scanorama_t scan;
	stringstream ss;
	ofstream outfile;
	tictoc_t clk;
	size_t i, n;
	int ret;

	/* prepare the progress bar for user */
	tic(clk);
	progbar.set_name("Generating scans");
	progbar.set_color(progress_bar_t::PURPLE);

	/* determine what subset of times should be generated */
	n = times.size();
	if(begin_idx < 0)
		begin_idx = 0; /* can't go before zero */
	if(end_idx > (int) n || end_idx < 0)
		end_idx = (int) n; /* can't go past end */

	/* initialize the output metadata */
	metaoutfile.set_dims(r, c);
	n = this->cameras.size();
	for(i = 0; i < n; i++)
		metaoutfile.add_camera(this->cameras[i]->name());

	/* iterate over the list of timestamps */
	for(i = (size_t) begin_idx; i < (size_t) end_idx; i++)
	{
		/* update progress bar 
		 *
		 * base progress only on the subset that will
		 * actually be generated */
		progbar.update( (i - begin_idx) , (end_idx - begin_idx) );

		/* populate the scan */
		ret = this->populate_scanorama(scan, times[i], r, c, bw);
		if(ret)
		{
			/* error occurred */
			progbar.clear();
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[scanorama_maker_t::generate_all]\t"
			     << "ERROR " << ret << ": Unable to generate "
			     << "scan #" << i << " at timestamp "
			     << times[i] << endl;
			return ret;
		}

		/* determine where to export this scanorama */
		ss.clear();
		ss.str("");
		ss << prefix_out;
		ss << std::setfill('0') << std::setw(8) << i;
		ss << ".ptx";

		/* open file for writing */
		outfile.open(ss.str());
		if(!(outfile.is_open()))
		{
			/* error occurred */
			progbar.clear();
			ret = -2;
			cerr << "[scanorama_maker_t::generate_all]\t"
			     << "ERROR " << ret << ": Unable to open "
			     << "scanorama file: " << ss.str() << endl;
			return ret;
		}
		
		/* write it to disk */
		scan.writeptx(outfile);
		outfile.close();

		/* write to png as well */
		ss.clear();
		ss.str("");
		ss << prefix_out;
		ss << std::setfill('0') << std::setw(8) << i;
		ss << ".png";
		ret = scan.writepng(ss.str());
		if(ret)
		{
			/* unable to export png */
			progbar.clear();
			cerr << "[scanorama_maker_t::generate_all]\t"
			     << "ERROR " << ret << ": Unable to export "
			     << "scanorama #" << i << " as a PNG image"
			     << endl;
			return PROPEGATE_ERROR(-3, ret);
		}
		
		/* store metadata */
		metaoutfile.add(
			scanolist_io::scanometa_t(i, times[i], ss.str()));
	}

	/* if specified, write the metadata to output file */
	if(!(meta_out.empty()))
	{
		ret = metaoutfile.write(meta_out);
		if(ret)
		{
			progbar.clear();
			cerr << "[scanorama_maker_t::generate_all]\t"
			     << "Unable to write output metadata file: \""
			     << meta_out << "\"" << endl;
			return ret;
		}
	}

	/* success */
	progbar.clear();
	toc(clk, "Generating scans");
	return 0;
}
		
int scanorama_maker_t::generate_along_path(const std::string& prefix_out,
			const std::string& meta_out,
			double minspacedist, double maxspacedist,
			size_t r, size_t c, double bw,
			int begin_idx, int end_idx)
{
	vector<double> times;
	tictoc_t clk;
	size_t i_chosen, i_start, i_end, n;
	int ret;
	
	/* begin processing */
	tic(clk);

	/* we want to align the scanoramas with the camera poses,
	 * not just any system pose.  So use a camera as reference */

	/* first check if there are any cameras */
	if(this->cameras.empty())
	{
		/* can't operate without cameras */
		ret = -1;
		cerr << "[scanorama_maker_t::generate_along_path]\t"
		     << "ERROR " << ret << ": No cameras found."
		     << endl;
		return ret;
	}

	/* get the positions of the first camera */
	const std::vector<transform_t,
	            Eigen::aligned_allocator<transform_t> >& camposes
			    = this->cameras[0]->get_poses();
	const std::vector<double>& camtimes 
			= this->cameras[0]->get_timestamps();

	/* we want to iterate over the path to find poses for
	 * the scanoramas */
	n = camposes.size();
	if(n <= 0)
	{
		ret = -2;
		cerr << "[scanorama_maker_t::generate_along_path]\t"
		     << "ERROR " << ret << ": No poses defined in path"
		     << endl;
		return ret; /* no poses */
	}

	/* put the first scanorama at the first pose that occurs
	 * after the start-time of the path */
	i_start = 0;
	while(camtimes[i_start] <= this->path.starttime())
	{
		/* current pose is too early, try next pose */
		i_start++;

		/* make sure we don't run out of poses */
		if(i_start >= n) /* don't want to reach last pose yet */
		{
			/* all cam poses are before start time */
			ret = -3;
			cerr << "[scanorama_maker_t::generate_along_path]\t"
			     << "ERROR " << ret << ": No poses occur at "
			     << "valid times." << endl;
			return ret;
		}
	}
	
	/* the first image is usually not the best, since the camera has
	 * not adapted to its conditions yet.  So start one the second
	 * image of the dataset. 
	 *
	 * Now determine which of the first few poses should be used
	 * as the first scanorama. */
	i_start++;
	i_end = this->index_jump_by_dist(
			camposes, /* the list of poses to search */
			i_start, /* start searching here */
			0, /* measure distance from pose #0 */
			maxspacedist-minspacedist); /* distance to search */
	i_chosen = this->get_best_index(camtimes, i_start, i_end);
	if(i_chosen >= n)
	{
		/* went past end of list */
		ret = -4;
		cerr << "[scanorama_maker_t::generate_along_path]\t"
		     << "Couldn't find valid first pose.  Camera "
		     << this->cameras[0]->name() << " has "
		     << n << " images.  Is that enough?" << endl;
		return ret;
	}

	/* insert as our first scanorama pose */
	times.push_back(camtimes[i_chosen]);

	/* iterate through the path.  Determine the distance spacing between
	 * poses in order to figure out where to place the generated
	 * scanoramas. */
	while(i_chosen < n && i_end < n)
	{
		/* determine when path goes relevant distances */
		i_start = this->index_jump_by_dist(camposes,
				i_chosen+1, i_chosen, minspacedist);
		i_end = this->index_jump_by_dist(camposes,
				i_start,    i_chosen, maxspacedist);

		/* now we know that camposes[i_start] is at least
		 * minspacedist from camposes[i_chosen], and camposes[i_end]
		 * is at least maxspacedist from camposes[i_chosen] 
		 *
		 * find the best index in this range to convert to
		 * a scanorama */
		i_chosen = this->get_best_index(camtimes, i_start, i_end);
		if(i_chosen >= n)
			break; /* reached end of list, stop iterating */

		/* now that we've selected the desired pose, add its
		 * timestamp to our list so we can put a scanorama there */
		times.push_back(camtimes[i_chosen]);
	}
	toc(clk, "Locating poses");
	
	/* now that we've populated the times list, make the scanoramas */
	ret = this->generate_all(prefix_out, meta_out,
			times, r, c, bw, begin_idx, end_idx);
	if(ret)
	{
		/* some error occurred during processing */
		ret = PROPEGATE_ERROR(-5, ret);
		cerr << "[scanorama_maker_t::generate_along_path]\t"
		     << "ERROR" << ret << ": Unable to generate poses"
		     << endl;
		return ret;
	}

	/* success */
	return 0;
}
		
int scanorama_maker_t::populate_octree(const std::string& modelfile)
{
	mesh_io::mesh_t mesh;
	vector<Triangle3<float> > triangles;
	float v1[3];
	float v2[3];
	float v3[3];
	int ret;

	/* first read mesh from disk */
	ret = mesh.read(modelfile);
	if(ret)
	{
		/* error occurred */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[scanorama_maker_t::populate_octree]"
		     << "\tERROR " << ret
		     << ": Unable to parse mesh file: " << modelfile
		     << endl;
		return ret;
	}

	/* loop over the triangles creating triangle */
	for(size_t i = 0; i < mesh.num_polys(); i++)
	{
		v1[0] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[0]).x);
		v1[1] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[0]).y);
		v1[2] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[0]).z);
		v2[0] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[1]).x);
		v2[1] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[1]).y);
		v2[2] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[1]).z);
		v3[0] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[2]).x);
		v3[1] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[2]).y);
		v3[2] = (float)(mesh.get_vert(
					mesh.get_poly(i).vertices[2]).z);
		triangles.push_back(Triangle3<float>(v1, v2, v3, i));
	}

	/* move triangles to octree */
	if(!(this->model.rebuild(triangles)))
	{	
		/* error occurred */
		ret = -2;
		cerr << "[scanorama_maker_t::populate_octree]"
		     << "\tERROR " << ret
		     << ": Unable to init octree" << endl;
		return ret;
	}

	/* success */
	return 0;
}
		
size_t scanorama_maker_t::index_jump_by_dist(
			const std::vector<transform_t,
	        	    Eigen::aligned_allocator<transform_t> >& poses,
			size_t i_start, size_t i_ref, 
			double min_dist) const
{
	double dist_sq, min_dist_sq;
	size_t i, num_poses;

	/* check valid input */
	num_poses = poses.size();
	if(i_ref >= num_poses || i_start >= num_poses)
		return num_poses; /* invalid input */

	/* precompute square of min-dist */
	min_dist_sq = min_dist * min_dist;

	/* iterate over poses until condition met */
	for(i = i_start; i < num_poses; i++)
	{
		/* get the distance between current pose and the
		 * reference point */
		dist_sq = poses[i_ref].dist_sq(poses[i]);

		/* check if min distance met */
		if(dist_sq >= min_dist_sq)
			return i; /* found first valid pose */
	}

	/* no match found, so return end-of-list */
	return num_poses;
}
		
size_t scanorama_maker_t::get_best_index(const std::vector<double>& times,
				size_t i_start, size_t i_end) const
{
	double w, w_min;
	size_t i, i_min, num_times;
	
	/* initialize variables */
	w_min     = DBL_MAX;
	i_min     = i_start;
	num_times = times.size();

	/* iterate over the range of indices */
	for(i = i_start; i < i_end && i < num_times; i++)
	{
		/* get rotational speed at i */
		w = this->path.rotational_speed_at(times[i]);
		if(w < w_min)
		{
			/* update min value */
			i_min = i;
			w_min = w;
		}
	}

	/* return best position */
	return i_min;
}
