#include "camera.h"
#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/image_cache.h>
#include <geometry/transform.h>
#include <geometry/system_path.h>
#include <util/error_codes.h>
#include <vector>
#include <string>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <Eigen/StdVector>

/**
 * @file     camera.h
 * @author   Eric Turner <elturner@eecs.berkeley.edu>
 * @brief    An abstract camera class
 *
 * @section DESCRIPTION
 *
 * This class is used to represent a camera from our scanning system.
 * Cameras represent a series of imagery taken over time, and can
 * be represented by any model, including fisheye, rectilinear, etc.
 *
 */

/* namespaces */
using namespace std;
using namespace cv;

/*----------------------*/
/* function definitions */
/*----------------------*/

int camera_t::load_mask(const std::string& maskFileName)
{
	this->mask = imread(maskFileName, IMREAD_GRAYSCALE);
	if(this->mask.empty())
	{
		cerr << "Error! Unable to load mask file " 
		     << maskFileName << endl;
		return -1;
	}

	/* return success */
	return 0;
}

int camera_t::color_point_antialias(double px, double py, double pz, 
                                  double rad, double t,
                                  int& r, int& g, int& b, double& q)
{
	const size_t num_samples = 7;
	size_t i;
	double dx[] = {0.0, 1.0, 0.0, 0.0, -1.0,  0.0,  0.0};
	double dy[] = {0.0, 0.0, 1.0, 0.0,  0.0, -1.0,  0.0};
	double dz[] = {0.0, 0.0, 0.0, 1.0,  0.0,  0.0, -1.0};
	double q_dummy;
	int red[num_samples];
	int green[num_samples];
	int blue[num_samples];
	int ret;

	/* this function samples the color space seven times,
	 * once in the center of the point, and in the six
	 * coordinate directions. */

	/* first, get the center sample */
	red[0] = green[0] = blue[0] = 0;
	ret = this->color_point(px,py,pz,t,red[0],green[0],blue[0],q);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* if given trivial radius, just call the other version
	 * of this function */
	if(rad <= 0)
	{
		/* copy over colors */
		r = red[0];
		g = green[0];
		b = blue[0];
		
		/* no other processing required */
		return 0; 
	}

	/* sample at the six other locations */
	for(i = 1; i < num_samples; i++)
	{
		/* get the color at this position */
		red[i] = green[i] = blue[i] = 0;
		ret = this->color_point(
				px + (dx[i]*rad),
				py + (dy[i]*rad),
				pz + (dz[i]*rad),
				t, red[i], green[i], blue[i],
				q_dummy); /* only keep original q */
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}

	/* return the average of all samples */
	r = g = b = 0;
	for(i = 0; i < num_samples; i++)
	{
		/* add this sample to sum */
		r +=   red[i];
		g += green[i];
		b +=  blue[i];
	}
	
	/* normalize */
	r /= num_samples;
	g /= num_samples;
	b /= num_samples;

	/* success */
	return 0;
}
