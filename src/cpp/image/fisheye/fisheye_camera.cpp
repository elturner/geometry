#include "fisheye_camera.h"
#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/fisheye/ocam_functions.h>
#include <geometry/transform.h>
#include <geometry/system_path.h>
#include <util/error_codes.h>
#include <util/binary_search.h>
#include <vector>
#include <string>
#include <new>
#include <math.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <opencv2/opencv.hpp>

/**
 * @file fisheye_camera.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the fisheye_camera_t class functions, which are used
 * to read and process the intrinsic and extrinsic calibration, and the
 * poses, for camera imagery with a fisheye lens.
 */

using namespace std;
using namespace Eigen;
using namespace cv;

/* function implementations */

fisheye_camera_t::fisheye_camera_t()
{
	/* set default empty values */
	this->poses.clear();
	this->image_directory = "";

	/* set cache to hold only one image at a time */
	this->images.set_capacity(1);
}

fisheye_camera_t::~fisheye_camera_t()
{
	/* free all memory and resources */
	this->clear();
}

int fisheye_camera_t::init(const std::string& calibfile,
                           const std::string& metafile,
                           const std::string& imgdir,
                           const system_path_t& path)
{
	color_image_reader_t infile;
	string name;
	int i, ret;

	/* clear any existing values */
	this->clear();

	/* attempt to read the calibration file */
	ret = get_ocam_model_bin(&(this->calibration),
                                 name, calibfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* store image directory location with a trailing slash */
	this->image_directory = imgdir;
	if(!imgdir.empty() 
			&& (*(imgdir.rbegin()) != '/') 
			&& (*(imgdir.rbegin()) != '\\'))
		this->image_directory += "/";

	/* read in metadata */
	ret = infile.open(metafile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* check that files match */
	if(name.compare(infile.get_camera_name()) != 0)
		return PROPEGATE_ERROR(-3, ret); /* different names */

	/* save the camera name */
	this->cameraName = infile.get_camera_name();

	/* initialize lists for metadata and transforms */
	this->metadata.resize(infile.get_num_images());
	this->timestamps.resize(infile.get_num_images());
	this->poses.resize(infile.get_num_images());

	/* iterate over image frames, storing metadata */
	for(i = 0; i < infile.get_num_images(); i++)
	{
		/* get the next frame info */
		ret = infile.next(this->metadata[i]);
		if(ret)
		{
			cerr << "[fisheye_camera_t::init]\tUnable to parse"
			     << " metadata #" << i << "/"
			     << infile.get_num_images() << " from "
			     << metafile << endl;
			return PROPEGATE_ERROR(-4, ret);
		}

		/* compute transform of camera at this frame */
		ret = path.compute_transform_for(this->poses[i],
		                    this->metadata[i].timestamp,
		                    infile.get_camera_name());
		if(ret)
		{
			cerr << "[fisheye_camera_t::init]\tUnable to "
			     << "get transform for camera \""
			     << infile.get_camera_name() << "\" ("
			     << infile.get_camera_name().size() << ")"
			     << endl;
			return PROPEGATE_ERROR(-5, ret);
		}

		/* save timestamp for this frame */
		this->timestamps[i] = this->metadata[i].timestamp; 
	}

	/* clean up */
	infile.close();
	return 0;
}

void fisheye_camera_t::clear()
{
	/* free lists */
	this->poses.clear();
	this->metadata.clear();
	this->timestamps.clear();

	/* free cache */
	this->images.clear();
	this->image_directory = "";
}

int fisheye_camera_t::color_point(double px, double py, double pz, double t,
                                  int& r, int& g, int& b, double& q)
{
	string path;
	MatrixXd pt;
	Mat img;
	double point3D[3];
	double point2D[2];
	double tmp;
	int i, ret;

	/* find the closest camera, with respect to time */
	i = binary_search::get_closest_index(this->timestamps, t);
	if(i < 0 || i >= (int) this->timestamps.size())
		return -1; /* invalid index */

	/* get the position of this point in camera 3D coordinates */
	pt.resize(3, 1);
	pt(0,0) = px;
	pt(1,0) = py;
	pt(2,0) = pz;
	this->poses[i].apply_inverse(pt);
	point3D[0] = pt(0,0);
	point3D[1] = pt(1,0);
	point3D[2] = pt(2,0);

	/* check if point is behind camera */
	if(point3D[2] < 0)
	{
		q = -DBL_MAX; /* bad quality */
		return 0; /* don't color, not seen by camera */
	}

	/* the fisheye library assumes camera coordinates use +z facing
	 * into the camera, so switch x and y, and negate z */
	tmp = point3D[0];
	point3D[0] = point3D[1];
	point3D[1] = tmp;
	point3D[2] = -point3D[2];

	/* get camera u/v coordinates of this point */
	world2cam(point2D, point3D, &(this->calibration));

	/* get the image matrix */
	path = this->image_directory + this->metadata[i].image_file;
	ret = this->images.get(path, img);
	if(ret)
	{
		cerr << "[fisheye_camera_t::color_point]\tCould not get"
		     << " image: \"" << this->metadata[i].image_file 
		     << "\" with full path \"" << path << "\"" << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* check if out of bounds */
	if(point2D[0] >= 0 && point2D[0] < this->calibration.height
		&& point2D[1] >= 0 && point2D[1] < this->calibration.width)
	{
		/* Check if there is a mask */
		if(!this->mask.empty())
		{
			/* check if this point is masked out */
			if(!this->mask.at<unsigned char>(
				(int)point2D[0], (int)point2D[1]))
			{
				q = -DBL_MAX;
				return 0;
			}
		}

		/* get color from these coordinates */
		b = img.at<Vec3b>((int)point2D[0], (int)point2D[1])[0];
		g = img.at<Vec3b>((int)point2D[0], (int)point2D[1])[1];
		r = img.at<Vec3b>((int)point2D[0], (int)point2D[1])[2];
	
		/* record normalized z-component as the quality 
		 * of this coloring */
		q = -point3D[2] / sqrt( (point3D[0]*point3D[0])
			+(point3D[1]*point3D[1])+(point3D[2]*point3D[2]) );
	}
	else
		q = -DBL_MAX; /* bad quality */

	/* success */
	return 0;
}
		
