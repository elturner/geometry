#include "pointcloud_writer.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <float.h>
#include <Eigen/Dense>
#include <io/data/urg/urg_data_reader.h>
#include <io/data/d_imager/d_imager_data_reader.h>
#include <io/data/fss/fss_io.h>
#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <util/error_codes.h>
#include <util/progress_bar.h>

/**
 * @file pointcloud_writer.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This file defines the pointcloud_writer_t class,
 * which is used to convert 3D scans into world
 * coordinates and export them in common pointcloud
 * file formats.
 */

using namespace std;
using namespace Eigen;

/* the following macros are used for unit conversion */
#define MM2METERS(x) ( (x) * 0.001 )

/* the following definitions are settings for output */
#define HEIGHT_COLORING_PERIOD 2.0  /* period of height coloring, meters */
#define MIN_URG_RANGE_VALUE    0.5  /* units: meters */

/* the following defines the default grayscale color of points */
#define DEFAULT_POINT_COLOR 0

/* function implementations */

pointcloud_writer_t::pointcloud_writer_t()
{
	/* set default values */
	this->units = 1;
	this->outfile_format = XYZ_FILE;
	this->coloring = NO_COLOR;
}

pointcloud_writer_t::~pointcloud_writer_t()
{
	/* free all resoureces */
	this->close();
}

int pointcloud_writer_t::open(const  string& pcfile,
                              const  string& pathfile,
                              const  string& timefile,
                              const  string& conffile,
                              double u,
                              COLOR_METHOD c,
			      double maxrange)
{
	int ret;

	/* attempt to parse time sync file */
	if(!(this->time_sync.read(timefile)))
	{
		cerr << "Error!  Unable to parse time file: "
		     << timefile << endl;
		return -1; /* unable to parse */
	}

	/* attempt to parse path */
	ret = this->path.readmad(pathfile);
	if(ret)
	{
		cerr << "Error!  Unable to parse path file: "
		     << pathfile << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* attempt to parse hardware configuration */
	ret = this->path.parse_hardware_config(conffile);
	if(ret)
	{
		cerr << "Error!  Unable to parse hardware config file: "
		     << conffile << endl;
		return PROPEGATE_ERROR(-3, ret);
	}

	/* attempt to open output file for writing */
	this->outfile.open(pcfile.c_str());
	if(!(this->outfile.is_open()))
	{
		cerr << "Error!  Unable to open output file: "
		     << pcfile << endl;
		return -4;
	}

	/* record additional parameters */
	this->units = u; /* record desired units */
	this->coloring = c; /* coloring method to use */
	this->max_range_limit = maxrange; /* range limit option */

	/* get output file type */
	this->outfile_format = pointcloud_writer_t::get_file_type(pcfile);
	if(this->outfile_format == UNKNOWN_FILE)
	{
		cerr << "Error!  Unknown output file format specified: "
		     << pcfile << endl;
		this->outfile.close();
		return -5;
	}

	/* success */
	return 0;
}
		
int pointcloud_writer_t::add_camera(const std::string& metafile,
                                    const std::string& calibfile,
                                    const std::string& imgdir)
{
	fisheye_camera_t cam;
	int ret;

	/* initialize the new camera */
	ret = cam.init(calibfile, metafile, imgdir, this->path);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* add to this structure */
	this->fisheye_cameras.push_back(cam);

	/* success */
	return 0;
}
		
int pointcloud_writer_t::export_urg(const string& name,
                                    const string& datfile)
{
	FitParams timefit;
	MatrixXd scan_points;
	vector<double> coses;
	vector<double> sines;
	vector<double> noise;
	progress_bar_t prog_bar;
	urg_reader_t infile;
	urg_frame_t scan;
	transform_t laser_pose;
	double ts;
	int ret;
	unsigned int i, num_points, num_scans;

	/* open input data file */
	ret = infile.open(datfile);
	if(ret)
	{
		/* report error */
		cerr << "Error!  Unable to open urg data file: "
		     << datfile << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* precompute trig */
	coses.resize(infile.pointsPerScan());
	sines.resize(infile.pointsPerScan());
	num_points = infile.pointsPerScan();
	for(i = 0; i < num_points; i++)
	{
		coses[i] = cos(infile.angleMap()[i]);
		sines[i] = sin(infile.angleMap()[i]);
	}

	/* get timestamp conversion values */
	if(!(time_sync.isMember(name)))
	{
		/* report error */
		cerr << "Error! This sensor is not in time sync file: "
		     << name << endl;
		return -2;
	}
	timefit = time_sync.get(name);

	/* prepare progress bar */
	prog_bar.set_name(name);

	/* iterate through file */
	num_scans = infile.numScans();
	for(i = 0; i < num_scans; i++)
	{
		/* update progress bar */
		prog_bar.update(i, num_scans);

		/* get next scan */
		ret = infile.next(scan);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error! Difficulty parsing urg data file: "
			     << datfile << endl;
			return PROPEGATE_ERROR(-3, ret);
		}

		/* get synchronized timestamp */
		ts = timefit.convert(scan.timestamp);

		/* check if valid timestamp */
		if(this->path.is_blacklisted(ts))
			continue; /* don't import these scans */

		/* rectify the points in this scan */
		ret = pointcloud_writer_t::rectify_urg_scan(scan_points,
					scan, coses, sines,
					this->max_range_limit);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty using urg data from: "
			     << datfile << endl;
			return PROPEGATE_ERROR(-4, ret);
		}
		
		/* get pose of system at this time */
		ret = this->path.compute_transform_for(laser_pose,ts,name);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error! Can't compute laser pose at time "
			     << ts << " for " << name << endl;
			return PROPEGATE_ERROR(-5, ret);
		}

		/* convert to world coordinates */
		laser_pose.apply(scan_points);

		/* write points to output file */
		ret = this->write_to_file(scan_points, i, ts, noise);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty writing to outfile"
			     << endl;
			return PROPEGATE_ERROR(-5, ret);
		}
	}

	/* success */
	infile.close();
	prog_bar.clear();
	return 0;
}

int pointcloud_writer_t::export_tof(const string& name,
                                    const string& datfile)
{
	FitParams timefit;
	MatrixXd scan_points;
	vector<double> noise;
	progress_bar_t prog_bar;
	d_imager_reader_t infile;
	d_imager_frame_t frame;
	transform_t tof_pose;
	double ts;
	int ret;
	unsigned int i, num_frames;

	/* open input data file */
	ret = infile.open(datfile);
	if(ret)
	{
		/* report error */
		cerr << "Error!  Unable to open d-imager data file: "
		     << datfile << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* get timestamp conversion values */
	if(!(time_sync.isMember(name)))
	{
		/* report error */
		cerr << "Error! This sensor is not in time sync file: "
		     << name << endl;
		return -2;
	}
	timefit = time_sync.get(name);

	/* prepare progress bar */
	prog_bar.set_name(name);

	/* iterate through file */
	num_frames = infile.get_num_scans();
	for(i = 0; i < num_frames; i++)
	{
		/* update progress bar */
		prog_bar.update(i, num_frames);

		/* get next scan */
		ret = infile.next(frame);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty parsing tof data file: "
			     << datfile << endl;
			return PROPEGATE_ERROR(-3, ret);
		}

		/* get synchronized timestamp */
		ts = timefit.convert(frame.timestamp);
		
		/* check if valid timestamp */
		if(this->path.is_blacklisted(ts))
			continue; /* don't import these scans */

		/* rectify the points in this scan */
		ret=pointcloud_writer_t::convert_d_imager_scan(scan_points,
					                       frame);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty using tof data from: "
			     << datfile << endl;
			return PROPEGATE_ERROR(-4, ret);
		}
		
		/* get pose of system at this time */
		ret = this->path.compute_transform_for(tof_pose,ts,name);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error! Can't compute tof pose at time "
			     << ts << " for " << name << endl;
			return PROPEGATE_ERROR(-5, ret);
		}

		/* convert to world coordinates */
		tof_pose.apply(scan_points);

		/* write points to output file */
		ret = this->write_to_file(scan_points, i, ts, noise);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty writing to outfile"
			     << endl;
			return PROPEGATE_ERROR(-6, ret);
		}
	}

	/* success */
	infile.close();
	prog_bar.clear();
	return 0;
}
		
int pointcloud_writer_t::export_fss(const std::string& filename)
{
	MatrixXd scan_points;
	progress_bar_t prog_bar;
	fss::reader_t infile;
	fss::frame_t frame;
	transform_t fss_pose;
	vector<double> noise;
	int ret;
	unsigned int i, j, num_frames;

	/* open input data file */
	infile.set_correct_for_bias(true);
	ret = infile.open(filename);
	if(ret)
	{
		/* report error */
		cerr << "Error!  Unable to open fss file: "
		     << filename << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* prepare progress bar */
	prog_bar.set_name(infile.scanner_name());

	/* iterate through file */
	num_frames = infile.num_frames();
	for(i = 0; i < num_frames; i++)
	{
		/* update progress bar */
		prog_bar.update(i, num_frames);

		/* get next scan */
		ret = infile.get(frame, i);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty parsing fss scan #"
			     << i << " from: "
			     << filename << endl;
			return PROPEGATE_ERROR(-2, ret);
		}
		
		/* check if valid timestamp */
		if(this->path.is_blacklisted(frame.timestamp))
			continue; /* don't import these scans */

		/* rectify the points in this scan */
		ret = pointcloud_writer_t::convert_fss_scan(scan_points,
					                       frame);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty using fss data from: "
			     << filename << endl;
			return PROPEGATE_ERROR(-3, ret);
		}
		
		/* get pose of system at this time */
		ret = this->path.compute_transform_for(fss_pose,
				frame.timestamp, infile.scanner_name());
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error! Can't compute fss pose at time "
			     << frame.timestamp << " for "
			     << infile.scanner_name() << endl;
			return PROPEGATE_ERROR(-4, ret);
		}

		/* get the noise of the points */
		noise.resize(frame.points.size());
		for(j = 0; j < frame.points.size(); j++)
			noise[j] = frame.points[j].stddev
					+ frame.points[j].width;

		/* convert to world coordinates */
		fss_pose.apply(scan_points);

		/* write points to output file */
		ret = this->write_to_file(scan_points, i,
		                          frame.timestamp, noise);
		if(ret)
		{
			/* report error */
			prog_bar.clear();
			cerr << "Error!  Difficulty writing to outfile"
			     << endl;
			return PROPEGATE_ERROR(-5, ret);
		}
	}

	/* success */
	infile.close();
	prog_bar.clear();
	return 0;
}

void pointcloud_writer_t::close()
{
	unsigned int i, n;

	/* check if output stream is open */
	if(this->outfile.is_open())
		this->outfile.close();
	
	/* clear input values */
	this->path.clear();

	/* clear camera info */
	n = this->fisheye_cameras.size();
	for(i = 0; i < n; i++)
		this->fisheye_cameras[i].clear();
	this->fisheye_cameras.clear();
}
		
int pointcloud_writer_t::write_to_file(const Eigen::MatrixXd& pts,
                         int ind, double ts, vector<double>& noise)
{
	size_t i, n;
	double x, y, z;
	int red, green, blue;
	int ret;

	/* iterate over points */
	n = pts.cols();
	red = green = blue = DEFAULT_POINT_COLOR;
	for(i = 0; i < n; i++)
	{
		/* get geometry */
		x = pts(0,i);
		y = pts(1,i);
		z = pts(2,i);

		/* optionally color points */
		switch(this->coloring)
		{
			case NEAREST_IMAGE:
				ret = this->color_from_cameras(
					red,green,blue,x,y,z,ts);
				if(ret)
					return PROPEGATE_ERROR(-1, ret);
				break;
			case NO_COLOR:
			default:
				/* make all points default color */
				red = green = blue = DEFAULT_POINT_COLOR;
				break;
			case COLOR_BY_HEIGHT:
				/* color points by height pattern */
				this->height_to_color(red, green, blue, z);
				break;
			case COLOR_BY_NOISE:
				/* check if noise provided */
				if(noise.size() > i)
					this->noise_to_color(red, 
						green, blue, noise[i]);
				break;
			case COLOR_BY_TIME:
				/* color points by timestamp */
				this->time_to_color(red, green, blue, ts);
				break;
		}
		
		/* write points to output file */
		if(this->outfile_format == OBJ_FILE)
			ret = this->write_to_obj_file(x, y, z,
			                              red, green, blue,
			                              ind, ts);
		else if(this->outfile_format == PTS_FILE)
			ret = this->write_to_pts_file(x, y, z,
			                              red, green, blue,
			                              ind, ts);
		else /* write to xyz file by default */
			ret = this->write_to_xyz_file(x, y, z,
			                              red, green, blue,
			                              ind, ts);
	}

	/* check for failure */
	if(this->outfile.fail() || this->outfile.bad())
		return -2;
	return 0;
}

int pointcloud_writer_t::write_to_xyz_file(double x, double y, double z,
                                           int r, int g, int b,
                                           int ind, double ts)
{
	/* write geometry in desired units */
	this->outfile << (x * this->units) << " " 
	              << (y * this->units) << " " 
	              << (z * this->units) 
	              << " " << ((unsigned int) r)
	              << " " << ((unsigned int) g)
	              << " " << ((unsigned int) b)
	              << " " << ind /* index of scan */
	              << " " << ts /* timestamp of scan */
	              << " 0"; /* serial number of scanner */
	
	/* new line at end of point information */
	this->outfile << endl;

	/* check for failure */
	if(this->outfile.fail() || this->outfile.bad())
		return -1;
	return 0;
}

int pointcloud_writer_t::write_to_obj_file(double x, double y, double z,
                                           int r, int g, int b,
                                           int ind, double ts)
{
	/* the following values are not used */
	ind = ind;
	ts = ts;

	/* write geometry in desired units */
	this->outfile << "v " << (x * this->units) << " " 
	              << (y * this->units) << " " 
	              << (z * this->units) 
	              << " " << ((unsigned int) r)
	              << " " << ((unsigned int) g)
	              << " " << ((unsigned int) b);
	
	/* new line at end of point information */
	this->outfile << endl;

	/* check for failure */
	if(this->outfile.fail() || this->outfile.bad())
		return -1;
	return 0;
}

int pointcloud_writer_t::write_to_pts_file(double x, double y, double z,
                                           int r, int g, int b,
                                           int ind, double ts)
{
	/* each line represents a point, formatted as:
	 *
	 *  x y z ts ind r g b
	 */

	/* write geometry in desired units */
	this->outfile << "v " << (x * this->units) << " " 
	              << (y * this->units) << " " 
	              << (z * this->units)
		      << ts << " " << ind
	              << " " << ((unsigned int) r)
	              << " " << ((unsigned int) g)
	              << " " << ((unsigned int) b);
	
	/* new line at end of point information */
	this->outfile << endl;

	/* check for failure */
	if(this->outfile.fail() || this->outfile.bad())
		return -1;
	return 0;
}

void pointcloud_writer_t::height_to_color(int& red, int& green, int& blue,
                                          double h) const
{
	char r, g, b; /* signed values [-128, 127] implicitely mod output */
	
	/* generate cycling pattern based on height */
	r = (char) (256.0 * h / (HEIGHT_COLORING_PERIOD * this->units));
	g = r + 80;
	b = r + 160;

	/* store in output */
	red = abs(2 * r);
	green = abs(2 * g);
	blue = abs(2 * b);

	/* sanitize output */
	if(red >= 256) red = 255;
	if(green >= 256) green = 255;
	if(blue >= 256) blue = 255;
}
		
void pointcloud_writer_t::noise_to_color(int& red, int& green, int& blue,
                                         double n) const
{
	double max_noise = 0.1; /* if noise more than 10cm,
	                         * we have problems */

	/* always use same green value */
	green = 128;

	/* check if noise is in reasonable range */
	if(n < 0)
	{
		/* red means bad, blue means good */
		red = 0;
		blue = 255;
	}
	else if(n > max_noise) 
	{
		/* red means bad, blue means good */
		red = 255;
		blue = 0;
	}
	else
	{
		/* interpolate */
		red = (int)(255 * n / max_noise);
		blue = (int)(255 * (max_noise - n) / max_noise);
	}	
}

void pointcloud_writer_t::time_to_color(int& red, int& green, int& blue,
		                   double ts) const
{
	double st, et, f;
	
	/* get timestamp as a fraction of total time of dataset */
	st = this->path.starttime();
	et = this->path.endtime();
	f = (ts - st) / (et - st);

	/* set values */
	red = abs(255 * (1-f));
	green = abs(100 * (1 - (2*abs(f-0.5))));
	blue = abs(255 * f);

	/* sanitize output */
	if(red >= 256) red = 255;
	if(green >= 256) green = 255;
	if(blue >= 256) blue = 255;
}
		
int pointcloud_writer_t::color_from_cameras(int& red,int& green,int& blue,
                                            double x, double y, double z,
                                            double t)
{
	double q, q_best;
	unsigned int i, n;
	int ret, r, g, b;

	/* search for the best quality, start with lowest possible value */
	q_best = 0;

	/* start with default color */
	red = green = blue = r = g = b = DEFAULT_POINT_COLOR;

	/* iterate over cameras */
	n = this->fisheye_cameras.size();
	for(i = 0; i < n; i++)
	{
		/* get coloring from this camera */
		ret = this->fisheye_cameras[i].color_point(x,y,z,t,r,g,b,q);
		if(ret)
		{
			cerr << "[pointcloud_writer_t::color_from_cameras]"
			     << "\tError " << ret << " from color point "
			     << "using camera #" << i << endl;
			return PROPEGATE_ERROR(-1, ret);
		}

		/* check if this is best quality so far */
		if(q > q_best)
		{
			/* save coloring */
			q_best = q;
			red = r;
			green = g;
			blue = b;
		}
	}

	/* success */
	return 0;
}

int pointcloud_writer_t::rectify_urg_scan(MatrixXd& mat,
                                          const urg_frame_t& scan,
                                          const vector<double>& coses,
                                          const vector<double>& sines,
					  double rangelimit)
{
	size_t i, n, num_written, num_to_write;
	double r;

	/* verify LUT is correct */
	n = scan.num_points;
	if(n != coses.size() || n != sines.size())
		return -1; /* invalid arguments */

	/* count number of good points */
	num_to_write = 0;
	for(i = 0; i < n; i++)
	{
		/* check if we want to keep this point */
		r = MM2METERS(scan.range_values[i]);
		if((rangelimit >= 0 && r > rangelimit) 
				|| r < MIN_URG_RANGE_VALUE)
			continue; /* bad point */

		/* if got here, it's a good point */	
		num_to_write++;
	}

	/* resize the matrix */
	mat.resize(3, num_to_write);

	/* iterate over points */
	num_written = 0;
	for(i = 0; i < n; i++)
	{
		/* check if we should skip this point */
		r = MM2METERS(scan.range_values[i]);
		if((rangelimit >= 0 && r > rangelimit)
				|| r < MIN_URG_RANGE_VALUE)
			continue;

		/* convert this point */
		mat(0, num_written) = MM2METERS(
				scan.range_values[i]*coses[i]);
		mat(1, num_written) = MM2METERS(
				scan.range_values[i]*sines[i]);
		mat(2, num_written) = 0;
		num_written++;
	}

	/* verify we're consistent */
	if(num_to_write != num_written)
		return -2;

	/* success */
	return 0;
}

int pointcloud_writer_t::convert_d_imager_scan(MatrixXd& mat,
		                             const d_imager_frame_t& frame)
{
	size_t i, n;

	/* verify frame validity */
	if(frame.xdat == NULL || frame.ydat == NULL || frame.zdat == NULL)
		return -1; /* invalid */

	/* resize the matrix */
	n = frame.image_width * frame.image_height;
	mat.resize(3, n);

	/* iterate over points */
	for(i = 0; i < n; i++)
	{
		mat(0, i) = MM2METERS(frame.xdat[i]);
		mat(1, i) = MM2METERS(frame.ydat[i]);
		mat(2, i) = MM2METERS(frame.zdat[i]);
	}

	/* success */
	return 0;
}
		
int pointcloud_writer_t::convert_fss_scan(Eigen::MatrixXd& mat,
		                            const fss::frame_t& frame)
{
	size_t i, n;

	/* resize the matrix */
	n = frame.points.size();
	mat.resize(3, n);

	/* iterate over points */
	for(i = 0; i < n; i++)
	{
		mat(0,i) = frame.points[i].x;
		mat(1,i) = frame.points[i].y;
		mat(2,i) = frame.points[i].z;
	}

	/* success */
	return 0;
}
		
pointcloud_writer_t::FILE_TYPE pointcloud_writer_t::get_file_type(
                                                 const std::string& file)
{
	string ext3;

	/* check argument */
	if(file.size() < 3)
		return UNKNOWN_FILE; /* not long enough to have extension */
	ext3 = file.substr(file.size() - 3);

	/* iterate over possible types */
	if(ext3.compare("xyz") == 0 || ext3.compare("txt") == 0)
		return XYZ_FILE;
	if(ext3.compare("obj") == 0)
		return OBJ_FILE;
	if(ext3.compare("pts") == 0)
		return PTS_FILE;

	/* no known file type fits */
	return UNKNOWN_FILE;
}
