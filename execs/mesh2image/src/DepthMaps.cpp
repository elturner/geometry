/*
	DepthMaps.cpp
*/
#include "DepthMaps.h"

/* includes */
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include <boost/filesystem.hpp>
#include <boost/threadpool.hpp>

#include <eigen3/Eigen/Dense>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdio>

#include <io/mesh/mesh_io.h>
#include <io/images/cam_pose_file.h>
#include <io/data/mcd/McdFile.h>
#include <util/tictoc.h>
#include <util/progress_bar.h>
#include "accel_struct/Triangle3.h"
#include "accel_struct/OctTree.h"

/* namspaces */
using namespace std;
using namespace Eigen;
using namespace cv;
using namespace boost::threadpool;
using namespace boost::filesystem;

/* need this for relative paths */
namespace boost 
{
	namespace filesystem 
	{
		template < >
		path& path::append< typename path::iterator >( typename path::iterator begin, typename path::iterator end, const codecvt_type& cvt)
		{
			(void) &cvt;   /* cvt not used */ 
			for( ; begin != end ; ++begin )
		        *this /= *begin;
		    return *this;
		}
		// Return path when appended to a_From will resolve to same as a_To
		boost::filesystem::path make_relative( boost::filesystem::path a_From, boost::filesystem::path a_To )
		{
		    a_From = boost::filesystem::absolute( a_From ); a_To = boost::filesystem::absolute( a_To );
		    boost::filesystem::path ret;
		    boost::filesystem::path::const_iterator itrFrom( a_From.begin() ), itrTo( a_To.begin() );
		    // Find common base
		    for( boost::filesystem::path::const_iterator toEnd( a_To.end() ), fromEnd( a_From.end() ) ; itrFrom != fromEnd && itrTo != toEnd && *itrFrom == *itrTo; ++itrFrom, ++itrTo );
		    // Navigate backwards in directory to reach previously found base
		    for( boost::filesystem::path::const_iterator fromEnd( a_From.end() ); itrFrom != fromEnd; ++itrFrom )
		    {
		        if( (*itrFrom) != "." )
		            ret /= "..";
		    }
		    // Now navigate down the directory branch
		    ret.append( itrTo, a_To.end() );
		    return ret;
		}
	}
}


/* DepthMaps namespace */
namespace DepthMaps
{

	/*
	* Copies the mesh into a Triangle3<float> vector
	*/
	bool copy_into_triangles(const mesh_io::mesh_t& mesh,
		vector<Triangle3<float> >& triangle);

	/*
	* Handles the tracing for a set of input pairs
	*/
	bool run_for_pair(const std::string& datasetDir,
		const std::string& mcdFile,
		const std::string& posefile,
		const std::string& output,
		const std::string& camTags,
		const OctTree<float>& tree,
		size_t numThreads,
		double dsFactor);

	/* 
		bool create_output_directory(const std::string& directory)

		Creates the output directory if it does not already exist
	*/
	bool create_output_directory(const std::string& directory);

	/*
		bool create_all_output_directories(const std::string& outdir,
			const std::string& depthFolderName,
			const std::string& normalFolderName);

		Creates all required output directories for an input pair
	*/
	bool create_all_output_directories(const std::string& outdir,
		const std::string& depthFolderName,
		const std::string& normalFolderName);

	/*
		bool get_image_size(const std::string& direct,
			const std::string& file_name,
			Size2i& imageSize);

		deduces image size from this image
	*/
	bool get_image_size(const std::string& direct,
		const std::string& file_name,
		Size2i& imageSize);

	/*
		Eigen::Matrix3f rpy2rot(double roll,
			double pitch, 
			double yaw);

		Converts the roll pitch and yaw to a rotation
		matrix
	*/
	Eigen::Matrix3f rpy2rot(double roll,
		double pitch, 
		double yaw);

	/*
		bool process_image(const OctTree<float>& tree,
			Size2i imageSize,
			const Matrix3f& invK,
			Matrix3f Rcam2world,
			Vector3f Tcam2world,
			string depthFile,
			string normalFile)

		Processes the image 
	*/
	void process_image(const OctTree<float>& tree,
		Size2i imageSize,
		double dsFactor,
		const Matrix3f& invK,
		Matrix3f Rcam2world,
		Vector3f Tcam2world,
		string depthFile,
		string normalFile);

}

/* Function definitions */

/*
	bool generate_depth_maps(const std::string& datasetDir,
		const std::string& modelFile,
		size_t octreeDepth,
		const std::vector<std::string> >& mcdFiles,
		const std::vector<std::string> >& poseFiles,
		const std::vector<std::string> >& outDirs,
		const std::vector<std::string>& cameraTags);

	Main function for depth map generation.
*/
bool DepthMaps::generate_depth_maps(const std::string& datasetDir,
	const std::string& modelFile,
	size_t octreeDepth,
	const std::vector<std::string>& mcdFiles,
	const std::vector<std::string>& poseFiles,
	const std::vector<std::string>& outDirs,
	const std::vector<std::string>& cameraTags,
	size_t numThreads,
	double dsFactor)
{
	tictoc_t timer;
	double elapedTime;

	/* the first thing we need to do is to import the mesh */
	cout << "====== Reading Model ======" << endl;
	tic(timer);
	mesh_io::mesh_t mesh;
	if(mesh.read(modelFile))
	{
		cerr << "Unable to read mesh file : " << modelFile << endl;
		return false;
	}
	else
	{
		elapedTime = toc(timer, NULL);
		cout << " Verts      : " << mesh.num_verts() << '\n'
			 << " Tris       : " << mesh.num_polys() << '\n'
			 << " Color      : " << (mesh.has_color() ? "true" : "false") << '\n'
			 << " Texture    : " << "false" << '\n'
			 << " Read Time  : " << elapedTime << " seconds" << endl << endl;
	}

	/* Then we need to build the OctTree */
	cout << "====== Creating OctTree ======" << endl;
	tic(timer);
	vector<Triangle3<float> > triangles;
	copy_into_triangles(mesh, triangles);
	if(triangles.empty())
	{
		cerr << "No triangles in the mesh.  Aborting." << endl;
		return false;
	}

	/* Then build the OctTree */
	OctTree<float> tree(triangles, octreeDepth);
	elapedTime = toc(timer, NULL);
	cout << " Depth      : " << octreeDepth << '\n'
	     << " Build Time : "  << elapedTime << " seconds " << endl << endl;

	/* Then call the function that will do the algorithm for each of the */
	/* input pairs */
	for(size_t i = 0; i < mcdFiles.size(); i++)
		if(!run_for_pair(datasetDir,
			mcdFiles[i],
			poseFiles[i],
			outDirs[i],
			cameraTags[i],
			tree,
			numThreads,
			dsFactor))
		{
			cerr << "Error running generation for input pair #" << i << endl;
			return false;
		}

	/* return success */
	return true;
}

/*
* Copies the mesh into a Triangle3<float> vector
*/
bool DepthMaps::copy_into_triangles(const mesh_io::mesh_t& mesh,
	vector<Triangle3<float> >& triangles)
{
	float v1[3];
	float v2[3];
	float v3[3];

	/* loop over the triangles creating triangle */
	for(size_t i = 0; i < mesh.num_polys(); i++)
	{
		v1[0] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[0]).x);
		v1[1] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[0]).y);
		v1[2] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[0]).z);
		v2[0] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[1]).x);
		v2[1] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[1]).y);
		v2[2] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[1]).z);
		v3[0] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[2]).x);
		v3[1] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[2]).y);
		v3[2] = (float)(mesh.get_vert(mesh.get_poly(i).vertices[2]).z);
		triangles.push_back(Triangle3<float>(v1, v2, v3, i));
	}
	return true;
}

/*
* Handles the tracing for a set of input pairs
*/
bool DepthMaps::run_for_pair(const std::string& datasetDir,
	const std::string& mcdFile,
	const std::string& posefile,
	const std::string& outputDir,
	const std::string& camTag,
	const OctTree<float>& tree,
	size_t numThreads,
	double dsFactor)
{

	/********************************************/
	/* Do the prep work for this pair of images */
	/********************************************/

	/* Print out the header */
	cout << "====== " <<  camTag << " ======" << endl;

	/* First thing we need to do is read in the input files */
	cam_pose_file_t poses;
	if(!poses.read(posefile))
	{
		cerr << "Unable to read camera pose file : " << posefile << endl;
		return false;
	}

	/* then we need to read the mcd file to get the file names */
	McdFile mcd;
	if(!mcd.read(mcdFile))
	{
		cerr << "Unable to read mcd file : " 
			<< mcdFile << endl;
		return false;
	}
	if(mcd.num_images() == 0)
	{
		cerr << "No images found in mcd file : " 
			<< mcdFile << endl;
		return false;
	}

	/* Then we need to create the output directories */
	if(!create_all_output_directories(outputDir,
		"depthmaps",
		"normalmaps"))
		return false;

	/* Then we need to ping the first image to get the image size */
	Size2i imgSize;
	if(!get_image_size(datasetDir,
		mcd.file_name(0)+".jpg",
		imgSize))
	{
		cerr << "Unable to deduce image sizes from first image."
			<< endl;
		return false;
	}

	/* Compute the new image size after downsampling */
	int oldwidth = imgSize.width;
	imgSize.width /= dsFactor;
	imgSize.height /= dsFactor;
	
	/* Recompute the ds factor */
	dsFactor = oldwidth/(double)imgSize.width;

	/* Compute the inverse K mapping */
	Matrix3f K;
	K << mcd.K()[0], mcd.K()[1], mcd.K()[2], 
		 mcd.K()[3], mcd.K()[4], mcd.K()[5],
		 mcd.K()[6], mcd.K()[7], mcd.K()[8];
	Matrix3f invK;
	invK = K.inverse();

	/* Print rest of banner */
	cout << " Image Size : " << imgSize << endl;
	cout << " Ds Factor  : " << dsFactor << endl;
	cout << " Num Images : " << mcd.num_images() << endl;
	cout << " Num Thread : " << numThreads << endl;

	/******************************************/
	/* Prep the output log file               */
	/******************************************/
	path depthlogfile = outputDir;
	depthlogfile /= "depthmaps";
	depthlogfile /= (camTag + "_imagelog.txt");
 	ofstream dlogStream(depthlogfile.string().c_str());
 	if(!dlogStream.is_open())
 	{
 		cerr << "Unable to open output image log file" 
 			<< depthlogfile.string() << endl;
 		return false;
 	}
 	dlogStream << camTag << endl;
 	dlogStream << mcd.num_images() << endl;
	for(size_t i = 0; i < 9; i++)
		dlogStream << mcd.K()[i] << " ";
	dlogStream << endl;
	dlogStream << dsFactor << endl;

	path normallogfile = outputDir;
	normallogfile /= "normalmaps";
	normallogfile /= (camTag + "_imagelog.txt");
 	ofstream nlogStream(normallogfile.string().c_str());
 	if(!nlogStream.is_open())
 	{
 		cerr << "Unable to open output image log file" 
 			<< depthlogfile.string() << endl;
 		return false;
 	}
 	nlogStream << camTag << endl;
 	nlogStream << mcd.num_images() << endl;
	for(size_t i = 0; i < 9; i++)
		nlogStream << mcd.K()[i] << " ";
	nlogStream << endl;
	nlogStream << dsFactor << endl;

	/******************************************/
	/* Do the actual processing of the images */
	/******************************************/

	/* Create the thread pool */
	pool tp(numThreads);

	/* create progress bar */
	progress_bar_t bar;
	bar.set_color(progress_bar_t::BLUE);
	bar.set_name("Depth Mapping");

	/* need this for making relative directories */
	path inDir = datasetDir;

	/* Loop over the images creating the files */
	for(size_t i = 0; i < mcd.num_images(); i++)
	{
		/* find the pose of this image */
		size_t poseIdx = poses.get_nearest_idx(mcd.timestamp(i));

		/* create the pose for this image */
		Vector3f Tcam2world;
		Tcam2world << poses.pose(poseIdx).x(),
			poses.pose(poseIdx).y(),
			poses.pose(poseIdx).z();
		Matrix3f Rcam2world = rpy2rot(poses.pose(poseIdx).roll(),
			poses.pose(poseIdx).pitch(),
			poses.pose(poseIdx).yaw());

		/* make the image names */
		path imageBase = mcd.file_name(i);
		imageBase = imageBase.filename().stem();

		path p1 = outputDir;
		p1 /= "depthmaps";
		p1 /= (imageBase.string() + "_depthmap.png");
		string depthFile = p1.string();
		path p2 = outputDir;
		p2 /= "normalmaps";
		p2 /= (imageBase.string() + "_normalmap.png");
		string normalFile = p2.string();

		/* write to the log files */
		path p1r = make_relative(inDir, p1);
		path p2r = make_relative(inDir, p2);
		nlogStream << mcd.timestamp(i) << " " << p2r.string() << endl;
		dlogStream << mcd.timestamp(i) << " " << p1r.string() << endl;

		/* run the image */
		tp.schedule(boost::bind(
			process_image,
			boost::cref(tree),
			imgSize,
			dsFactor,
			boost::cref(invK),
			Rcam2world,
			Tcam2world,
			depthFile,
			normalFile));
	}

	tictoc_t timer;
	tic(timer);
	while(!tp.empty())
	{
		bar.update(mcd.num_images()-(tp.pending()+tp.active()),mcd.num_images());
		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	}
	bar.clear();
	double elapsedTime = toc(timer, NULL);
	cout << " Total Time : " << elapsedTime << " seconds" << endl << endl;
	
	/* return success */
	return true;
}

/* 
	bool create_output_directory(const std::string& directory)

	Creates the output directory if it does not already exist
*/
bool DepthMaps::create_output_directory(const std::string& directory)
{
	/* check if the folder already exists */
	boost::system::error_code ec;
	boost::filesystem::path outDir(directory);
	if(boost::filesystem::exists(outDir, ec))
		return true;

	/* else we need to attempt to create it */
	return boost::filesystem::create_directories(outDir);
}

/*
	bool create_all_output_directories(const std::string& outdir,
		const std::string& depthFolderName,
		const std::string& normalFolderName);

	Creates all required output directories for an input pair
*/
bool DepthMaps::create_all_output_directories(const std::string& outputDir,
	const std::string& depthFolderName,
	const std::string& normalFolderName)
{
	path depthDir = outputDir;
	depthDir /= depthFolderName;
	path normalDir = outputDir;
	normalDir /= normalFolderName;
	if(!create_output_directory(depthDir.string()))
	{
		cerr << "Unable to create output directory : " 
			<< depthDir.string() << endl;
		return false;
	}
	if(!create_output_directory(normalDir.string()))
	{
		cerr << "Unable to create output directory : " 
			<< normalDir.string() << endl;
		return false;
	}
	return true;
}

/*
	bool get_image_size(const std::string& direct,
		const std::string& file_name,
		Size2i& imageSize);

	deduces image size from this image
*/
bool DepthMaps::get_image_size(const std::string& direct,
	const std::string& file_name,
	Size2i& imageSize)
{
	path p = direct;
	p /= file_name;
	Mat img = imread(p.string());
	if(img.data == NULL)
		return false;
	imageSize = img.size();
	return true;
}

/*
	Eigen::Matrix3f rpy2rot(double roll,
		double pitch, 
		double yaw);

	Converts the roll pitch and yaw to a rotation
	matrix
*/
Matrix3f DepthMaps::rpy2rot(double roll,
	double pitch, 
	double yaw)
{
	Matrix3f R;
	float cr = cos(roll);
	float sr = sin(roll);
	float cp = cos(pitch);
	float sp = sin(pitch);
	float cy = cos(yaw);
	float sy = sin(yaw);

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
	bool process_image(const OctTree<float>& tree,
		Size2i imageSize,
		const Matrix3f& invK,
		Matrix3f Rcam2world,
		Vector3f Tcam2world,
		string depthFile,
		string normalFile)

	Processes the image 
*/
void DepthMaps::process_image(const OctTree<float>& tree,
	Size2i imageSize,
	double dsFactor,
	const Matrix3f& invK,
	Matrix3f Rcam2world,
	Vector3f Tcam2world,
	string depthFile,
	string normalFile)
{
	/* first thing we need to do is allocate an image buffer for */
	/* each of the normal and depth information */
	Mat depthMap(imageSize, CV_16UC1);
	Mat normalMap(imageSize, CV_16UC3);

	/* Then simply loop over the pixels creating the vectors and tracing */
	/* the depth */
	float o[3] = {Tcam2world(0), Tcam2world(1), Tcam2world(2)};
	float d[3], inter[3];
	float n[3];
	float depthVal;
	size_t triangleId;
	Map<Matrix<float,3,1> > direction(d);
	Map<Matrix<float,3,1> > origin(o);
	Map<Matrix<float,3,1> > intersection(inter);
	Map<Matrix<float,3,1> > normal(n);
	unsigned short maxNum = ((1<<16) - 1);
	for(size_t i = 0; i < (size_t) imageSize.height; i++)
	{
		for(size_t j = 0; j < (size_t) imageSize.width; j++)
		{
			/* create the uv of the pixel */
			direction << dsFactor*j, dsFactor*i, 1;
			direction = invK*direction;
			direction /= direction.norm();

			/* convert direction to world coordinates */
			direction = Rcam2world*direction;

			/* do the ray trace */
			if(!tree.ray_trace(o,d,inter,&triangleId))
			{
				depthVal = 0;
				normal(0) = 0;
				normal(1) = 0;
				normal(2) = 0;
			}
			else
			{
				depthVal = (intersection-origin).norm();
				normal(0) = tree.triangle(triangleId).normal(0);
				normal(1) = tree.triangle(triangleId).normal(1);
				normal(2) = tree.triangle(triangleId).normal(2);
			}

			/* store the depth val */
			depthMap.at<unsigned short>(i,j) = (unsigned short)(depthVal*100.0);			

			/* put it in camera coordinates */
			normal = Rcam2world.transpose()*normal;

			normalMap.at<Vec<unsigned short, 3> >(i,j)[0]
				= (unsigned short)(((normal(0) + 1)/2.0)*maxNum);
			normalMap.at<Vec<unsigned short, 3> >(i,j)[1]
				= (unsigned short)(((normal(1) + 1)/2.0)*maxNum);
			normalMap.at<Vec<unsigned short, 3> >(i,j)[2]
				= (unsigned short)(((normal(2) + 1)/2.0)*maxNum);

		}
	}

	/* write out the files */
	imwrite(normalFile, normalMap);
	imwrite(depthFile, depthMap);
}


