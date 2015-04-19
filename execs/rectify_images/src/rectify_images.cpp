/*
	rectify_images.cpp
*/

#include "rectify_images.h"

/* includes */
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <chrono>

#include <io/data/color_image/color_image_metadata_reader.h>
#include <image/fisheye/ocam_functions.h>
#include <util/progress_bar.h>
#include <util/tictoc.h>

#include <opencv/cxcore.h>
#include <opencv/highgui.h>

#include <boost/filesystem.hpp>
#include <boost/threadpool.hpp>

/* namspaces */
using namespace std;
using namespace cv;
using namespace boost::threadpool;
using namespace boost::filesystem;

/* defines */
#define IMAGEMASK_NAME "mask.bmp"

/* make relative function for boost */
namespace boost 
{
	namespace filesystem 
	{
		template < >
		path& path::append< typename path::iterator >( typename path::iterator begin, typename path::iterator end, const codecvt_type& cvt)
		{
		    (void) &cvt;
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

/* function declerations */
namespace rectify_images
{

	/*
		bool create_undistortion_mask(const double* K,
			const double* rvcam,
			ocam_model& calibParameters,
			Mat& mapX,
			Mat& mapY);

		Creates the undistortion map for the given K matrix, ocam model,
		and rotation matrix
	*/
	bool create_undistortion_mask(const double* K,
		const double* rvcam,
		ocam_model& calibParameters,
		Mat& mapX,
		Mat& mapY);

	/*
		void rpy2rot(Mat& R,
			double roll,
			double pitch,
			double yaw);

		Makes rotation matrix from roll pitch and yaw
	*/
	void rpy2rot(Mat& R,
		double roll,
		double pitch,
		double yaw);


	/* 
		bool create_output_directory(const std::string& directory)

		Creates the output directory if it does not already exist
	*/
	bool create_output_directory(const std::string& directory);

	/*
		vector<string> collect_image_names(
			color_image_reader_t& imageMetaData,
			vector<double>& timestamps);

		Gets all of the files names of the images in the
		meta data file
	*/
	vector<string> collect_image_names(
		color_image_reader_t& imageMetaData,
		vector<double>& timestamps);

	/*
		void rectify(const std::string& dataSetDir,
			const std::string& imgDir,
			const std::string& outputDir,
			const std::string& fileName,
			Mat& mapX,
			Mat& mapY);

		The function that will be run by the thread pool that does the actual
		rectification
	*/
	void rectify(const std::string& dataSetDir,
		const std::string& imgDir,
		const std::string& outputDir,
		const std::string& fileName,
		Mat& mapX,
		Mat& mapY);

	/*
		bool copy_mask(const std::string& maskFileName,
			cv::Size& imageSize,
			const std::string& outputDirectory);

		Copies the mask into the output directory after resizing
	*/
	bool copy_mask(const std::string& maskFileName,
		cv::Size& imageSize,
		const std::string& outputDirectory);

	/*
		bool write_mcd(const InParams& params,
			vector<string>& fileList,
			vector<double>& timestamps);

		Writes the mcd file 
	*/
	bool write_mcd(const InParams& params,
		vector<string>& fileList,
		vector<double>& timestamps);



}

/* function definitions */

/*
	int run(const InParams& params)

	Runs the rectification process.  See above for a description of the
	parameters to the code.

	Returns zero on success and non-zero on failure
*/
int rectify_images::run(const InParams& params)
{
	int ret;
	tictoc_t timer;

	/* First thing we need to do is parse the input metadata file and scrape */
	/* the image names that we need to process */
	color_image_reader_t imageMetaData;
	ret = imageMetaData.open(params.metaDataFile.c_str());
	if(ret)
	{
		cerr << "[rectify_images::run] - Unable to open camera metadata file "
			<< params.metaDataFile << endl;
		return 1;
	}

	/* Then we need to collect all of the images names from the file */
	vector<double> timestamps;
	vector<string> imageFiles = collect_image_names(imageMetaData, timestamps);

	/* Then we need to import the calibration parameters */
	ocam_model calibParameters;
	string cameraName;
	ret = get_ocam_model_bin(&calibParameters, 
		cameraName, 
		params.cameraCalibrationFile);
	if(ret)
	{
		cerr << "[rectify_images::run] - Unable to open camera calibration file "
			<< params.cameraCalibrationFile << endl;
		return 2;
	}

	/* Ensure that the output directory exists.  And if not then we create it */
	if(!create_output_directory(params.outputDirectory))
	{
		cerr << "[rectify_images::run] - Unable to create output directory "
			<< params.outputDirectory << endl;
		return 3;
	}

	/* Then we need to create the undistortion mask */
	tic(timer);
	Mat mapX(Size(params.imgSize[1], params.imgSize[0]), CV_32FC1);
	Mat mapY(Size(params.imgSize[1], params.imgSize[0]), CV_32FC1);
	create_undistortion_mask(params.KMatrix,
		params.rVcam,
		calibParameters,
		mapX,
		mapY);
	toc(timer, "Creating Undistortion Mask");

	/* create a threadpool */
	pool tp(params.numThreads);

	/* create a progress bar */
	progress_bar_t bar;
	bar.set_color(progress_bar_t::BLUE);
	bar.set_name("Rectifying Images");

	/* schedule all of the tasks in the pool */
	string datasetDir = params.datasetDirectory;
	string imgDir = imageMetaData.get_output_dir();
	string outputDir = params.outputDirectory;
	for(size_t i = 0; i < imageFiles.size(); i++)
	{
		tp.schedule(boost::bind(rectify,
			boost::cref(datasetDir),
			boost::cref(imgDir),
			boost::cref(outputDir),
			boost::cref(imageFiles[i]),
			boost::ref(mapX),
			boost::ref(mapY)));
	}

	/* Monitor the progress with a progress bar */
	tic(timer);
	while(!tp.empty())
	{
		bar.update(imageFiles.size()-(tp.pending()+tp.active()),imageFiles.size());
		boost::this_thread::sleep(boost::posix_time::milliseconds(250));
	}
	bar.clear();
	toc(timer, "Image Processing");

	/* Then we need to check if we need to do a copy of the mask */
	tic(timer);
	if(!params.imageMaskFile.empty())
	{
		cv::Size maskSize = mapX.size();
		if(!copy_mask(params.imageMaskFile,
			maskSize,
			params.outputDirectory))
		{
			cerr << "[rectify_images::run] - Unable to copy mask file : " 
				<< params.imageMaskFile << endl;
			return 4;
		}
	}
	toc(timer, "Moving Mask File");

	/* Lastly we need to create a spoofed mcd file so that other code can use the images */
	tic(timer);
	if(!write_mcd(params,
		imageFiles,
		timestamps))
	{
		cerr << "[rectify_images::run] - Unable to write MCD file" << endl;
		return 5;
	}
	toc(timer, "Creating MCD File");


	/* return success */
	return 0;
}


/*
	bool create_undistortion_mask(const double* K,
		const double* rvcam,
		ocam_model& calibParameters,
		Mat& mapX,
		Mat& mapY);

	Creates the undistortion map for the given K matrix, ocam model,
	and rotation matrix
*/
bool rectify_images::create_undistortion_mask(const double* K,
	const double* rvcam,
	ocam_model& calibParameters,
	Mat& mapX,
	Mat& mapY)
{

	/* The first thing we need to do is generate an OpenCV matrix version of */
	/* K and its inverse. 													 */
	Mat Kmat(3, 3, CV_32FC1);
	for(size_t i = 0; i < 3; i++)
		for(size_t j = 0; j < 3; j++)
			Kmat.ptr<float>(i)[j] = K[3*i+j];

	/* Then we take the inverse of it */
	Mat Kinv = Kmat.inv();

	/* Then we need to build the rotation matrix given from the values */
	Mat Rvcam;
	rpy2rot(Rvcam, rvcam[0], rvcam[1], rvcam[2]);

	/* Then for each pixel in the target image we need to find the undistorted */
	/* value */
	Mat p(3, 1, CV_32FC1);
	double uv[2];
	double ray[3];
	for(size_t i = 0; i < (size_t) mapX.rows; i++)
	{
		for(size_t j = 0; j < (size_t) mapX.cols; j++)
		{
			/* set the p to be the homogenous pixel location */
			p.ptr<float>(0)[0] = j;
			p.ptr<float>(1)[0] = i;
			p.ptr<float>(2)[0] = 1;

			/* multiply by inverse K */
			p = Kinv*p;

			/* rotate by the given rotation matrix */
			p = Rvcam*p;

			/* swap into dumb coordinates */
			ray[0] = p.ptr<float>(1)[0];
			ray[1] = p.ptr<float>(0)[0];
			ray[2] = -p.ptr<float>(2)[0];

			/* compute what the pixel coordinates should be in the fisheye */
			world2cam(uv, ray, &calibParameters);

			/* copy these into the xy mapping */
			/* the flip here is for opencv conventions */
			mapX.ptr<float>(i)[j] = (float)(uv[1]);
			mapY.ptr<float>(i)[j] = (float)(uv[0]);
		}
	}

	/* return success */
	return true;
}

/*
	void rpy2rot(Mat& R,
		double roll,
		double pitch,
		double yaw);

	Makes rotation matrix from roll pitch and yaw
*/
void rectify_images::rpy2rot(Mat& R,
	double roll,
	double pitch,
	double yaw)
{
	// Force R to be the correct size
	R.create( Size(3,3), CV_32FC1);

	// Cache the trig results
	const double cr = cos(roll);
	const double sr = sin(roll);
	const double cp = cos(pitch);
	const double sp = sin(pitch);
	const double cy = cos(yaw);
	const double sy = sin(yaw);

	// Create the values
	R.ptr<float>(0)[0] = cy*cp;
	R.ptr<float>(0)[1] = cy*sp*sr-sy*cr;
	R.ptr<float>(0)[2] = cy*cr*sp+sy*sr;
	R.ptr<float>(1)[0] = cp*sy;
	R.ptr<float>(1)[1] = sy*sp*sr+cy*cr;
	R.ptr<float>(1)[2] = sy*cr*sp-cy*sr;
	R.ptr<float>(2)[0] = -sp;
	R.ptr<float>(2)[1] = cp*sr;
	R.ptr<float>(2)[2] = cp*cr;
}

/* 
	bool create_output_directory(const std::string& directory)

	Creates the output directory if it does not already exist
*/
bool rectify_images::create_output_directory(const std::string& directory)
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
	vector<string> collect_image_names(
		color_image_reader_t& imageMetaData,
		vector<double>& timestamps);

	Gets all of the files names of the images in the
	meta data file
*/
vector<string> rectify_images::collect_image_names(
	color_image_reader_t& imageMetaData,
	vector<double>& timestamps)
{
	vector<string> fileList;
	timestamps.clear();

	for(size_t i = 0; i < (size_t) imageMetaData.get_num_images(); i++)
	{
		color_image_frame_t imageFrame;
		imageMetaData.next(imageFrame);
		fileList.push_back(imageFrame.image_file);
		timestamps.push_back(imageFrame.timestamp);

	}

	return fileList;
}

/*
	void rectify(const std::string& dataSetDir,
		const std::string& imgDir,
		const std::string& outputDir,
		const std::string& fileName,
		Mat& mapX,
		Mat& mapY);

	The function that will be run by the thread pool that does the actual
	rectification
*/
void rectify_images::rectify(const std::string& dataSetDir,
	const std::string& imgDir,
	const std::string& outputDir,
	const std::string& fileName,
	Mat& mapX,
	Mat& mapY)
{
	/* create the input and output names */
	path pIn(dataSetDir);
	pIn /= imgDir;
	pIn /= fileName;
	path pOut(outputDir);
	pOut /= fileName;

	/* load the image */
	Mat srcImage = imread(pIn.string());
	if(srcImage.data == NULL)
		return;

	/* do the rectification */
	Mat destImage;
	remap(srcImage, destImage, mapX, mapY, CV_INTER_LINEAR, BORDER_CONSTANT, Scalar(0,0,0));

	/* write it to file */
	vector<int> params;
	params.push_back(CV_IMWRITE_JPEG_QUALITY);
	params.push_back(100);
	imwrite(pOut.string(), destImage, params);
}

/*
	bool copy_mask(const std::string& maskFileName,
		cv::Size& imageSize,
		const std::string& outputDirectory);

	Copies the mask into the output directory after resizing
*/
bool rectify_images::copy_mask(const std::string& maskFileName,
	cv::Size& imageSize,
	const std::string& outputDirectory)
{

	/* attempt to load the mask file */
	Mat maskIn = imread(maskFileName);
	if(maskIn.data == NULL)
		return false;

	/* then we need to resize the image */
	Mat maskOut(imageSize.height, imageSize.width, CV_32FC1);
	resize(maskIn, maskOut, imageSize, 0, 0, INTER_NEAREST);

	/* write the output image */
	path p(outputDirectory);
	p /= IMAGEMASK_NAME;
	imwrite(p.string(), maskOut);

	/* return success */
	return true;
}

/*
	bool write_mcd(const InParams& params,
		vector<string>& fileList,
		vector<double>& timestamps);

	Writes the mcd file 
*/
bool rectify_images::write_mcd(const InParams& params,
	vector<string>& fileList,
	vector<double>& timestamps)
{

	/* open the output file */
	path p(params.outputDirectory);
	p /= (params.vcamSerialNumber + ".mcd");
	ofstream f(p.string());
	if(!f.is_open())
		return false;

	/* write the serial number and number of images */
	f << params.vcamSerialNumber << " " << fileList.size() << endl;

	/* then write the K matrix */
	for(size_t i = 0; i < 9; i++)
		f << params.KMatrix[i] << " ";
	f << endl;

	/* create the combined rotation and write it to file */
	Mat Rvtoc;
	Mat Rctoi;
	rpy2rot(Rvtoc, params.rVcam[0], params.rVcam[1], params.rVcam[2]);
	rpy2rot(Rctoi, params.eTransform[0], params.eTransform[1], params.eTransform[2]);
	Mat Rvtoi = Rctoi*Rvtoc;

	for(size_t i = 0; i < 3; i++)
		for(size_t j = 0; j < 3; j++)
			f << Rvtoi.ptr<float>(i)[j] << " ";
	f << endl;

	/* write the translation */
	for(size_t i = 0; i < 3; i++)
		f << params.eTransform[3+i] << " ";
	f << endl;

	/* make the relative directory */
	path p1(params.outputDirectory);
	path p2(params.datasetDirectory);
	path p3 = make_relative(p2, p1);
	
	/* loop over the files writing them */
	for(size_t i = 0; i < fileList.size(); i++)
	{
		path pFile = p3 / fileList[i];
		f << pFile.replace_extension("").string() << " " << timestamps[i] << endl;
	}

	/* write the mask file */
	path pFile = p3 / IMAGEMASK_NAME;
	f << pFile.string() << endl;

	/* return success */
	return true;
}

