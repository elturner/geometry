#ifndef H_POINT2D_H
#define H_POINT2D_H

/*
	Point.h

	This class is used hold point objects of any number of dimensions
*/

#include <stdlib.h>
#include <ostream>
#include <cmath>

/* the actual point class */
class Point2D
{

	// The underlying point data
	double _data[2];

public:

	/*
		Point2D();
		Point2D(double x, double y);
		Point2D(double* data);

		Constructors.  The point can either be constructed from a pair of 
		doubles or a double array.  Defaults to the origin point.
	*/
	Point2D();
	Point2D(double x, double y);
	Point2D(double* data);

	/* data access members for convenience */
	inline double& x() 
	{
		return _data[0];
	};
	const inline double& x() const 
	{
		return _data[0];
	};
	inline double& y() 
	{
		return _data[1];
	};
	const inline double& y() const 
	{
		return _data[1];
	};

	/* overloaded operators */
	
	// Array Indexing
	inline double& operator[](size_t idx) 
	{
		return _data[idx];
	};
    inline const double& operator[](size_t idx) const 
    {
    	return _data[idx];
    };

    // Stream Overload 
    friend std::ostream& operator<< (std::ostream& , const Point2D&);

    /* geometry functions */

    /*
		double sq_dist_to(const Point2D& other) const;

		Returns the square L2 distance between this and other.
    */
	inline double sq_dist_to(const Point2D& other) const
	{
		return ((other.x()-this->x())*(other.x()-this->x()) +
			(other.y()-this->y())*(other.y()-this->y()));
	};

	/*
		double inf_dist_to(const Point2D& other) const;

		Returns the L-infinity norm to the other point
	*/
	inline double inf_dist_to(const Point2D& other) const
	{
		const double xDist = abs(this->x() - other.x());
		const double yDist = abs(this->y() - other.y());
		return (xDist > yDist ? xDist : yDist);
	};

};



#endif