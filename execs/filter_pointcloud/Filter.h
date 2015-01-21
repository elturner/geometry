#ifndef H_FILTER_H
#define H_FILTER_H

/*
	Filter.h

	This file contains all of the filtering objects that will be usable on a
	point.  They should inherit from the general filter type.
*/

/* includes */
#include <string>
#include <fstream>
#include <vector>
#include <ostream>
#include <iostream>
#include <io/pointcloud/writer/PointCloudWriter.h>

#include "Point.h"

/* abstract Filter class */
class PointCloudFilter
{
public:

	/*
	*	Enumeration that holds the various "types" of filters.
	*/
	enum FILTER_TYPE
	{
		FILTER_FLIPVALIDITY,
		FILTER_ROTATE,
		FILTER_SCALE,
		FILTER_TRANSLATE,
		FILTER_RECOLOR,
		FILTER_DECIMATE,
		FILTER_KILL,
		FILTER_PARTITION,
		FILTER_PRINTSTATS,
		FILTER_OUTPUT
	};

	/*
	*	Enumeration that specifies if they operate on valid, invalid,
	*	or all
	*/
	enum FILTER_OPERATES_ON
	{
		FILTER_ALL,
		FILTER_VALID,
		FILTER_INVALID
	};

protected:

	/* enum that sets what the filter operates on */
	FILTER_OPERATES_ON _operatesOn;

public:

	/*
	*	Getter setter for the _operatesOn flag
	*/
	inline const FILTER_OPERATES_ON& operates_on() const
		{return _operatesOn; };
	inline FILTER_OPERATES_ON& operates_on()
		{return _operatesOn; };

	/*
	*	get what it operates on from string.  if it does not match any of 
	*	the acceptable ones it defaults to ALL and returns false
	*/
	static bool get_operates_on(const std::string& s,
		FILTER_OPERATES_ON& operatesOn);

	/*
	*	The filter function. The is the abstract interface that defines
	*	the filter action. 
	*
	*	This function returns a boolean value that indicates if a point is 
	*	still alive for processing after a point has been passed through it.
	*	This allows for computation to be short circuited during evaluation
	*	of the filter chain. The most common use case of this is after a 
	*	point passes though an output filter it should not be processed any
	*	more
	*/
	virtual bool apply(Point& p) =0;

	/*
	*	Function that returns the type of filter
	*/
	virtual FILTER_TYPE type() const =0;

	/*
	*	Virtual Destructor to allow for inherited classes to call their
	*	destructors 
	*/
	virtual ~PointCloudFilter() {};

};

/*
*	Concrete implementations of filters are declared here
*/

/*
*	FilterFilpValidity
*
*	This filter is responsible for flipping the valid state flag
*/
class FilterFlipValidity : public PointCloudFilter
{	
	public:
		/* Constructor and required funtions */
		FilterFlipValidity(FILTER_OPERATES_ON operatesOn = FILTER_ALL);
		bool apply(Point& p);
		FILTER_TYPE type() const;
};

/*
*	FilterRotate
*
*	This filter is responsible for rotating a point using a rigid
*	body rotation
*/
class FilterRotate : public PointCloudFilter
{
	/* holds the rotation matrix in ROW MAJOR ordering */
	double _R[9];
	double _x,_y,_z;
public:
	/* Constructor and required funtions */
	FilterRotate(double roll, double pitch, double yaw,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	FilterRotate(double w, double x, double y, double z,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	FilterRotate(double r00, double r01, double r02,
				 double r10, double r11, double r12,
				 double r20, double r21, double r22,
				 FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterScale
*
*	This filter is responsible for scaling the point on a per axis or global
*	basis
*/
class FilterScale : public PointCloudFilter
{
	/* Axis scaling */
	double _S[3];
public:
	/* Constructors and required functions */
	FilterScale(double s,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	FilterScale(double sx, double sy, double sz,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterTranslate
*
*	This filter is responsible for translating a point either using a 
*	single additive point 
*/
class FilterTranslate : public PointCloudFilter
{
	/* holds the per axis translation */
	double _T[3];
public:
	/* Constructors and required functions */
	FilterTranslate(double offset,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	FilterTranslate(double offset_x, double offset_y, double offset_z,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterRecolor
*
*	This filter is responsible for resetting the color of a point
*/
class FilterRecolor : public PointCloudFilter
{
	/* holds the new color */
	unsigned char _c[3];
public:
	/* Constructors and required functions */
	FilterRecolor(unsigned char graylevel = 0,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	FilterRecolor(unsigned char r, 
		unsigned char g,
		unsigned char b,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterDecimate
*
*	This filter is responsible for decimating the pointcloud by invalidating 
*	every non nth point
*/
class FilterDecimate : public PointCloudFilter
{
	/* holds the current index and decimation rate */
	size_t _decimationRate;
	size_t _currentIndex;
public:
	/* Constructors and required functions */
	FilterDecimate(size_t decimationRate,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterKill
*
*	This filter kills off a point 
*/
class FilterKill : public PointCloudFilter
{
public:
	/* Constructors and required functions */
	FilterKill(FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterPartitionPlane
*
*	This filter will partition a pointcloud based on splitting plane
*/
class FilterPartitionPlane : public PointCloudFilter
{
	/* holds the point on plane and normal */
	double _pointOnPlane[3];
	double _normal[3];
public:
	/* Constructors and required functions */
	FilterPartitionPlane(double nx, double ny, double nz,
		double px = 0, double py = 0, double pz = 0,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterParitionRadius
*
*	This filter will partition based on radius to point
*/
class FilterPartitionRadius : public PointCloudFilter
{
	/* holds the point and radius */
	double _point[3];
	double _radiusSquared;
public:
	/* Constructors and required functions */
	FilterPartitionRadius(double px, double py, double pz, double radius,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterPartitionCylinder
*
*	This filter partitions a pointcloud using an arbitrary infinite height 
*	cylinder
*/
class FilterPartitionCylinder : public PointCloudFilter
{
	/* holds the parameters of the cylinder */
	double _pointOnLine[3];
	double _direction[3];
	double _radiusSquared;
public:
	/* Constructors and required functions */
	FilterPartitionCylinder(double px, double py, double pz,
		double dx, double dy, double dz,
		double radius,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterPartitionAABB
*
*	The filter partitions a pointcloud using an axis aligned bounding box
*/
class FilterPartitionAABB : public PointCloudFilter
{
	/* holds the bounds of the aabb */
	double _xlims[2];
	double _ylims[2];
	double _zlims[2];
public:
	/* Constructors and required functions */
	FilterPartitionAABB(double minX, double maxX,
		double minY, double maxY,
		double minZ, double maxZ,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterPrintStats
*
*	This filter does nothing to the points but collects information about
*	the points such as bounding box and number of points
*/
class FilterPrintStats : public PointCloudFilter
{
	/* holds the stat information */
	size_t _numPoints;
	double _minX, _maxX;
	double _minY, _maxY;
	double _minZ, _maxZ;
	std::string _description;
	std::ostream * _outStream;
public:
	FilterPrintStats(std::ostream& os = std::cout,
		const std::string& description = "",
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	~FilterPrintStats();
	bool apply(Point& p);
	FILTER_TYPE type() const;
};

/*
*	FilterOutputToFile
* 
* 	This filter is responsible for outputing data to file
*/
class FilterOutputToFile : public PointCloudFilter
{
	/* holds the pointcloud writer object */
	PointCloudWriter _writer;
public:
	FilterOutputToFile(const std::string& output_file,
		FILTER_OPERATES_ON operatesOn = FILTER_ALL);
	bool apply(Point& p);
	FILTER_TYPE type() const;
};




#endif