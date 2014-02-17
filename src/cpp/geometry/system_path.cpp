#include "system_path.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <new>
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <config/backpackConfig.h>
#include <config/sensorProp.h>
#include <config/imuProp.h>
#include <config/laserProp.h>
#include <config/cameraProp.h>
#include <config/tofProp.h>
#include <geometry/transform.h>
#include <util/error_codes.h>

/* using both standard and eigen namespaces */
using namespace std;
using namespace Eigen;

/* the following macros are used to convert input */
#define DEG2RAD(x)  ( (x) * 3.1415926535897932384626433 / 180.0 )

/****** PATH FUNCTIONS ********/

system_path_t::system_path_t()
{
	this->pl = NULL;
	this->pl_size = 0;
}

system_path_t::~system_path_t()
{
	this->clear();
}

/*** i/o ***/

/* number of bytes used in file to define elements */
#define ZUPT_ELEMENT_SIZE 2

int system_path_t::readmad(const std::string& filename)
{
	ifstream infile;
	unsigned int i, num_zupts, num_poses;
	double t, x, y, z, roll, pitch, yaw;
	pose_t p;

	/* check arguments */
	if(filename.empty())
		return -1;

	/* open file for reading -- binary */
	infile.open(filename.c_str(), ifstream::binary);
	if(!(infile.is_open()))
		return -2; /* can't open file */
	if(infile.eof())
	{
		/* empty file */
		infile.close();
		return -3;
	}

	/* file assumed same endianness as system */
	
	/* read zupts header from file */
	infile.read((char*) (&num_zupts), sizeof(num_zupts)); 
	if(infile.fail())
	{
		infile.close();
		return -4;
	}

	/* skip zupt info */
	infile.seekg(num_zupts * ZUPT_ELEMENT_SIZE * sizeof(double), 
	             ios_base::cur);
	
	/* read number of poses */
	infile.read((char*) (&num_poses), sizeof(num_poses)); 
	if(infile.fail())
	{
		infile.close();
		return -5;
	}

	/* prepare the pose list */
	this->clear();
	this->pl = new pose_t[num_poses];
	this->pl_size = num_poses;

	/* read all poses */
	for(i = 0; i < num_poses && !(infile.eof()); i++)
	{
		/* read information for this pose */
		infile.read((char*) (&t),     sizeof(t)); 
		infile.read((char*) (&x),     sizeof(x)); 
		infile.read((char*) (&y),     sizeof(y)); 
		infile.read((char*) (&z),     sizeof(z)); 
		infile.read((char*) (&roll),  sizeof(roll)); 
		infile.read((char*) (&pitch), sizeof(pitch)); 
		infile.read((char*) (&yaw),   sizeof(yaw)); 

		/* check file state */
		if(infile.fail())
		{
			infile.close();
			return -6;
		}

		/* store translation vector */
		p.timestamp = t;
		p.T(0) = x; /* already in units of meters */
		p.T(1) = y; /* '' */
		p.T(2) = z; /* '' */

		/* convert the orientation information that was just
		 * read into radians in NED coordinates. */
		roll = DEG2RAD(roll);
		pitch = DEG2RAD(pitch);
		yaw = DEG2RAD(yaw);

		/* compute rotation matrix */
		p.compute_transform_NED(roll, pitch, yaw);

		/* check that poses are given in order */
		if(i > 0 && pl[i-1].timestamp > p.timestamp)
		{
			infile.close();
			return -7;
		}
		
		/* store this info */
		pl[i] = p;
	}

	/* cleanup */
	infile.close();

	/* final pass-over of poses to compute additional values,
	 * such as linear and rotational velocity */
	for(i = 1; i < num_poses; i++)
		this->pl[i-1].compute_velocity(this->pl[i]);

	/* success */
	return 0;
}
		
int system_path_t::parse_hardware_config(const std::string& xml)
{
	vector<imuProp> imus;
	vector<laserProp> lasers;
	vector<cameraProp> cameras;
	vector<tofProp> tofs;
	backpackConfig conf;
	transform_t* t;
	size_t i, n;
	int ret;

	/* read the config file */
	if(!conf.read_config_file(xml))
		return -1; /* unable to read file */

	/* store imu transforms */
	conf.get_props(imus, true);
	n = imus.size();
	for(i = 0; i < n; i++)
	{
		/* generate a transform for each sensor */
		imus[i].toRadianMeters();
		t = new transform_t();
		ret = t->set(imus[i].tToCommon, imus[i].rToCommon);
		if(ret)
		{
			/* clean up and return error */
			delete t;
			return PROPEGATE_ERROR(-2, ret);
		}

		/* add this transform to our map */
		this->transform_map.insert(make_pair<string, transform_t*>(
				imus[i].name, t));
	}

	/* store laser transforms */
	conf.get_props(lasers, true);
	n = lasers.size();
	for(i = 0; i < n; i++)
	{
		/* generate transform for each sensor */
		lasers[i].toRadianMeters();
		t = new transform_t();
		ret = t->set(lasers[i].tToCommon, lasers[i].rToCommon);
		if(ret)
		{
			/* clean up and return error */
			delete t;
			return PROPEGATE_ERROR(-3, ret);
		}

		/* add this transform to our map */
		this->transform_map.insert(make_pair<string, transform_t*>(
				lasers[i].name, t));
	}

	/* store camera transforms */
	conf.get_props(cameras, true);
	n = cameras.size();
	for(i = 0; i < n; i++)
	{
		/* generate transform for each sensor */
		cameras[i].toRadianMeters();
		t = new transform_t();
		ret = t->set(cameras[i].tToCommon, cameras[i].rToCommon);
		if(ret)
		{
			/* clean up and return error */
			delete t;
			return PROPEGATE_ERROR(-4, ret);
		}

		/* add this transform to our map */
		this->transform_map.insert(make_pair<string, transform_t*>(
				cameras[i].name, t));
	}

	/* store d-imager transforms */
	conf.get_props(tofs, true);
	n = tofs.size();
	for(i = 0; i < n; i++)
	{
		/* generate transform for each sensor */
		tofs[i].toRadianMeters();
		t = new transform_t();
		ret = t->set(tofs[i].tToCommon, tofs[i].rToCommon);
		if(ret)
		{
			/* clean up and return error */
			delete t;
			return PROPEGATE_ERROR(-5, ret);
		}

		/* add this transform to our map */
		this->transform_map.insert(make_pair<string, transform_t*>(
				tofs[i].name, t));
	}

	/* success */
	return 0;
}
		
void system_path_t::clear()
{
	map<string, transform_t*>::iterator it;

	/* free any allocated memory for poses */
	if(this->pl != NULL)
	{
		delete[] this->pl;
		this->pl = NULL;
	}

	/* set number of poses */
	this->pl_size = 0;

	/* free allocated memory for transforms */
	for(it = this->transform_map.begin();
			it != this->transform_map.end(); it++)
	{
		/* free dynamic memory pointed to in this element */
		delete (it->second);
	}
	this->transform_map.clear();
}

/*** accessors ***/

int system_path_t::compute_pose_at(pose_t& p, double t) const
{
	int i;
	double weight;

	/* find closest pose to t that is before t */
	i = this->closest_index(t);
	if(i < 0)
		return PROPEGATE_ERROR(-1, i);

	/* if the closest pose is the very first pose, just return
	 * the first pose */
	if(i == 0)
	{
		/* copy first pose */
		p = this->pl[0];
		return 0;
	}

	/* if the closest pose is the very last pose, just return
	 * the last pose */
	if(((unsigned int) i) == this->pl_size - 1)
	{
		/* copy last pose */
		p = this->pl[this->pl_size - 1];
		return 0;
	}

	/* use linear interpolation to construct new pose.  First,
	 * find weight of the desired time between the two closest
	 * poses. */
	weight = (t - this->pl[i].timestamp) 
	         / (this->pl[i+1].timestamp - this->pl[i].timestamp);

	/* populate the pose */
	p.timestamp = t;
	p.T = ((1 - weight) * this->pl[i].T)
	      + (weight * (this->pl[i+1].T));
	p.R = this->pl[i].R.slerp(weight, this->pl[i+1].R);

	/* success */
	return 0;
}
		
int system_path_t::compute_transform_for(transform_t& p, double t,
                                         const string& s) const
{
	map<string, transform_t*>::const_iterator it;	
	pose_t system_pose;
	transform_t system2world;
	int ret;

	/* check if valid sensor */
	it = this->transform_map.find(s);
	if(it == this->transform_map.end())
		return -1; /* not a valid sensor name */

	/* save the sensor's transform */
	p = *(it->second); /* sensor -> system */

	/* get the system pose at the specified time */
	ret = this->compute_pose_at(system_pose, t);
	if(ret)
		return PROPEGATE_ERROR(-2, ret); /* can't find pose */

	/* system -> world */
	system2world.T = system_pose.T;
	system2world.R = system_pose.R.toRotationMatrix();

	/* The system pose gives us:  system -> world
	 * we want:  sensor -> world */
	p.cat(system2world); /* sensor -> system + system -> world */

	/* success */
	return 0;
}

/*** helper functions ***/

int system_path_t::closest_index(double t) const
{
	unsigned int low, high, mid, last;
	double comp;

	/* check arguments */
	if(this->pl == NULL || this->pl_size == 0)
		return -1;

	/* check outer bounds */
	if(t < this->pl[0].timestamp)
		return 0;
	last = this->pl_size - 1;
	if(t > this->pl[last].timestamp)
		return last;

	/* assume poses are in order, and perform binary search */
	low = 0;
	high = this->pl_size;
	while(low <= high)
	{
		/* check middle of range */
		mid = (low + high)/2;
		comp = this->pl[mid].timestamp - t;

		/* subdivide */
		if(comp < 0)
			low = mid + 1;
		else if(comp > 0)
		{
			high = mid - 1;
			
			/* check if high is now less than t */
			if(mid != 0 && this->pl[high].timestamp < t)
			{
				low = high;
				break;
			}
		}
		else
			return mid; /* exactly the right time */
	}

	/* we know t falls between indices low and low+1 */
	return low;
}

/*** POSE FUNCTIONS ***/
	
pose_t::pose_t() {}

pose_t::~pose_t() {}

double pose_t::dist_sq(const pose_t& other) const
{
	double x, y, z;

	/* compute distance */
	x = this->T(0) - other.T(0);
	y = this->T(1) - other.T(1);
	z = this->T(2) - other.T(2);
	return x*x + y*y + z*z;
}
		
void pose_t::print() const
{
	Matrix<double,3,3> r = this->R.toRotationMatrix();

	cout << this->timestamp << " "
	     << this->T(0) << " " << this->T(1) << " " << this->T(2) << " "
	     << r(0,0) << " " << r(0,1) << " " << r(0,2) << " "
	     << r(1,0) << " " << r(1,1) << " " << r(1,2) << " "
	     << r(2,0) << " " << r(2,1) << " " << r(2,2) << " "
	     << endl;
}

void pose_t::compute_transform_NED(double roll, double pitch, double yaw)
{
	double cr, sr, cp, sp, cy, sy;
	double A11, A12, A13, A21, A22, A23, A31, A32, A33;
	double q1, q2, q3, q4;
	double coef;

	/* compute trig values */
	cr = cos(roll);
	sr = sin(roll);
	cp = cos(pitch);
	sp = sin(pitch);
	cy = cos(yaw);
	sy = sin(yaw);

	/* Compute elements for rotation matrix
	 *
	 * this quaternion represents the following rotation matrix:
	 *
	 *	NED2ENU * Rz * Ry * Rx = 
	 *
	 *	cp*sy   cr*cy + sp*sr*sy        cr*sp*sy - cy*sr;
	 *	cp*cy   cy*sp*sr - cr*sy        cr*cy*sp + sr*sy;
	 *	sp      -cp*sr                  -cp*cr;
	 *
	 * where:
	 *
	 * 	cr = cos(roll)
	 * 	sr = sin(roll)
	 * 	cp = cos(pitch)
	 * 	sp = sin(pitch)
	 * 	cy = cos(yaw)
	 * 	sy = sin(yaw)
	 */
	A11 = cp*sy; A12 = cr*cy+sp*sr*sy; A13 = cr*sp*sy-cy*sr;
	A21 = cp*cy; A22 = cy*sp*sr-cr*sy; A23 = cr*cy*sp+sr*sy;
	A31 = sp;    A32 = -cp*sr;         A33 = -cp*cr;

	/* compute elements for quaternion based on rotation matrix
	 *
	 * see:
	 *	http://en.wikipedia.org/wiki/
	 *	Rotation_formalisms_in_three_dimensions
	 *	#Conversion_formulae_between_formalisms
	 */
	q4 = 0.5 * sqrt(1 + A11 + A22 + A33);
	q1 = 0.5 * sqrt(1 + A11 - A22 - A33);
	if(abs(q1) > abs(q4))
	{
		/* compute other values based on q1, to avoid
		 * dividing by a small value */
		coef = 0.25 / q1;
		q2 = coef * (A12 + A21);
		q3 = coef * (A13 + A31);
		q4 = coef * (A32 - A23);
	}
	else
	{
		/* compute other values based on q4, to avoid
		 * dividing by a small value */
		coef = 0.25 / q4;
		q1 = coef * (A32 - A23);
		q2 = coef * (A13 - A31);
		q3 = coef * (A21 - A12);
	}

	/* populate quaternion 
	 *
	 * See:
	 * 	http://en.wikipedia.org/wiki/
	 * 	Conversion_between_quaternions_and_Euler_angles
	 *
	 * Note that wikipedia assumes that the real component is the
	 * last value, whereas Eigen assumes the real value is the first
	 * component.  Hence why q4 is listed first below.
	 */
	this->R = Quaternion<double>(q4, q1, q2, q3);
}
		
void pose_t::compute_velocity(const pose_t& next_pose)
{
	Matrix<double, 3, 3> inc;
	double theta, dt;

	/* get the elapsed time between poses */
	dt = next_pose.timestamp - this->timestamp;

	/* Compute the linear velocity by finding the incremental
	 * translation:
	 *
	 * v = dT / dt
	 *
	 * This gives the velocity in world coordinates in units of
	 * meters per second.
	 */
	this->v = (next_pose.T - this->T) / dt;

	/* The rotational velocity is derived from the incremental
	 * rotation matrix with the equations found here:
	 *
	 * http://en.wikipedia.org/wiki/Axis%E2%80%93angle_representation
	 * #Log_map_from_SO.283.29_to_so.283.29
	 *
	 * The magnitude of the rotation from a matrix is:
	 *
	 * theta = arccos( (trace(R) - 1) / 2 )
	 * w = (1 / (2 * sin(theta) )) * [R(3,2) - R(2,3) ;
	 *                                R(1,3) - R(3,1) ;
	 *                                R(2,1) - R(1,2) ]
	 *
	 * This computation gives the rotational velocity in the system
	 * coordinate frame for the current pose, NOT the world coordinate
	 * frame.
	 */
	inc = this->R.inverse().toRotationMatrix()
			* next_pose.R.toRotationMatrix();
	theta = acos((inc.trace() - 1.0) / 2.0);
	
	/* get unit vector for rotational velocity */
	this->w(0) = inc(3,2) - inc(2,3);
	this->w(1) = inc(1,3) - inc(3,1);
	this->w(2) = inc(2,1) - inc(1,2);
	this->w /= (0.5/sin(theta));

	/* compute the magnitude of this vector to be the rotational
	 * velocity in units of radians per second */
	this->w *= theta / dt;
}
