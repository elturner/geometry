#ifndef H_POINT_H
#define H_POINT_H

/*
	Point.h

	Holds the internal point type for the filter_pointcloud program
*/

class Point
{
public:

	/* The point attributes supported */
	double x, y, z;
	unsigned char r, g, b;
	double timestamp;
	int index;

	/* Holds if the point is valid or invalid in the current processing */
	/* This is used for telling if a point should be written to a dead pile */
	/* or good pile in output filtering */
	bool isValid;

	Point() :
		x(0), y(0), z(0), r(0), g(0), b(0), 
		timestamp(0), index(0), isValid(true) {};
};

#endif