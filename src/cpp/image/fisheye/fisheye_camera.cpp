#include "fisheye_camera.h"
#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/fisheye/ocam_functions.h>
#include <geometry/transform.h>
#include <geometry/system_path.h>
#include <util/error_codes.h>
#include <vector>
#include <string>
#include <new>

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

/* function implementations */

fisheye_camera_t::fisheye_camera_t()
{
	/* set default empty values */
	this->clear();
}

fisheye_camera_t::~fisheye_camera_t()
{
	/* free all memory and resources */
	this->clear();
}

int fisheye_camera_t::init(const std::string& calibfile,
                           const std::string& metafile,
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

	/* read in metadata */
	ret = infile.open(metafile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* check that files match */
	if(name.compare(infile.get_camera_name()) != 0)
		return PROPEGATE_ERROR(-3, ret); /* different names */

	/* initialize lists for metadata and transforms */
	this->metadata.resize(infile.get_num_images());
	this->poses = new transform_t[infile.get_num_images()];

	/* iterate over image frames, storing metadata */
	for(i = 0; i < infile.get_num_images(); i++)
	{
		/* get the next frame info */
		ret = infile.next(this->metadata[i]);
		if(ret)
			return PROPEGATE_ERROR(-4, ret);

		/* compute transform of camera at this frame */
		ret = path.compute_transform_for(this->poses[i],
		                    this->metadata[i].timestamp,
		                    infile.get_camera_name());
		if(ret)
			return PROPEGATE_ERROR(-5, ret);
	}

	/* clean up */
	infile.close();
	return 0;
}

void fisheye_camera_t::clear()
{
	/* check for dynamically allocated memory */
	if(this->poses != NULL)
	{
		/* free it */
		delete[] (this->poses);
		this->poses = NULL;
	}

	/* free lists */
	this->metadata.clear();
}
