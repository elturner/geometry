#ifndef H_POLYGON_H
#define H_POLYGON_H

/*
	Polygon.h

	This class provides an abstraction for boost polygon objects.
*/

/* includes */
#include <vector>
#include <ostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/io/svg/write_svg.hpp>

#include "Point2D.h"

/* typedefs */
typedef boost::geometry::model::d2::point_xy<double> BoostPoint;
typedef boost::geometry::model::polygon<BoostPoint> BoostPolygon;
typedef boost::geometry::model::box<BoostPoint> BoostBox;
typedef boost::geometry::model::segment<BoostPoint> BoostSegment;

/* the Polygon Class */
class Polygon
{
	/* private data members */

	/* This holds the underlying elements of the bounding box.  This is */
	/* the boost representation of the points */
	BoostPolygon _poly;

	/* This contains the boost representation of the polygons axis aligned */
	/* bounding box */
	BoostBox _aabb;

	/* this stores the area of the polygon */
	double _area;

	/* this stores a flag that tells if we need to compute the area */
	bool _isAreaComputed;

public:

	/* public interface */

	/*
		Polygon(const std::vector<Point2D>& verts = std::vector<Point2D>())

		Constructor.  Allows for the class to be constructed using some verts

		NOTE : The verts are assumed to be in CLOCKWISE ordering and that
		the first vertex and the last vertex are identical!
	*/
	Polygon(const std::vector<Point2D>& verts = std::vector<Point2D>());

	/*
		Polygon(const BoostPolygon& poly);

		Construct from boost polygon
	*/
	Polygon(const BoostPolygon& poly);

	/*
		void set_verts(const std::vector<Point2D>& verts)

		Sets the verts of polygon

		NOTE : The verts are assumed to be in CLOCKWISE ordering and that
		the first vertex and the last vertex are identical!
	*/
	void set_verts(const std::vector<Point2D>& verts);


	/* access functions */


	/*
		size_t num_verts() const

		Returns the number of verts
	*/
	inline size_t num_verts() const 
	{
		return boost::geometry::num_points(_poly);
	};

	/*
		double area()

		Returns the precomputed area of the object
	*/
	inline double area()
	{
		if(!_isAreaComputed)
		{
			_area = boost::geometry::area(_poly);
			_isAreaComputed = true;
		}
		return _area;
	};
	inline double area() const
	{
		if(!_isAreaComputed)
		{
			return boost::geometry::area(_poly);
		}
		return _area;
	};

	/*
		double minX() const
		double minY() const
		double maxX() const
		double maxY() const

		Returns the parameters of the axis aligned bounding box
	*/
	inline double minX() const
	{
		return boost::geometry::get<boost::geometry::min_corner, 0>(_aabb);
	};
	inline double minY() const
	{
		return boost::geometry::get<boost::geometry::min_corner, 1>(_aabb);
	};
	inline double maxX() const
	{
		return boost::geometry::get<boost::geometry::max_corner, 0>(_aabb);
	};
	inline double maxY() const
	{
		return boost::geometry::get<boost::geometry::max_corner, 1>(_aabb);
	};

	/*
		inline double vert_x(size_t idx) const
		inline double vert_y(size_t idx) const

		Returns a constant reference to the idx'th vertex's x or y component
	*/
	inline double vert_x(size_t idx) const
	{
		return boost::geometry::get<0>(
			*(boost::geometry::exterior_ring(_poly).begin()+idx));
	};
	inline double vert_y(size_t idx) const
	{
		return boost::geometry::get<1>(
			*(boost::geometry::exterior_ring(_poly).begin()+idx));
	};


	/* simplify functions */

	/*
		void simplify(double distance = 0.1);

		Runs the split-merge algorithm to simplify the polygon.
		Distance sets the threshold used for applying simplification
	*/
	void simplify(double distance = 0.1);


	/* intersection functions */

	/*
		bool contains(double x, double y) const;

		Returns if the point is contained within or on the boarder of
		this polygon
	*/
	bool contains(double x, double y) const;

	/*
		bool intersects(const Polygon& other) const;

		Returns if the current polygon and the other polygon 
		intersect.  Does not compute the actual intersecting
		polygon
	*/
	bool intersects(const Polygon& other) const;

	/*
		bool intersects(double minX,
			double minY,
			double maxX,
			double maxY) const;
		bool intersects(const BoostBox& box) const;

		Tests for intersection of this polygon and an axis aligned 
		box specified by the input parameters
	*/
	bool intersects(double minX,
		double minY,
		double maxX,
		double maxY) const;
	bool intersects(const BoostBox& box) const;

	/*
		bool covers(double minX,
			double minY,
			double maxX,
			double maxY) const;
		bool covers(const BoostBox& box) const;

		Tests if a polygon completely covers an axis aligned bounding box
	*/
	bool covers(double minX,
		double minY,
		double maxX,
		double maxY) const;
	bool covers(const BoostBox& box) const;

	/*
		std::vector<Polygon> intersection(const Polygon& other) const;

		Finds the intersection of this polygon and the other polygon.
		Since the intersection can be multiple polygons, the result
		is returned as a vector of Polygons
	*/
	std::vector<Polygon> intersection(const Polygon& other) const;

	/*
		friend std::ostream& operator<< (std::ostream& , const Polygon&);

		Stream overload
	*/
	friend std::ostream& operator<< (std::ostream& , const Polygon&);

private:

	/* private interface */

};

#endif