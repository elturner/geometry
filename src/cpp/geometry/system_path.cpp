#include "system_path.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <new>
#include <cmath>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <io/carve/noisypath_io.h>
#include <config/backpackConfig.h>
#include <config/sensorProp.h>
#include <config/flirProp.h>
#include <config/imuProp.h>
#include <config/laserProp.h>
#include <config/cameraProp.h>
#include <config/tofProp.h>
#include <geometry/transform.h>
#include <util/rotLib.h>
#include <util/range_list.h>
#include <util/error_codes.h>

/* using both standard and eigen namespaces */
using namespace std;
using namespace Eigen;

/* the following macros are used to convert input */
#define DEG2RAD(x)  ( (x) * 3.1415926535897932384626433 / 180.0 )
#define RAD2DEG(x)  ( (x) * 180.0 / 3.1415926535897932384626433 )

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

int system_path_t::read(const std::string& pathfile)
{
	string file_ext;
	size_t p;
	int ret;

	/* determine what type of file was given for the path trajectory */
	p = pathfile.find_last_of('.');
	if(p == string::npos)
		file_ext = "";
	else 
		file_ext = pathfile.substr(p+1);
	
	/* next, load the path file, based on its type */
	if(!file_ext.compare("mad"))
	{
		/* attempt to read the mad file */
		ret = this->readmad(pathfile);
		if(ret)
		{
			/* can't read input */
			ret = PROPEGATE_ERROR(-1, ret);
			cerr << "[system_path_t::read]\tERROR " << ret
			     << ": Unable to parse mad path file: "
			     << pathfile << endl;
			return ret;
		}
	}
	else if(!file_ext.compare("noisypath"))
	{
		/* attempt to parse the noisypath file */
		ret = this->readnoisypath(pathfile);
		if(ret)
		{
			/* can't read input */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[system_path_t::read]\tERROR " << ret
			     << ": Unable to parse noisy path file: "
			     << pathfile << endl;
			return ret;
		}
	}
	else
	{
		/* not a recognized format */
		ret = -3;
		cerr << "[system_path_t::read]\tERROR " << ret
		     << ": Invalid file format given for path: "
		     << pathfile << endl;
		return ret;
	}

	/* success */
	return 0;
}

/* number of bytes used in file to define elements */
#define ZUPT_ELEMENT_SIZE 2

int system_path_t::readmad(const std::string& filename)
{
	ifstream infile;
	unsigned int i, num_zupts, num_poses;
	double t, x, y, z, roll, pitch, yaw, zupt_beg, zupt_end;
	pose_t p;

	/* check arguments */
	if(filename.empty())
		return -1;

	/* open file for reading -- binary */
	infile.open(filename.c_str(), ifstream::binary);
	if(!(infile.is_open()))
	{
		cerr << "[system_path_t::readmad]\tUnable to open file: "
		     << filename << endl;
		return -2; /* can't open file */
	}
	if(infile.eof())
	{
		/* empty file */
		infile.close();
		return -3;
	}

	/* file assumed same endianness as system */
	this->clear();
	
	/* read zupts header from file */
	infile.read((char*) (&num_zupts), sizeof(num_zupts)); 
	if(infile.fail())
	{
		infile.close();
		this->clear();
		return -4;
	}

	/* iterate over the zupt info */
	for(i = 0; i < num_zupts; i++)
	{
		/* parse the beginning and ending times for this zupt */
		infile.read((char*) &zupt_beg, sizeof(zupt_beg));
		infile.read((char*) &zupt_end, sizeof(zupt_end));
		
		/* add this range to the timestamp blacklist */
		this->timestamp_blacklist.add(zupt_beg, zupt_end);
	}

	/* read number of poses */
	infile.read((char*) (&num_poses), sizeof(num_poses)); 
	if(infile.fail())
	{
		infile.close();
		this->clear();
		return -5;
	}
	
	/* prepare the pose list */
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
			this->clear();
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
			cerr << "[readmad]\tERROR! Poses out of order!" 
			     << endl
			     << "\ti = " << i << endl 
			     << "\ttime[" << (i-1) << "] = "
			     << pl[i-1].timestamp << endl
			     << "\ttime[" << i << "] = " << p.timestamp
			     << endl << endl;
			infile.close();
			this->clear();
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
		
int system_path_t::writemad(const std::string& filename) const
{
	vector<pair<double, double> > zupts;
	Matrix3d R, ENU2NED;
	Vector3d euler;
	ofstream outfile;
	unsigned int i, num_zupts, num_poses;
	double t, x, y, z, roll, pitch, yaw, zupt_beg, zupt_end;

	/* check arguments */
	if(filename.empty())
		return -1; /* can't open empty filename */

	/* attempt to open file for writing */
	outfile.open(filename.c_str(), ios_base::binary | ios_base::out);
	if(!(outfile.is_open()))
		return -2; /* can't open file */
	
	/*------------------*/
	/* export zupt info */
	/*------------------*/

	/* get the zupts for this path */
	this->timestamp_blacklist.get_ranges(zupts);
	num_zupts = zupts.size();

	/* export number of zupts */
	outfile.write((char*) (&num_zupts), sizeof(num_zupts));

	/* iterate over the zupt info */
	for(i = 0; i < num_zupts; i++)
	{
		/* get next zupt range */
		zupt_beg = zupts[i].first;
		zupt_end = zupts[i].second;

		/* write the beginning and ending times for this zupt */
		outfile.write((char*) &zupt_beg, sizeof(zupt_beg));
		outfile.write((char*) &zupt_end, sizeof(zupt_end));
	}
	
	/*------------------*/
	/* export pose info */
	/*------------------*/

	/* prepare matrix for converting rotations from ENU to NED,
	 * which is required when exporting to .mad files */
	ENU2NED << 0, 1, 0,
	           1, 0, 0,
		   0, 0,-1;

	/* export number of poses */
	num_poses = this->pl_size;
	outfile.write((char*) (&num_poses), sizeof(num_poses)); 

	/* iterate over poses, exporting to file */
	for(i = 0; i < num_poses; i++)
	{
		/* prepare info for export */
		t = this->pl[i].timestamp;
		x = this->pl[i].T(0);
		y = this->pl[i].T(1);
		z = this->pl[i].T(2);
		
		/* convert rotations from a quaternion to NED euler
		 * angles */
		R = ENU2NED * this->pl[i].R.toRotationMatrix(); 
		rotLib::rot2rpy(R, euler);
		roll  = RAD2DEG(euler(0));
		pitch = RAD2DEG(euler(1));
		yaw   = RAD2DEG(euler(2));
		
		/* read information for this pose */
		outfile.write((char*) (&t),     sizeof(t)); 
		outfile.write((char*) (&x),     sizeof(x)); 
		outfile.write((char*) (&y),     sizeof(y)); 
		outfile.write((char*) (&z),     sizeof(z)); 
		outfile.write((char*) (&roll),  sizeof(roll)); 
		outfile.write((char*) (&pitch), sizeof(pitch)); 
		outfile.write((char*) (&yaw),   sizeof(yaw)); 
	}

	/* clean up */
	outfile.close();
	return 0;
}
		
int system_path_t::readnoisypath(const std::string& filename)
{
	noisypath_io::reader_t infile;
	noisypath_io::pose_t p;
	vector<noisypath_io::zupt_t> zupts;
	unsigned int i, n;
	int ret;

	/* attempt to open file */
	ret = infile.open(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	
	/* clear any existing data */
	this->clear();
	
	/* incorporate zupts */
	infile.get_zupts(zupts);	
	n = zupts.size();
	for(i = 0; i < n; i++)
	{
		/* add this range to the timestamp blacklist */
		this->timestamp_blacklist.add(zupts[i].start_time,
		                              zupts[i].end_time);
	}
	
	/* prepare the pose list */
	n = infile.num_poses();
	this->pl = new pose_t[n];
	this->pl_size = n;
	
	/* iterate through poses in file */
	for(i = 0; i < n; i++)
	{
		/* get the next pose */
		ret = infile.read(p, i);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);

		/* store in this structure */
		this->pl[i].timestamp = p.timestamp;
		this->pl[i].T         = p.position.mean;
		this->pl[i].T_cov     = p.position.cov;
		this->pl[i].compute_transform_ENU(
				p.rotation.mean(0),
				p.rotation.mean(1),
				p.rotation.mean(2));
		this->pl[i].R_cov     = p.rotation.cov;
	}

	/* clean up */
	infile.close();
	
	/* final pass-over of poses to compute additional values,
	 * such as linear and rotational velocity */
	for(i = 1; i < n; i++)
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
	vector<flirProp> flirs;
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
		this->transform_map.insert(make_pair(imus[i].name, t));
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
		this->transform_map.insert(make_pair(lasers[i].name, t));
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
		this->transform_map.insert(make_pair(cameras[i].name, t));
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
		this->transform_map.insert(make_pair(tofs[i].name, t));
	}

	/* store the flir transforms */
	conf.get_props(flirs, true);
	n = flirs.size();
	for(i = 0; i < n; i++)
	{
		/* generate transform for each sensor */
		flirs[i].toRadianMeters();
		t = new transform_t();
		ret = t->set(flirs[i].tToCommon, flirs[i].rToCommon);
		if(ret)
		{
			/* clean up and return error */
			delete t;
			return PROPEGATE_ERROR(-6, ret);
		}

		/* add this transform to our map */
		this->transform_map.insert(make_pair(flirs[i].name, t));
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

	/* clear any timestamp blacklist information */
	this->timestamp_blacklist.clear();
}
		
int system_path_t::apply_transform(const Eigen::Quaternion<double>& R,
					const Eigen::Vector3d& T)
{
	unsigned int i;

	/* iterate over the poses */
	for(i = 0; i < this->pl_size; i++)
	{
		/* modify this pose */
		this->pl[i].T = (R.toRotationMatrix() 
				* this->pl[i].T) + T; 
		this->pl[i].T_cov = (R.toRotationMatrix() 
				* this->pl[i].T_cov);
		this->pl[i].R = R * this->pl[i].R;
		this->pl[i].R_cov = (R.toRotationMatrix()
				* this->pl[i].R_cov);
		this->pl[i].v = (R.toRotationMatrix()
				* this->pl[i].v);
		this->pl[i].w = (R.toRotationMatrix()
				* this->pl[i].w);
	}

	/* success */
	return 0;
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
	p.v = this->pl[i].v; /* velocity is assumed constant for edge */
	p.w = this->pl[i].w; /* ang. vel. also assumed piece-wise const. */
	
	/* uncertainty matices can also just be averaged */
	p.T_cov = ((1 - weight) * this->pl[i].T_cov)
		+ (weight * (this->pl[i+1].T_cov));
	p.R_cov = ((1 - weight) * this->pl[i].R_cov)
		+ (weight * (this->pl[i+1].R_cov));
	
	/* success */
	return 0;
}
		
int system_path_t::get_extrinsics_for(transform_t& t,
                                      const std::string& s) const
{
	map<string, transform_t*>::const_iterator it;	

	/* check if valid sensor */
	it = this->transform_map.find(s);
	if(it == this->transform_map.end())
		return -1; /* not a valid sensor name */

	/* save the sensor's transform */
	t = *(it->second); /* sensor -> system */
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
		
bool system_path_t::is_blacklisted(double ts) const
{
	int a, b;

	/* get the range of indices */
	a = b = this->closest_index(ts);
	b++;
	
	/* error-check */
	if(a < 0)
		return true; /* no poses have been read in */
	
	/* check if ts is out of bounds */
	if(ts < this->pl[0].timestamp 
			|| ts > this->pl[this->pl_size-1].timestamp)
		return true; /* out of bounds */

	/* check if any point in the range [a,b] intersects any
	 * zupts, because this full range will be used to interpolate
	 * values at this timestamp */
	range_t r(this->pl[a].timestamp, this->pl[b].timestamp);
	return this->timestamp_blacklist.intersects(r);
}
		
double system_path_t::rotational_speed_at(double ts) const
{
	int pose_ind;

	/* get the index of the pose to check */
	pose_ind = this->closest_index(ts);
	if(pose_ind < 0)
		return 0; /* no path defined, no rotation either */

	/* get the magnitude of w at this pose */
	return (this->pl[pose_ind].w.norm());
}
	
double system_path_t::starttime() const
{
	if(this->pl == NULL)
		return 0.0;
	return this->pl[0].timestamp;
}

double system_path_t::endtime() const
{
	if(this->pl == NULL)
		return 0.0;
	return this->pl[this->pl_size-1].timestamp;
}
		
double system_path_t::total_distance() const
{
	Vector3d inc;
	unsigned int i;
	double dist;

	/* verify this path */
	dist = 0;
	if(this->pl == NULL)
		return dist;
	
	/* iterate over poses */
	for(i = 1; i < this->pl_size; i++)
	{
		/* get the incremental distance between poses
		 * (i-1) and (i) */
		inc = this->pl[i].T - this->pl[i-1].T;
		dist += inc.norm();
	}

	/* return total distance */
	return dist;
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

pose_t* system_path_t::get_pose(size_t i) const
{	
	if(i >= this->pl_size)
		return NULL;
	return (this->pl + i);
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
		
void pose_t::compute_transform_ENU(double roll, double pitch,
                                   double yaw)
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
	 *	Rz * Ry * Rx = 
	 *
	 *	cp*cy   cy*sp*sr - cr*sy        cr*cy*sp + sr*sy;
	 *	cp*sy   sp*sr*sy + cr*cy        cr*sp*sy - cy*sr;
	 *	-sp     cp*sr                   cp*cr;
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
	A11 = cp*cy; A12 = cy*sp*sr-cr*sy; A13 = cr*cy*sp+sr*sy;
	A21 = cp*sy; A22 = sp*sr*sy+cr*cy; A23 = cr*sp*sy-cy*sr;
	A31 = -sp;   A32 = cp*sr;          A33 = cp*cr;

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
	this->w(0) = inc(2,1) - inc(1,2);
	this->w(1) = inc(0,2) - inc(2,0);
	this->w(2) = inc(1,0) - inc(0,1);
	this->w /= (2*sin(theta)); /* normalize */

	/* compute the magnitude of this vector to be the rotational
	 * velocity in units of radians per second */
	this->w *= theta / dt;
}
