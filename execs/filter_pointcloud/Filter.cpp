/*
	Filter.h

	This file contains all of the filtering objects that will be usable on a
	point.  They should inherit from the general filter type.
*/
#include "Filter.h"

/* includes */
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

#include <io/pointcloud/writer/PointCloudWriter.h>

#include "Point.h"

/* defines */
#define CMD_FILTERALL "ALL"
#define CMD_FILTERVALID "VALID"
#define CMD_FILTERINVALID "INVALID"

/* namespaces */
using namespace std;

/* Function implementations */

/*
*	PointCloudFilter
*/
bool PointCloudFilter::get_operates_on(const std::string& s,
	FILTER_OPERATES_ON& _operatesOn)
{
	if(s.compare(CMD_FILTERALL) == 0)
		_operatesOn = FILTER_ALL;
	else if(s.compare(CMD_FILTERVALID) == 0)
		_operatesOn = FILTER_VALID;
	else if(s.compare(CMD_FILTERINVALID) == 0)
		_operatesOn = FILTER_INVALID;
	else
	{
		_operatesOn = FILTER_ALL;
		return false;
	}
	return true;
}

/* 
* 	FilterFlipValidity
*/
FilterFlipValidity::FilterFlipValidity(FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
}
bool FilterFlipValidity::apply(Point& p) 
{ 
	switch(_operatesOn)
	{
		case FILTER_ALL:
			p.isValid = !p.isValid; 
			break;
		case FILTER_VALID:
			if(p.isValid)
				p.isValid = !p.isValid; 
			break;
		case FILTER_INVALID:
			if(!p.isValid)
				p.isValid = !p.isValid; 
			break;
	}
	return true; 
}
PointCloudFilter::FILTER_TYPE FilterFlipValidity::type() const
{
	return PointCloudFilter::FILTER_FLIPVALIDITY;
}

/*
*	FilterRotate
*/
FilterRotate::FilterRotate(double roll, double pitch, double yaw,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	const double cr = cos(roll);
	const double sr = sin(roll);
	const double cp = cos(pitch);
	const double sp = sin(pitch);
	const double cy = cos(yaw);
	const double sy = sin(yaw);
	_R[0] = cy*cp;
	_R[1] = cy*sp*sr-sy*cr;
	_R[2] = cy*cr*sp+sy*sr;
	_R[3] = cp*sy;
	_R[4] = sy*sp*sr+cy*cr;
	_R[5] = sy*cr*sp-cy*sr;
	_R[6] = -sp;
	_R[7] = cp*sr;
	_R[8] = cp*cr;
}
FilterRotate::FilterRotate(double w, double x, double y, double z,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_R[0] = w*w + x*x - y*y - z*z;
	_R[1] = 2*(x*y + w*z);
	_R[2] = 2*(x*z - w*y);
	_R[3] = 2*(x*y - w*z);
	_R[4] = w*w - x*x + y*y - z*z;
	_R[5] = 2*(y*z + w*x);
	_R[6] = 2*(x*z + w*y);
	_R[7] = 2*(y*z - w*x);
	_R[8] = w*w - x*x - y*y + z*z;
}
FilterRotate::FilterRotate(double r00, double r01, double r02,
				 double r10, double r11, double r12,
				 double r20, double r21, double r22,
				 FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_R[0] = r00; _R[1] = r01; _R[2] = r02;
	_R[3] = r10; _R[4] = r11; _R[5] = r12;
	_R[6] = r20; _R[7] = r21; _R[8] = r22;
}
bool FilterRotate::apply(Point& p) 
{
	switch(_operatesOn)
	{
		case FILTER_ALL:
			_x = p.x*_R[0] + p.y*_R[1] + p.z*_R[2];
			_y = p.x*_R[3] + p.y*_R[4] + p.z*_R[5];
			_z = p.x*_R[6] + p.y*_R[7] + p.z*_R[8];
			p.x = _x; p.y = _y; p.z = _z;
			break;
		case FILTER_VALID:
			if(p.isValid)
			{
				_x = p.x*_R[0] + p.y*_R[1] + p.z*_R[2];
				_y = p.x*_R[3] + p.y*_R[4] + p.z*_R[5];
				_z = p.x*_R[6] + p.y*_R[7] + p.z*_R[8];
				p.x = _x; p.y = _y; p.z = _z;
			}
			break;
		case FILTER_INVALID:
			if(!p.isValid)
			{
				_x = p.x*_R[0] + p.y*_R[1] + p.z*_R[2];
				_y = p.x*_R[3] + p.y*_R[4] + p.z*_R[5];
				_z = p.x*_R[6] + p.y*_R[7] + p.z*_R[8];
				p.x = _x; p.y = _y; p.z = _z;
			}
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterRotate::type() const
{
	return PointCloudFilter::FILTER_ROTATE;
}

/*
*	FilterScale
*/
FilterScale::FilterScale(double s, FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_S[0] = _S[1] = _S[2] = s;
}
FilterScale::FilterScale(double sx, double sy, double sz,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_S[0] = sx; _S[1] = sy; _S[2] = sz;
}
bool FilterScale::apply(Point& p)
{
	switch(_operatesOn)
	{
		case FILTER_ALL:
			p.x *= _S[0];
			p.y *= _S[1];
			p.z *= _S[2];
			break;
		case FILTER_VALID:
			if(p.isValid)
			{
				p.x *= _S[0];
				p.y *= _S[1];
				p.z *= _S[2];
			}
			break;
		case FILTER_INVALID:
			if(!p.isValid)
			{
				p.x *= _S[0];
				p.y *= _S[1];
				p.z *= _S[2];
			}
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterScale::type() const 
{
	return PointCloudFilter::FILTER_SCALE; 
}

/*
*	FilterTranslate
*/
FilterTranslate::FilterTranslate(double offset, 
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_T[0] = _T[1] = _T[2] = offset;
}
FilterTranslate::FilterTranslate(double offset_x, double offset_y, 
	double offset_z, FILTER_OPERATES_ON operatesOn)
{	
	_operatesOn = operatesOn;
	_T[0] = offset_x;
	_T[1] = offset_y;
	_T[2] = offset_z;
}
bool FilterTranslate::apply(Point& p)
{
	switch(_operatesOn)
	{
		case FILTER_ALL:
			p.x += _T[0];
			p.y += _T[1];
			p.z += _T[2];
			break;
		case FILTER_VALID:
			if(p.isValid)
			{
				p.x += _T[0];
				p.y += _T[1];
				p.z += _T[2];
			}
			break;
		case FILTER_INVALID:
			if(!p.isValid)
			{
				p.x += _T[0];
				p.y += _T[1];
				p.z += _T[2];
			}
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterTranslate::type() const
{
	return PointCloudFilter::FILTER_TRANSLATE;
}

/*
*	FilterRecolor
*/
FilterRecolor::FilterRecolor(unsigned char graylevel,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_c[0] = _c[1] = _c[2] = graylevel;
}
FilterRecolor::FilterRecolor(unsigned char r, 
	unsigned char g,
	unsigned char b,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_c[0] = r; _c[1] = g; _c[2] = b;
}
bool FilterRecolor::apply(Point& p)
{
	switch(_operatesOn)
	{
		case FILTER_ALL:
			p.r = _c[0];
			p.g = _c[1];
			p.b = _c[2];
			break;
		case FILTER_VALID:
			if(p.isValid)
			{
				p.r = _c[0];
				p.g = _c[1];
				p.b = _c[2];
			}
			break;
		case FILTER_INVALID:
			if(!p.isValid)
			{
				p.r = _c[0];
				p.g = _c[1];
				p.b = _c[2];
			}
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterRecolor::type() const
{
	return PointCloudFilter::FILTER_RECOLOR;
}

/*
*	FilterDecimate
*/
FilterDecimate::FilterDecimate(size_t decimationRate,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_decimationRate = decimationRate > 0 ? decimationRate : 1;
	_currentIndex = 0;
}
bool FilterDecimate::apply(Point& p)
{
	switch(_operatesOn)
	{
		case FILTER_ALL:

			/* check if we are dropping this point */
			if(_currentIndex % _decimationRate)
			{
				_currentIndex++;
				p.isValid = false;
			}
			/* if keep this point then just do nothing */
			else
			{
				_currentIndex = 1;
			}
			break;
		case FILTER_VALID:
			
			/* only apply logic to valid points */
			if(p.isValid)
			{
				/* check if we are dropping this point */
				if(_currentIndex % _decimationRate)
				{
					_currentIndex++;
					p.isValid = false;
				}
				/* if keep this point then just do nothing */
				else
				{
					_currentIndex = 1;
				}
			break;
			}
			break;
		case FILTER_INVALID:
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterDecimate::type() const
{
	return PointCloudFilter::FILTER_DECIMATE;
}

/*
*	FilterKill
*/
FilterKill::FilterKill(FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
}
bool FilterKill::apply(Point& p)
{	
	switch(_operatesOn)
	{
		case FILTER_ALL:
			return false;
		case FILTER_VALID:
			if(p.isValid)
				return false;
			break;
		case FILTER_INVALID:
			if(!p.isValid)
				return false;
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterKill::type() const
{
	return PointCloudFilter::FILTER_KILL;
}

/*
*	FilterPartitionPlane
*/
FilterPartitionPlane::FilterPartitionPlane(double nx, double ny, double nz,
	double px, double py, double pz,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_normal[0] = nx;
	_normal[1] = ny;
	_normal[2] = nz;
	_pointOnPlane[0] = px;
	_pointOnPlane[1] = py;
	_pointOnPlane[2] = pz;
}
bool FilterPartitionPlane::apply(Point& p)
{
	if(!p.isValid)
		return true;
	const double d = _normal[0]*(p.x-_pointOnPlane[0]) +
		_normal[1]*(p.y-_pointOnPlane[1]) +
		_normal[2]*(p.z-_pointOnPlane[2]);
	p.isValid = (d > 0);
	return true;
}
PointCloudFilter::FILTER_TYPE FilterPartitionPlane::type() const
{
	return PointCloudFilter::FILTER_PARTITION;
}

/*
*	FilterPartitionRadius
*/
FilterPartitionRadius::FilterPartitionRadius(double px, double py, double pz, 
	double radius,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_point[0] = px; _point[1] = py; _point[2] = pz;
	_radiusSquared = radius*radius;
}
bool FilterPartitionRadius::apply(Point& p)
{
	if(!p.isValid)
		return true;
	const double d = (p.x-_point[0])*(p.x-_point[0]) +
		(p.y-_point[1])*(p.y-_point[1]) +
		(p.z-_point[2])*(p.z-_point[2]);
	if(d > _radiusSquared)
		p.isValid = false;
	return true;
}
PointCloudFilter::FILTER_TYPE FilterPartitionRadius::type() const
{
	return PointCloudFilter::FILTER_PARTITION;
}

/*
*	FilterPartitionCylinder
*/
FilterPartitionCylinder::FilterPartitionCylinder(
	double px, double py, double pz,
	double dx, double dy, double dz,
	double radius,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_pointOnLine[0] = px;
	_pointOnLine[1] = py;
	_pointOnLine[2] = pz;
	_direction[0] = dx;
	_direction[1] = dy;
	_direction[2] = dz;
	_radiusSquared = radius*radius;

}
bool FilterPartitionCylinder::apply(Point& p)
{
	if(!p.isValid)
		return true;
	const double d[3] = {p.x-_pointOnLine[0],
		p.y-_pointOnLine[1],
		p.z-_pointOnLine[2]};
	const double dp = d[0]*_direction[0] + 
		d[1]*_direction[1] + 
		d[2]*_direction[2];
	const double e[3] = {d[0]-dp*_direction[0],
		d[1]-dp*_direction[1],
		d[2]-dp*_direction[2]};
	const double r = e[0]*e[0]+e[1]*e[1]+e[2]*e[2];
	if(r > _radiusSquared)
		p.isValid = false;
	return true;
}
PointCloudFilter::FILTER_TYPE FilterPartitionCylinder::type() const
{
	return PointCloudFilter::FILTER_PARTITION;
}

/*
*	Filter PartitionAABB
*/
FilterPartitionAABB::FilterPartitionAABB(double minX, double maxX,
	double minY, double maxY,
	double minZ, double maxZ,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_xlims[0] = minX; _xlims[1] = maxX;
	_ylims[0] = minY; _ylims[1] = maxY;
	_zlims[0] = minZ; _zlims[1] = maxZ;
}
bool FilterPartitionAABB::apply(Point& p)
{
	if(!p.isValid)
		return true;
	
	/* check x */
	if(p.x < _xlims[0])
		p.isValid = false;
	else if(p.x > _xlims[1])
		p.isValid = false;

	/* check y */
	else if(p.y < _ylims[0])
		p.isValid = false;
	else if(p.y > _ylims[1])
		p.isValid = false;

	/* check z */
	else if(p.z < _zlims[0])
		p.isValid = false;
	else if(p.z > _zlims[1])
		p.isValid = false;

	return true;
}
PointCloudFilter::FILTER_TYPE FilterPartitionAABB::type() const
{
	return PointCloudFilter::FILTER_PARTITION;
}

/*
*	FilterPrintStats
*/
FilterPrintStats::FilterPrintStats(std::ostream& os,
	const std::string& description,
	FILTER_OPERATES_ON operatesOn)
{
	_description = description;
	_operatesOn = operatesOn;
	_numPoints = 0;
	_minX = _minY = _minZ = 1e30;
	_maxX = _maxY = _maxZ = -1e30;
	_outStream = &os;
}
FilterPrintStats::~FilterPrintStats()
{
	string pType;
	switch(_operatesOn)
	{
		case FILTER_ALL:
			pType = "All";
			break;
		case FILTER_VALID:
			pType = "Valid";
			break;
		case FILTER_INVALID:
			pType = "Invalid";
			break;
	}
	*_outStream << "\n";
	if(!_description.empty())
	{
		*_outStream << "Stats for " << _description << "\n";
	}
	else
	{
		*_outStream << "Stats for " << pType << " points" << "\n";
	}
	*_outStream << "Number of Points : " << _numPoints << "\n"
			  << "Bounding Box: " << "\n"
			  << "\tx : [" << _minX << ", " << _maxX << "]\n"
			  << "\ty : [" << _minY << ", " << _maxY << "]\n"
			  << "\tz : [" << _minZ << ", " << _maxZ << "]\n" << endl;
}
bool FilterPrintStats::apply(Point& p)
{
	switch(_operatesOn)
	{
		case FILTER_ALL:
			_numPoints++;
			if(p.x > _maxX)
				_maxX = p.x;
			if(p.x < _minX)
				_minX = p.x;
			if(p.y > _maxY)
				_maxY = p.y;
			if(p.y < _minY)
				_minY = p.y;
			if(p.z > _maxZ)
				_maxZ = p.z;
			if(p.z < _minZ)
				_minZ = p.z;
			break;
		case FILTER_VALID:
			if(p.isValid)
			{
				_numPoints++;
				if(p.x > _maxX)
					_maxX = p.x;
				if(p.x < _minX)
					_minX = p.x;
				if(p.y > _maxY)
					_maxY = p.y;
				if(p.y < _minY)
					_minY = p.y;
				if(p.z > _maxZ)
					_maxZ = p.z;
				if(p.z < _minZ)
					_minZ = p.z;
				break;
			}
			break;
		case FILTER_INVALID:
			if(!p.isValid)
			{
				_numPoints++;
				if(p.x > _maxX)
					_maxX = p.x;
				if(p.x < _minX)
					_minX = p.x;
				if(p.y > _maxY)
					_maxY = p.y;
				if(p.y < _minY)
					_minY = p.y;
				if(p.z > _maxZ)
					_maxZ = p.z;
				if(p.z < _minZ)
					_minZ = p.z;
				break;
			}
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterPrintStats::type() const
{
	return PointCloudFilter::FILTER_PRINTSTATS;
}

/*
*	FilterOutputToFile
*/
FilterOutputToFile::FilterOutputToFile(const std::string& output_file,
	FILTER_OPERATES_ON operatesOn)
{
	_operatesOn = operatesOn;
	_writer = PointCloudWriter::create(output_file);
	if(!_writer.open(output_file))
		throw std::runtime_error("Unable to create output file : " + 
			output_file);
}
bool FilterOutputToFile::apply(Point& p)
{
	switch(_operatesOn)
	{
		case FILTER_ALL:
			_writer.write_point(p.x, p.y, p.z, 
				p.r, p.g, p.b, 
				p.index, p.timestamp);
			break;
		case FILTER_VALID:
			if(p.isValid)
				_writer.write_point(p.x, p.y, p.z, 
					p.r, p.g, p.b, 
					p.index, p.timestamp);
			break;
		case FILTER_INVALID:
			if(!p.isValid)
				_writer.write_point(p.x, p.y, p.z, 
					p.r, p.g, p.b, 
					p.index, p.timestamp);
			break;
	}
	return true;
}
PointCloudFilter::FILTER_TYPE FilterOutputToFile::type() const
{
	return PointCloudFilter::FILTER_OUTPUT;
}


