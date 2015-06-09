#include "scanorama_maker.h"
#include "scanorama.h"
#include <io/mesh/mesh_io.h>
#include <image/camera.h>
#include <image/fisheye/fisheye_camera.h>
#include <geometry/system_path.h>
#include <geometry/raytrace/OctTree.h>
#include <geometry/raytrace/Triangle3.h>
#include <util/error_codes.h>
#include <iostream>
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
	vector<Triangle3<float> > empty;

	this->path.clear();
	this->cameras.clear();
	this->model.rebuild(empty);
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
	this->cameras.push_back(
			make_shared<fisheye_camera_t>());

	/* initialize */
	ret = this->cameras.back()->init(calibfile, metafile, 
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
		cerr << "[scanorama_maker_t::populate_octree]\tERROR " << ret
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

	/* success */
	return 0;
}
