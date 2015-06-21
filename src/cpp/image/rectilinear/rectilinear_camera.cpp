#include "rectilinear_camera.h"
#include <io/data/mcd/McdFile.h>
#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/rectilinear/rectilinear_functions.h>
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
 * @file rectilinear_camera.cpp
 * @author Nicholas Corso <ncorso@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file implements the rectilinear_camera_t class functions, 
 * which are used to read and process the intrinsic and extrinsic 
 * calibration, and the poses, for camera imagery with a rectilinear lens.
 */

using namespace std;
using namespace Eigen;
using namespace cv;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

rectilinear_camera_t::rectilinear_camera_t()
{
	/* set default empty values */
	this->poses.clear();
	this->image_directory = "";

	/* set cache to hold only one image at a time */
	this->images.set_capacity(1);
}

rectilinear_camera_t::~rectilinear_camera_t()
{
	/* free all memory and resources */
	this->clear();
}

int rectilinear_camera_t::init(const std::string& calibfile,
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
	if(!calibration.read(calibfile))
		return -1;

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

	/* initialize lists for metadata and transforms */
	this->metadata.resize(infile.get_num_images());
	this->timestamps.resize(infile.get_num_images());
	this->poses.resize(infile.get_num_images());

	/* log the camera name */
	this->cameraName = infile.get_camera_name();

	/* iterate over image frames, storing metadata */
	for(i = 0; i < infile.get_num_images(); i++)
	{
		/* get the next frame info */
		ret = infile.next(this->metadata[i]);
		if(ret)
		{
			cerr << "[rectilinear_camera_t::init]\t"
			     << "Unable to parse"
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
			return PROPEGATE_ERROR(-5, ret);

		/* save timestamp for this frame */
		this->timestamps[i] = this->metadata[i].timestamp; 
	}

	/* clean up */
	infile.close();
	return 0;
}
		
int rectilinear_camera_t::init_mcd(const std::string& mcdfile,
                                   const system_path_t& path)
{
	McdFile infile;
	pose_t p;
	transform_t extrinsics, syspose;
	size_t i, n, r, c;
	int ret;

	/* clear any existing values */
	this->clear();

	/* read in the mcd file */
	if(!(infile.read(mcdfile)))
	{
		/* unable to read */
		cerr << "[rectilinear_camera_t::init_mcd]\tUnable to read "
		     << "mcd file: " << mcdfile << endl;
		return -1;
	}

	/* copy the K-matrix from the MCD file.  NOTE, the mcd
	 * does not store any linear distortion coefficients, so
	 * just leave these as zero */
	this->calibration.set_K(infile.K());

	/* copy translation extrinsics into transform structure */
	for(r = 0; r < 3; r++)
		extrinsics.T(r) = infile.t_cam_to_common()[r];

	/* copy rotation extrinsics */
	for(r = 0; r < 3; r++)
		for(c = 0; c < 3; c++)
			extrinsics.R(r,c) = infile.r_cam_to_common()[3*r+c];

	/* do not know the camera name or image path */
	this->cameraName = infile.serial_num();
	this->image_directory = ""; /* unknown */

	/* copy over the metadata for each image */
	n = infile.num_images();
	this->metadata.resize(n);
	this->timestamps.resize(n);
	this->poses.resize(n);
	for(i = 0; i < n; i++)
	{
		/* copy info into each struct */
		this->metadata[i].image_file = infile.file_name(i);
		this->metadata[i].index      = i;
		this->metadata[i].timestamp  = infile.timestamp(i);
		this->metadata[i].exposure   = -1; /* unknown */
		this->metadata[i].gain       = -1; /* unknown */
		this->timestamps[i]          = infile.timestamp(i);
	
		/* get the pose of this camera at this index */
		ret = path.compute_pose_at(p, this->timestamps[i]);
		if(ret)
		{
			/* unable to compute pose */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[rectilinear_camera_t::init_mcd]\t"
			     << "Error " << ret << ": Unable to compute "
			     << "pose for index #" << i << " of camera "
			     << this->cameraName << endl;
			return ret;
		}
		syspose.T = p.T;
		syspose.R = p.R.toRotationMatrix();

		/* now that we have the pose of the system, apply
		 * the extrinsics from the MCD file to find pose
		 * of camera */
		this->poses[i] = extrinsics;
		this->poses[i].cat(syspose);
	}

	/* success */
	return 0;
}

void rectilinear_camera_t::clear()
{
	/* free lists */
	this->poses.clear();
	this->metadata.clear();
	this->timestamps.clear();

	/* free cache */
	this->images.clear();
	this->image_directory = "";
}

int rectilinear_camera_t::color_point(double px, double py, double pz, double t,
                                  int& r, int& g, int& b, double& q)
{
	string path;
	MatrixXd pt;
	Mat img;
	double point3D[3];
	double point2D[2];
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

	/* get camera u/v coordinates of this point */
	calibration.project_into_image(point3D, point2D);	
	
	/* the code below expects the U and V coordinates to be flipped because 
	* potato, thats why *
	*
	* The rectilinear calibration expects  --> point2D[0]
	*									  |
	*									  |
	*									  \/ point2D[1]
	*
	* However the opencv defines it the other way, so we flip 
	*/
	swap(point2D[0], point2D[1]);

	/* get the image matrix */
	path = this->image_directory + this->metadata[i].image_file;
	ret = this->images.get(path, img);
	if(ret)
	{
		cerr << "[rectilinear_camera_t::color_point]\tCould not get"
		     << " image: \"" << this->metadata[i].image_file 
		     << "\" with full path \"" << path << "\"" << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* check if out of bounds */
	if(point2D[0] >= 0 && point2D[0] < img.size().height
		&& point2D[1] >= 0 && point2D[1] < img.size().width)
	{
		
		/* Check if there is a mask */
		if(!this->mask.empty())
		{
			/* check if this point is masked out */
			if(!this->mask.at<unsigned char>((int)point2D[0], (int)point2D[1]))
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
		q = point3D[2] / sqrt( (point3D[0]*point3D[0])
			+(point3D[1]*point3D[1])+(point3D[2]*point3D[2]) );
	}
	else
		q = -DBL_MAX; /* bad quality */

	/* success */
	return 0;
}
