#include "scanorama_maker.h"
#include "scanorama.h"
#include <io/mesh/mesh_io.h>
#include <image/camera.h>
#include <image/fisheye/fisheye_camera.h>
#include <geometry/system_path.h>
#include <geometry/raytrace/OctTree.h>
#include <geometry/raytrace/Triangle3.h>
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
	this->path.clear();
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
		
int scanorama_maker_t::add_camera(const std::string& metafile,
		               const std::string& calibfile,
		               const std::string& imgdir)
{
	int ret;

	/* push a new fisheye camera object */
	this->cameras.push_back(fisheye_camera_t());

	/* initialize */
	ret = this->cameras.back().init(calibfile, metafile, 
			imgdir, this->path);
	if(ret)
	{
		/* unable to init */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[scanorama_maker_t::add_camera]\tERROR " << ret
		     << ": Unable to initialize camera" << endl;
		return ret;
	}

	/* success */
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
				const std::vector<double>& times,
				size_t r, size_t c, double bw)
{
	scanorama_t scan;
	stringstream ss;
	ofstream outfile;
	size_t i, n;
	int ret;

	/* iterate over the list of timestamps */
	n = times.size();
	for(i = 0; i < n; i++)
	{
		/* populate the scan */
		ret = this->populate_scanorama(scan, times[i], r, c, bw);
		if(ret)
		{
			/* error occurred */
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
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[scanorama_maker_t::generate_all]\t"
			     << "ERROR " << ret << ": Unable to open "
			     << "scanorama file: " << ss.str() << endl;
			return ret;
		}
		
		/* write it to disk */
		scan.writeptx(outfile);
		outfile.close();
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
