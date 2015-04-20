/*
	image_mapping.h this file supports the image mapping functionality
*/
#include "image_mapping.h"
#include "accel_struct/imagemap.h"

/* includes */
#include <string>
#include <vector>
#include <fstream>

#include <io/images/NormalLog.h>
#include <io/images/DepthLog.h>
#include <io/images/cam_pose_file.h>
#include <util/progress_bar.h>
#include <util/tictoc.h>

#include <eigen3/Eigen/Dense>

#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include <boost/filesystem.hpp>

/* namesapces */
using namespace std;
using namespace cv;
using namespace Eigen;
using namespace boost::filesystem;

/* function declerations */
namespace ImageMapping
{
	/*
		Eigen::Matrix3f rpy2rot(double roll,
			double pitch, 
			double yaw);

		Converts the roll pitch and yaw to a rotation
		matrix
	*/
	Eigen::Matrix3d rpy2rot(double roll,
		double pitch, 
		double yaw);

	/*
		bool process_image(Matrix3d& Rcam2world,
			Vector3d& Tcam2world,
			Matrix3d& K,
			quadtree_t& tree,
			double dsFactor,
			const string& depthFileName,
			const string& normalFileName)

		Inserts the image points into the quadtree
	*/
	bool process_image(Matrix3d& Rcam2world,
		Vector3d& Tcam2world,
		Matrix3d& K,
		quadtree_t& tree,
		double dsFactor,
		const string& depthFileName,
		const string& normalFileName,
		size_t imageId);
}

/* function definitions */
/*
	int map_images(const std::string& datasetDir,
		const std::vector<std::string>& poseFiles,
		const std::vector<std::string>& depthmaps,
		const std::vector<std::string>& normalmaps,
		const std::string& imapFileName,
		const std::string& keyFileName,
		double resolution);

	The main entry point of the image mapping code
*/
int ImageMapping::map_images(const std::string& datasetDir,
	const std::vector<std::string>& poseFiles,
	const std::vector<std::string>& depthmaps,
	const std::vector<std::string>& normalmaps,
	const std::string& imapFileName,
	const std::string& keyFileName,
	double resolution)
{
	tictoc_t timer;
	double elapsedTime;

	/* first thing we need to do is create a quadtree to store */
	/* all of the points in */
	quadtree_t tree(resolution);

	/* then keep a unique id of the image ids */
	size_t imageId = 0;

	/* open a file for it */
	ofstream idStream(keyFileName.c_str());
	if(!idStream.is_open())
	{
		cout << "Unable to create img id file : " << keyFileName << endl;
		return 10101;
	}

	/* for each of the pairs do the following */
	for(size_t i = 0; i < poseFiles.size(); i++)
	{

		/* read the pose file */
		cam_pose_file_t poses;
		if(!poses.read(poseFiles[i]))
		{
			cerr << "Unable to read pose file : " << poseFiles[i] << endl;
			return 1;
		}

		/* read the depth map log file */
		DepthLog dlog;
		if(!dlog.read(depthmaps[i]))
		{
			cerr << "Unable to read depth map log file : " << depthmaps[i] << endl;
			return 2;
		}

		cout << "====== Mapping " << dlog.name() << " ======" << endl;
		cout << " Num Img    : " << dlog.num_images() << '\n';

		/* read the normal map log file */
		NormalLog nlog;
		if(!nlog.read(normalmaps[i]))
		{
			cerr << "Unable to read normal map log file : " << normalmaps[i] << endl;
			return 3;
		}

		/* Make the K matrix */
		Matrix3d K;
		K << dlog.K()[0], dlog.K()[1], dlog.K()[2],
		     dlog.K()[3], dlog.K()[4], dlog.K()[5],
		     dlog.K()[6], dlog.K()[7], dlog.K()[8];
		Matrix3d invK = K.inverse();

		progress_bar_t bar;
		bar.set_color(progress_bar_t::BLUE);
		bar.set_name("Image Mapping");

		/* loop over the depth maps creating the poses and then */
		/* processing the images */
		tic(timer);
		for(size_t j = 0; j < dlog.num_images(); j++)
		{
			/* update bar */
			bar.update(j,dlog.num_images());

			/* get correct pose idx */
			size_t poseIdx = poses.get_nearest_idx(dlog.timestamp(j));

			/* Make the pose */
			Vector3d Tcam2world;
			Tcam2world << poses.pose(poseIdx).x(), 
						  poses.pose(poseIdx).y(),
						  poses.pose(poseIdx).z();
			Matrix3d Rcam2world = rpy2rot(poses.pose(poseIdx).roll(),
				poses.pose(poseIdx).pitch(),
				poses.pose(poseIdx).yaw());

			path depthFile = datasetDir;
			depthFile /= dlog.file_name(j);
			path normalFile = datasetDir;
			normalFile /= nlog.file_name(j);

			char buf[9];
			snprintf(buf,9,"%08lu",j);
			string imageName = dlog.name() + "_image_" + buf + ".jpg";

			idStream << imageId << " " << imageName << endl;

			/* process the image */
			if(!process_image(Rcam2world,
				Tcam2world,
				invK,
				tree,
				dlog.dsFactor(),
				depthFile.string(),
				normalFile.string(),
				imageId))
			{
				cerr << "Error processing : " << depthFile.string() << endl;
				return 33;
			}
			imageId++;
		}
		bar.clear();
		elapsedTime = toc(timer,NULL);
		cout << " Total Time : " << elapsedTime << " seconds" << endl << endl;
	}
	idStream.close();

	/* Write out the output file */
	ofstream f(imapFileName.c_str());
	if(!f.is_open())
	{
		cerr << "Unable to open output file : " << imapFileName << endl;
		return 99;
	}
	tree.print(f);
	f.close();

	/* return success */
	return 0;
}



/*
	Eigen::Matrix3f rpy2rot(double roll,
		double pitch, 
		double yaw);

	Converts the roll pitch and yaw to a rotation
	matrix
*/
Matrix3d ImageMapping::rpy2rot(double roll,
	double pitch, 
	double yaw)
{
	Matrix3d R;
	double cr = cos(roll);
	double sr = sin(roll);
	double cp = cos(pitch);
	double sp = sin(pitch);
	double cy = cos(yaw);
	double sy = sin(yaw);

	// Fill the matrix
	R(0,0) = cy*cp;
	R(0,1) = cy*sp*sr-sy*cr;
	R(0,2) = cy*cr*sp+sy*sr;
	R(1,0) = cp*sy;
	R(1,1)  = sy*sp*sr+cy*cr;
	R(1,2)  = sy*cr*sp-cy*sr;
	R(2,0)  = -sp;
	R(2,1)  = cp*sr;
	R(2,2)  = cp*cr;
	return R;
}

/*
	bool process_image(Matrix3d& Rcam2world,
		Vector3d& Tcam2world,
		Matrix3d& invK,
		quadtree_t& tree,
		double dsFactor,
		const string& depthFileName,
		const string& normalFileName)

	Inserts the image points into the quadtree
*/
bool ImageMapping::process_image(Matrix3d& Rcam2world,
	Vector3d& Tcam2world,
	Matrix3d& invK,
	quadtree_t& tree,
	double dsFactor,
	const string& depthFileName,
	const string& normalFileName,
	size_t imgid)
{
	/* load the two images into memory */
	Mat depthMap = imread(depthFileName, CV_LOAD_IMAGE_ANYDEPTH);
	if(depthMap.data == NULL)
	{
		cerr << "Unable to read file " << depthFileName << endl;
		return false;
	}
	Mat normalMap = imread(normalFileName, 
		CV_LOAD_IMAGE_COLOR | CV_LOAD_IMAGE_ANYDEPTH);
	if(normalMap.data == NULL)
	{
		cerr << "Unable to read file " << normalFileName << endl;
		return false;
	}

	/* then step through the image and trace through the tree */
	Vector3d ray;
	Size2i imgSize = depthMap.size();
	vector<quaddata_t *> boxes;
	Point2D a, b;
	double d,nz;
	unsigned short maxUShort = ((1<<16) - 1);
	for(size_t i = 0; i < (size_t) imgSize.height; i++)
	{
		for(size_t j = 0; j < (size_t) imgSize.width; j++)
		{
			/* create the ray in space */
			ray << dsFactor*j, dsFactor*i, 1;
			ray = invK*ray;
			ray /= ray.norm();
			ray = Rcam2world*ray;
			
			/* create the intersection point */
			ray = (depthMap.at<unsigned short>(i,j)/100.0)*ray + Tcam2world;

			/* then trace through the grid and find all the points */
			a.x() = Tcam2world(0); a.y() = Tcam2world(1);
			b.x() = ray(0); b.y() = ray(1);
			tree.trace_and_insert(boxes, a, b);

			/* then go through inserting all the points, the weight and label */
			for(size_t k = 0; k < boxes.size(); k++)
			{
				d = sqrt(boxes[k]->pos.sq_dist_to(a));
				nz = 2*((double)(normalMap.at<Vec<unsigned short, 3> >(i,j)[2]))/maxUShort-1;
				tree.insert(boxes[k]->pos, 
					imgid, -1.0/d*nz);
			}
		}
	}

	return true;
}
