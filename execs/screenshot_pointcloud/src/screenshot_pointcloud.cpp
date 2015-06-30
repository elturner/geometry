/*
	screenshot_pointcloud.h

	Specifies the settings structure and entry function for the 
	screenshot_pointcloud code
*/
#include "screenshot_pointcloud.h"

/* includes */
#include <string>
#include <fstream>
#include <sstream>

#include <util/error_codes.h>
#include <util/progress_bar.h>
#include <util/tictoc.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

/* namespaces */
using namespace std;
using namespace cv;

/* class decleartions */
namespace sp
{
	/* class that holds the vitals about the pointcloud */
	class PointCloudStats
	{
	public:
		double xlims[2];
		double ylims[2];
		double zlims[2];
		size_t numPoints;
	};

	/* class that holds the image mapping */
	class ImageMapper
	{
		// This holds the offsets 
		int offSetX, offSetY;

		// This is the size of the images
		size_t sizeX, sizeY;

		// This is the limits in the x and y
		double xlims[2], ylims[2];

		// this is the resolution
		double resolution;

	public:

		/* creates the ImapeMapper */
		ImageMapper(const PointCloudStats& stats,
			double inResolution)
		{
			/* store the resolution */
			resolution = inResolution;

			/* copy over the limits */
			xlims[0] = stats.xlims[0];
			xlims[1] = stats.xlims[1];
			ylims[0] = stats.ylims[0];
			ylims[1] = stats.ylims[1];

			/* compute what the offset is */
			offSetX = -(int)(xlims[0]/resolution);
			offSetY = -(int)(ylims[0]/resolution);

			/* compute the size */
			sizeX = (size_t)(offSetX + (int)(xlims[1]/resolution))+1;
			sizeY = (size_t)(offSetY + (int)(ylims[1]/resolution))+1;
		};

		/* Function that maps values to the image */
		inline size_t mapX(double x) const 
			{ return (size_t)((int)(x/resolution)+offSetX); };
		inline size_t mapY(double y) const 
			{ return (size_t)((int)(y/resolution)+offSetY); };

		/* getters */
		inline size_t size_x() const {return sizeX;};
		inline size_t size_y() const {return sizeY;};
		inline size_t offset_x() const {return offSetX;};
		inline size_t offset_y() const {return offSetY;};
		inline double res() const {return resolution;};
		inline double x_limits(size_t i) const {return xlims[i];};
		inline double y_limits(size_t i) const {return ylims[i];};
	};

}

/* function declerations */
namespace sp
{
	/*
		int parse_pointcloud_stats(const std::string& pcfile,
			double converstionToMeters,
			sp::PointCloudStats& stats);

		Computes the essential point cloud stats that are needed to create
		the point cloud screenshot
	*/
	int parse_pointcloud_stats(const std::string& pcfile,
		double converstionToMeters,
		sp::PointCloudStats& stats);

	/* 
		int fill_image(const std::string& pcfile,
			const sp::ImageMapper& mapper,
			cv::Mat& image,
			cv::Mat& timeImage,
			double converstionToMeters,
			size_t totalPoints,
			bool skipUncolored)

		Fills the image from the point cloud
	*/
	int fill_image(const std::string& pcfile,
		const sp::ImageMapper& mapper,
		cv::Mat& image,
		cv::Mat& timeImage,
		double converstionToMeters,
		size_t totalPoints,
		bool skipUncolored);

	/*
		int write_image_mapping(const std::string& filename,
			const sp::ImageMapper& mapper);

		Writes the data need for image mapping to a file
	*/
	int write_image_mapping(const std::string& filename,
		const sp::ImageMapper& mapper);

}

/* function definitions */
int sp::run(const sp::AlgSettings& settings)
{

	int ret;
	tictoc_t timer;

	/* calculate the pointcloud stats */
	tic(timer);
	sp::PointCloudStats pcstats;
	ret = parse_pointcloud_stats(settings.inFile, 
		settings.unitConversion,
		pcstats);
	if(ret)
		POST_RETURN_ERROR("Error calculating pointcloud stats",1);
	toc(timer, "Computing bounding box");

	/* create the imasge mapper class */
	ImageMapper mapper(pcstats, settings.imageResolution);
	
	/* Create the output image and depth buffer that will be used */
	Mat image(mapper.size_x(), mapper.size_y(), 
		CV_8UC3, 
		Scalar(settings.backgroundColor[2],
			settings.backgroundColor[1],
			settings.backgroundColor[0]));

	/* Create an output image for the time map */
	Mat timeMap(mapper.size_x(), mapper.size_y(),
		CV_16U,
		Scalar(-1));

	/* fill the image */
	tic(timer);
	ret = fill_image(settings.inFile,
		mapper,
		image,
		timeMap,
		settings.unitConversion,
		pcstats.numPoints,
		settings.ignoreUncolored);
	if(ret)
		POST_RETURN_ERROR("Error filling image",2);
	toc(timer, "Creating pointcloud image");

	/* write the output image */
	tic(timer);
	if(!imwrite(settings.outImgFile, image))
		POST_RETURN_ERROR("Error writing output image file : " + 
			settings.outImgFile, 3);
	if(!imwrite(settings.outTimeFile, timeMap))
		POST_RETURN_ERROR("Error writting time output :" +
			settings.outTimeFile, 4);

	/* write the mapping */
	ret = write_image_mapping(settings.outCoordFile,
		mapper);
	if(ret)
		POST_RETURN_ERROR("Unable to write output mapping file : " + 
			settings.outCoordFile, 5);
	toc(timer, "Writing output files");

	/* return success */
	return 0;
}

/*
	int parse_pointcloud_stats(const std::string& pcfile,
		double converstionToMeters,
		sp::PointCloudStats& stats);

	Computes the essential point cloud stats that are needed to create
	the point cloud screenshot
*/
int sp::parse_pointcloud_stats(const std::string& pcfile,
	double converstionToMeters,
	sp::PointCloudStats& stats)
{

	/* open the input point cloud file */
	ifstream pcStream(pcfile);
	if(!pcStream.is_open())
		POST_RETURN_ERROR("Unable to open pointcloud file : " + pcfile, 1);

	/* reset the stats */
	stats.numPoints = 0;
	stats.xlims[0] = stats.ylims[0] = stats.zlims[0] = 1e30;
	stats.xlims[1] = stats.ylims[1] = stats.zlims[1] = -1e30;

	/* read the file line by line */
	string line;
	stringstream ss;
	double x,y,z;
	progress_bar_t bar;
	bar.set_name("Computing bounding box");
	bar.set_color(progress_bar_t::BLUE);
	while(pcStream.good())
	{
		/* read the line */
		getline(pcStream, line);
		if(line.empty())
			continue;

		/* tell the user the code is still going */
		if(!(stats.numPoints % 10000))
		{
			bar.update();
		}

		/* get the x y and z points */
		ss.clear();
		ss.str(line);
		ss >> x; ss >> y; ss >> z;

		/* add another point to the counter */
		stats.numPoints++;

		/* update the bounding box */
		if(x < stats.xlims[0])
			stats.xlims[0] = x;
		if(x > stats.xlims[1])
			stats.xlims[1] = x;
		if(y < stats.ylims[0])
			stats.ylims[0] = y;
		if(y > stats.ylims[1])
			stats.ylims[1] = y;
		if(z < stats.zlims[0])
			stats.zlims[0] = z;
		if(z > stats.zlims[1])
			stats.zlims[1] = z;
	}

	/* convert to meter units */
	stats.xlims[0] *= converstionToMeters;
	stats.xlims[1] *= converstionToMeters;
	stats.ylims[0] *= converstionToMeters;
	stats.ylims[1] *= converstionToMeters;
	stats.zlims[0] *= converstionToMeters;
	stats.zlims[1] *= converstionToMeters;

	/* return success */
	return 0;

}

/* 
	int fill_image(const std::string& pcfile,
		const sp::ImageMapper& mapper,
		cv::Mat& image,
		cv::Mat& timeImage,
		double converstionToMeters,
		size_t totalPoints,
		bool skipUncolored)

	Fills the image from the point cloud
*/
int sp::fill_image(const std::string& pcfile,
	const sp::ImageMapper& mapper,
	cv::Mat& image,
	cv::Mat& timeImage,
	double converstionToMeters,
	size_t totalPoints,
	bool skipUncolored)
{

	/* open up the point cloud file */
	ifstream pcStream(pcfile);
	if(!pcStream.is_open())
		POST_RETURN_ERROR("Unable to open pointcloud file : " + pcfile, 1);

	/* create a zbuffer */
	Mat zbuffer(mapper.size_x(), mapper.size_y(), 
		CV_32F, 
		Scalar(-1e30));	

	/* Create a progress bar so that we can see the progress.  This will */
	/* take a long ass time */
	progress_bar_t bar;
	bar.set_color(progress_bar_t::BLUE);
	bar.set_name("Creating Image");

	/* read the point cloud line by line */
	string line;
	stringstream ss;
	double x,y,z,idx,t;
	int r,g,b;
	size_t mappedX, mappedY;
	size_t numPts = 0;
	while(pcStream.good())	
	{

		/* update our progress bar */
		if(!(numPts % 10000))
			bar.update(numPts, totalPoints);

		/* read the line and skip if nothing on it */
		getline(pcStream, line);
		if(line.empty())
			continue;

		/* increment point cound */
		numPts++;

		/* extract the lines data */
		ss.clear();
		ss.str(line);
		ss >> x; ss >> y; ss >> z;
		ss >> r; ss >> g; ss >> b; 
		ss >> idx; ss >> t;

		/* check if this point should be skipped due to it being black */
		if(skipUncolored && r == 0 && g == 0 && b == 0 )
			continue;

		/* map the point */
		mappedX = mapper.mapX(x*converstionToMeters);
		mappedY = mapper.mapY(y*converstionToMeters);

		/* check if this is blocked in the zbuffer */
		if(zbuffer.at<float>(mappedX,mappedY) > z)
			continue;

		/* assign the point and update the z buffer */
		zbuffer.at<float>(mappedX,mappedY) = z;
		image.at<Vec3b>(mappedX,mappedY)[0] = b;
		image.at<Vec3b>(mappedX,mappedY)[1] = g;
		image.at<Vec3b>(mappedX,mappedY)[2] = r;
		timeImage.at<unsigned short>(mappedX, mappedY) = (unsigned short)(10*t);
	}


	/* return succes */
	return 0;
}

/*
	int write_image_mapping(const std::string& filename,
		const sp::ImageMapper& mapper);

	Writes the data need for image mapping to a file
*/
int sp::write_image_mapping(const std::string& filename,
	const sp::ImageMapper& mapper)
{

	/* open the output stream */
	ofstream outStream(filename);
	if(!outStream.is_open())
		POST_RETURN_ERROR("Unable to open output file : " + filename, 1);

	/* write the resolution */
	outStream << mapper.res() << endl;

	/* write the offsets */
	outStream << mapper.offset_x() << endl << mapper.offset_y() << endl;

	/* return success */
	return 0;
}