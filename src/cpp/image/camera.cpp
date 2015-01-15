/*
	This class provides an abstract version of the camera_t class
*/
#include "camera.h"

/* includes */
#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/image_cache.h>
#include <geometry/transform.h>
#include <geometry/system_path.h>
#include <vector>
#include <string>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <Eigen/StdVector>

/* namespaces */
using namespace std;
using namespace cv;

/* function definitions */
int camera_t::load_mask(const std::string& maskFileName)
{
	this->mask = imread(maskFileName, IMREAD_GRAYSCALE);
	if(this->mask.empty())
	{
		cerr << "Error! Unable to load mask file " << maskFileName << endl;
		return -1;
	}

	/* return success */
	return 0;
}
