/*
	split_image_by_floorplan

	This splits an input point cloud image by floorplan
*/
#include "split_image_by_floorplan.h"

/* includes */
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdio>

#include <mesh/floorplan/floorplan.h>
#include <util/error_codes.h>
#include <util/progress_bar.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <boost/filesystem.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/io/svg/write_svg.hpp>

/* namespaces */
using namespace std;
using namespace cv;

/* typedefs */
typedef boost::geometry::model::d2::point_xy<double> point_xy;
typedef boost::geometry::model::polygon<point_xy> polygon;

/* function declerations */
namespace sf
{
	/* class that holds the image mapping */
	class ImageMapper
	{
		// This holds the offsets 
		int offSetX, offSetY;

		// this is the resolution
		double resolution;

	public:

		/* constructor */
		ImageMapper() {};
		ImageMapper(int offX, int offY, double res) :
			offSetX(offX), offSetY(offY), resolution(res) {};
		
		/* read function */
		int read(const std::string& file)
		{
			ifstream instream(file);
			if(!instream.is_open())
				POST_RETURN_ERROR("Unable to open file : " + file, 1);
			instream >> resolution;
			instream >> offSetX;
			instream >> offSetY;
			return 0;
		}
		int write(const std::string& file) const
		{
			ofstream outstream(file);
			if(!outstream.is_open())
				POST_RETURN_ERROR("Unable to open output file : " + file, 1);
			outstream << resolution << endl
					  << offSetX << endl
					  << offSetY << endl
					  << endl
					  << "To map from image coordiantes to model coordinates : " << endl
					  << " model_x = (image_x - offsetX)*resolution" << endl
					  << " model_y = (image_y - offsetY)*resolution" << endl
					  << endl
					  << "The order above is : resolution, offsetX, and offsetY" << endl
					  << endl
					  << "Image Coordinate System : " << endl
					  << " -------------------> +y" << endl
					  << " |" << endl
					  << " |" << endl
					  << " |" << endl
					  << " |" << endl
					  << " |" << endl
					  << " |" << endl
					  << "\\/" << endl
					  << "+x" << endl;
			return 0;
		}

		/* Function that maps values to the image */
		inline size_t mapX(double x) const 
			{ return (size_t)((int)(x/resolution)+offSetX); };
		inline size_t mapY(double y) const 
			{ return (size_t)((int)(y/resolution)+offSetY); };

		/* getters */
		inline size_t offset_x() const {return offSetX;};
		inline size_t offset_y() const {return offSetY;};
		inline double res() const {return resolution;};
	};

	/* class to hold aabb information */
	class Aabb
	{
		public:
			double minx, maxx, miny, maxy;
			inline double area() const 
			{ return (maxx-minx)*(maxy-miny); };
	};


	/*
		int compute_aabb(const fp::floorplan_t& fp,
			const vector<vector<int> >& boundary_list,
			sf::Aabb& box);

		Computes the aabb of a room given the boundary list
	*/
	int compute_aabb(const fp::floorplan_t& fp,
		const vector<vector<int> >& boundary_list,
		sf::Aabb& box);

	/*
		int trim_room_image(const fp::floorplan_t fp,
			const vector<vector<int> >& boundary_list,
			const ImageMapper& mapper,
			Mat& roomimage,
			unsigned char const backgroundColor[3])

		Trims out the non-room pixels from the image and replaces them with
		background color 
	*/
	int trim_room_image(const fp::floorplan_t fp,
		const vector<vector<int> >& boundary_list,
		const ImageMapper& mapper,
		Mat& roomimage,
		unsigned char const backgroundColor[3]);

}

/* function definitions */
int sf::run(const sf::AlgSettings& settings)
{

	int ret;

	/* first thing we need to do is read the input image files */
	Mat pcImage = imread(settings.pointcloud_image_file);
	if(pcImage.empty())
		POST_RETURN_ERROR("Unable to read image file : " 
			+ settings.pointcloud_image_file, 1);

	/* read the time mapping image */
	Mat timeImage = imread(settings.time_map_file, IMREAD_ANYDEPTH);
	if(timeImage.empty())
		POST_RETURN_ERROR("Unable to read time image : "
			+ settings.time_map_file, 100);

	/* read the coordinate mapping file */
	sf::ImageMapper mapper;
	ret = mapper.read(settings.coord_mapping_file);
	if(ret)
		POST_RETURN_ERROR("Unable to read coordiante mapping file : "
			+ settings.coord_mapping_file, 2);

	/* read the floorplan */
	fp::floorplan_t fp;
	ret = fp.import_from_fp(settings.floorplan_file);
	if(ret)
		POST_RETURN_ERROR("Unable to read floorplan file : "
			+ settings.floorplan_file, 3);

	/* create the output folder */
	/* check if the folder already exists */
	boost::system::error_code ec;
	boost::filesystem::path outDir(settings.output_prefix);
	if(!boost::filesystem::exists(outDir, ec))
		if(!boost::filesystem::create_directories(outDir, ec))
			POST_RETURN_ERROR("Unable to create output directory",4);

	/* for each room we need to do the following */
	progress_bar_t bar;
	bar.set_name("Making Room Images");
	bar.set_color(progress_bar_t::BLUE);
	for(size_t i = 0; i < fp.rooms.size(); i++)
	{
		bar.update(i, fp.rooms.size());

		/* compute the bounding box for the room */
		vector<vector<int> > boundary_list;
		fp.compute_oriented_boundary(boundary_list,
			fp.rooms[i].tris);

		/* compute the bounding box of the room */
		sf::Aabb box;
		ret = compute_aabb(fp, boundary_list, box);
		if(ret)
			POST_RETURN_ERROR("Unable to compute bounding box!",5);

		/* compute the bounds of the subimage corrisponding to this room */
		size_t minx = mapper.mapX(box.minx);
		size_t miny = mapper.mapY(box.miny);
		size_t maxx = mapper.mapX(box.maxx);
		size_t maxy = mapper.mapY(box.maxy);

		/* create a new mapper for this image */
		ImageMapper thisMapper(mapper.offset_x()-minx,
			mapper.offset_y()-miny,
			mapper.res());

		/* create the room image */
		Mat roomImage = pcImage(
			cv::Rect(Point2i(miny,minx),Point2i(maxy,maxx))
			).clone();

		/* Create the time room image */
		Mat timeRoomImage = timeImage(
			cv::Rect(Point2i(miny,minx),Point2i(maxy,maxx))
			).clone();

		/* lastly we need to blank out the portions of the image that are not */
		/* within the actual boundary of the room */
		ret = trim_room_image(fp, boundary_list, 
			thisMapper, roomImage,
			settings.backgroundColor);

		/* print out the results */
		char filenum[5];
		snprintf(filenum, 5, "%04lu", i);
		boost::filesystem::path p;
		
		p = settings.output_prefix;
		p /= (string("room") + filenum + string(".png"));
		if(!imwrite(p.string(), roomImage))
			POST_RETURN_ERROR("Unable to write image : " + p.string(), 6);
		p = settings.output_prefix;
		p /= (string("coordinate_mapping_room") + filenum + string(".txt"));
		ret = thisMapper.write(p.string());
		if(ret)
			POST_RETURN_ERROR("Unable to write coordinate mapping file : "
				+ p.string(), 7);
		p = settings.output_prefix;
		p /= (string("time_mapping_room") + filenum + string(".png"));
		if(!imwrite(p.string(), timeRoomImage))
			POST_RETURN_ERROR("Unable to write time room image : " + 
				p.string(), 8);
	}

	/* return success */
	return 0;
}

int sf::compute_aabb(const fp::floorplan_t& floorplan,
	const vector<vector<int> >& boundary_list,
	sf::Aabb& box)
{

	box.minx = box.miny = 1e30;
	box.maxx = box.maxy = -1e30;
	for(size_t i = 0; i < boundary_list.size(); i++)
		for(size_t j = 0; j < boundary_list[i].size(); j++)
		{
			if(floorplan.verts[boundary_list[i][j]].x < box.minx)	
				box.minx = floorplan.verts[boundary_list[i][j]].x;
			if(floorplan.verts[boundary_list[i][j]].x > box.maxx)	
				box.maxx = floorplan.verts[boundary_list[i][j]].x;
			if(floorplan.verts[boundary_list[i][j]].y < box.miny)	
				box.miny = floorplan.verts[boundary_list[i][j]].y;
			if(floorplan.verts[boundary_list[i][j]].y > box.maxy)	
				box.maxy = floorplan.verts[boundary_list[i][j]].y;
		}

	/* return success */
	return 0;
}

int sf::trim_room_image(const fp::floorplan_t fp,
	const vector<vector<int> >& boundary_list,
	const ImageMapper& mapper,
	Mat& roomImage,
	unsigned char const backgroundColor[3])
{

	/* for each of the boundary lists we make an aabb */
	vector<sf::Aabb> boxes(boundary_list.size());
	for(size_t i = 0; i < boundary_list.size(); i++)
	{
		sf::Aabb& box = boxes[i];
		box.minx = box.miny = 1e30;
		box.maxx = box.maxy = -1e30;
		for(size_t j = 0; j < boundary_list[i].size(); j++)
		{
			if(fp.verts[boundary_list[i][j]].x < box.minx)	
				box.minx = fp.verts[boundary_list[i][j]].x;
			if(fp.verts[boundary_list[i][j]].x > box.maxx)	
				box.maxx = fp.verts[boundary_list[i][j]].x;
			if(fp.verts[boundary_list[i][j]].y < box.miny)	
				box.miny = fp.verts[boundary_list[i][j]].y;
			if(fp.verts[boundary_list[i][j]].y > box.maxy)	
				box.maxy = fp.verts[boundary_list[i][j]].y;
		}
	}

	/* find the largest box */
	vector<bool> isHole(boundary_list.size(), true);
	double maxArea = 0; size_t maxIdx = 0;
	for(size_t i = 0; i < boxes.size(); i++)
	{
		double area = boxes[i].area();
		if(area > maxArea)
		{
			maxArea = area;
			maxIdx = i;
		}
	}
	isHole[maxIdx] = false;

	/* then we need to create a boost polygon from this */
	polygon room;

	/* create the externior of the room */
	for(size_t i = boundary_list[maxIdx].size(); i != 0; i--)
		boost::geometry::append(room, 
			boost::make_tuple( 
				mapper.mapX(fp.verts[boundary_list[maxIdx][i-1]].x), 
				mapper.mapY(fp.verts[boundary_list[maxIdx][i-1]].y)));
	boost::geometry::append( room,
			boost::make_tuple(
				mapper.mapX(fp.verts[boundary_list[maxIdx].back()].x),
				mapper.mapY(fp.verts[boundary_list[maxIdx].back()].y)) );

	/* allocate space for the interior rings */
	if(boundary_list.size() > 1)
	{
		boost::geometry::interior_rings(room).resize(boundary_list.size()-1);

		/* add the interior rings */
		size_t ringIdx = 0;
		for(size_t i = 0; i < boundary_list.size(); i++)
		{
			/* if this is the boundary then skip it */
			if(!isHole[i])
				continue;

			/* insert the points */
			for(size_t j = 0; j < boundary_list[i].size(); j++)
				boost::geometry::append(room,
					boost::make_tuple(
						mapper.mapX(fp.verts[boundary_list[i][j]].x),
						mapper.mapY(fp.verts[boundary_list[i][j]].y)),
					ringIdx);
			boost::geometry::append(room,
					boost::make_tuple(
						mapper.mapX(fp.verts[boundary_list[i][0]].x),
						mapper.mapY(fp.verts[boundary_list[i][0]].y)),
					ringIdx);


			/* increment ring index */
			ringIdx++;
		}
	}

	/* then we need to iterate over the axis of the image checking to see if */
	/* they are on the inside of the room */
	for(size_t i = 0; i < (size_t) roomImage.rows; i++)
	{
		for(size_t j = 0; j < (size_t) roomImage.cols; j++)
		{
			/* check if this point is inside the room */
			if(boost::geometry::disjoint(room, point_xy(i,j)))
			{
				roomImage.at<Vec3b>(i,j)[0] = backgroundColor[2];
				roomImage.at<Vec3b>(i,j)[1] = backgroundColor[1];
				roomImage.at<Vec3b>(i,j)[2] = backgroundColor[0];
			}
		}
	}

	/* return success */
	return 0;
}
