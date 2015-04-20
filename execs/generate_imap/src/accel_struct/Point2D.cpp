/*
	Point.cpp

	This class is used hold point objects of any number of dimensions
*/

#include "Point2D.h"

/* function definitions */

/*
	Point2D();
	Point2D(double x, double y);
	Point2D(double* data);

	Constructors.  The point can either be constructed from a pair of 
	doubles or a double array.  Defaults to the origin point.
*/
Point2D::Point2D() 
{
	_data[0] = 0;
	_data[1] = 0;
}
Point2D::Point2D(double x, double y) 
{
	_data[0] = x;
	_data[1] = y;
}
Point2D::Point2D(double* data)
{
	_data[0] = data[0];
	_data[1] = data[1];
}

/* Stream Operations */
std::ostream& operator<<(std::ostream& os, const Point2D& obj)
{
  os << obj.x() << ", " << obj.y(); 
  return os;
};
