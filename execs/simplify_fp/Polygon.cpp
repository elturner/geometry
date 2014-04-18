/*
	Polygon.cpp

	This class provides an abstraction for polygon objects
*/
#include "Polygon.h"

/* includes */
#include <vector>
#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/io/svg/write_svg.hpp>

#include "Point2D.h"

/* namespaces */
using namespace std;

/* function definitions */

/*
	Polygon(const std::vector<Point2D>& verts = std::vector<Point2D>())

	Constructor.  Allows for the class to be constructed using some verts
*/
Polygon::Polygon(const std::vector<Point2D>& verts)
	: _isAreaComputed(false)
{
	set_verts(verts);
}

/*
	Polygon(const BoostPolygon& poly);

	Construct from boost polygon
*/
Polygon::Polygon(const BoostPolygon& poly)
	: _isAreaComputed(false)
{
	_poly = poly;
	boost::geometry::envelope(_poly, _aabb);
}

/*
	void set_verts(const std::vector<Point2D>& verts)

	Sets the verts of polygon
*/
void Polygon::set_verts(const std::vector<Point2D>& verts)
{
	/* clear the polygon of any information */
	_poly.clear();

	/* flag that the area is no longer computed */
	_isAreaComputed = false;

	/* push the indices into the _poly */
	for(size_t i = 0; i < verts.size(); i++)
	{
		boost::geometry::append(
			_poly, 
			boost::make_tuple(verts[i].x(), verts[i].y()));
	}

	/* update the axis aligned bounding box */
	boost::geometry::envelope(_poly, _aabb);
}

/*
	void simplify(double distance);

	Runs the split-merge algorithm to simplify the polygon
*/
void Polygon::simplify(double distance)
{
	/* run the boost simplification code */
	BoostPolygon newPoly;
	boost::geometry::simplify(_poly, newPoly, distance);

	/* set the class data to be the new poly */
	_isAreaComputed = false;
	_poly = newPoly;
	boost::geometry::envelope(_poly, _aabb);
}

/*
	bool contains(double x, double y) const;

	Returns if the point is contained within or on the boarder of
	this polygon
*/
bool Polygon::contains(double x, double y) const
{
	BoostPoint p(x,y);
	return boost::geometry::covered_by(p, _poly);
}

/*
	bool intersects(const Polygon& other) const;

	Returns if the current polygon and the other polygon 
	intersect.  Does not compute the actual intersecting
	polygon
*/
bool Polygon::intersects(const Polygon& other) const
{
	return boost::geometry::intersects(_poly, other._poly);
}

/*
	bool intersects(double minX,
		double minY,
		double maxX,
		double maxY) const;

	Tests for intersection of this polygon and an axis aligned 
	box specified by the input parameters
*/
bool Polygon::intersects(double minX,
	double minY,
	double maxX,
	double maxY) const
{
	BoostBox box(BoostPoint(minX,minY), BoostPoint(maxX,maxY));
	return boost::geometry::intersects(_poly, box);
}
bool Polygon::intersects(const BoostBox& box) const
{
	return boost::geometry::intersects(_poly, box);
}

/*
	bool covers(double minX,
		double minY,
		double maxX,
		double maxY) const;
	bool covers(const BoostBox& box) const;

	Tests if a polygon completely covers an axis aligned bounding box
*/
bool Polygon::covers(double minX,
	double minY,
	double maxX,
	double maxY) const
{
	const BoostBox box(BoostPoint(minX,minY), BoostPoint(maxX, maxY));
	return covers(box);
}
bool Polygon::covers(const BoostBox& box) const
{
	/* check if this is a zero sized polygon */
	if(num_verts() == 0)
		return false;

	/* For each edge of this polygon, we check if it intersects the box */
	for(size_t i = 1; i < num_verts(); i++)
	{
		const BoostSegment seg(BoostPoint(vert_x(i), vert_y(i)),
			BoostPoint(vert_x(i-1), vert_y(i-1)));
		if(boost::geometry::intersects(box, seg))
			return false;
	}

	/* if no lines intersect than it is either completely inside or */
	/* completely outside */
	const double minX = boost::geometry::get<boost::geometry::min_corner, 0>(box);
	const double minY = boost::geometry::get<boost::geometry::min_corner, 1>(box);
	return contains(minX,minY);
}

/*
	std::vector<Polygon> intersection(const Polygon& other) const;

	Finds the intersection of this polygon and the other polygon.
	Since the intersection can be multiple polygons, the result
	is returned as a vector of Polygons
*/
vector<Polygon> Polygon::intersection(const Polygon& other) const
{
	vector<Polygon> retVect;

	/* compute the intersection of the polygons */
	vector<BoostPolygon> intersectingPolygons;
	boost::geometry::intersection(_poly, other._poly, intersectingPolygons);

	/* for each of the polygons we have we make a Polygon object and */
	/* place it in the return vector */
	for(size_t i = 0; i < intersectingPolygons.size(); i++)
		retVect.push_back(Polygon(intersectingPolygons[i]));

	/* return */
	return retVect;
}

/*
	friend std::ostream& operator<< (std::ostream& , const Polygon&);

	Stream overload
*/
std::ostream& operator<< (std::ostream& os, const Polygon& other)
{
	if(other.num_verts() > 1)
	{
		for(size_t i = 0; i < other.num_verts()-1; i++)
		{
			os << other.vert_x(i) << " " << other.vert_y(i) << "\n";
		}
		os << other.vert_x(other.num_verts()-1) << " " << other.vert_y(other.num_verts()-1);
	}
	return os;
}
