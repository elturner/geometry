#include "align_path_run_settings.h"
#include <io/data/ic4/ic4_data_reader.h>
#include <timestamp/sync_xml.h>
#include <geometry/system_path.h>
#include <geometry/transform.h>
#include <util/error_codes.h>
#include <util/tictoc.h>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>

/**
 * @file   main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Will align a system path to global coordinates (e.g. ENU)
 *
 * @section DESCRIPTION
 *
 * This is the main file for the path alignment program.  This program
 * (align_path) will read in a given path (as a .mad file), and will
 * finds the scans' alignment to north.  It will then export a modified
 * version of the path such that it is aligned to the global orientation.
 */

using namespace std;
using namespace Eigen;

/*-------------------------*/
/* helper function headers */
/*-------------------------*/

int import_files(const align_path_run_settings_t& args,
			system_path_t& path,
			ic4_reader_t& ic4data,
			SyncXml& timesync);
int find_magnetic_south(Eigen::Vector3d& south, const system_path_t& path,
			ic4_reader_t& ic4data,
			const SyncXml& timesync);
int adjust_path(const Eigen::Vector3d& south, system_path_t& path);
int export_path(const align_path_run_settings_t& args,
		const system_path_t& path);

/*--------------------------*/
/* function implementations */
/*--------------------------*/

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	align_path_run_settings_t args;
	system_path_t path;
	ic4_reader_t ic4data;
	SyncXml timesync;
	Vector3d south;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* read the input files */
	ret = import_files(args, path, ic4data, timesync);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse input files" << endl;
		return 2;
	}

	/* compute the compass direction in the existing model
	 * coordinate system */
	ret = find_magnetic_south(south, path, ic4data, timesync);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not analyze compass data" << endl;
		return 3;
	}

	/* modify the path to be aligned with the computed direction */
	ret = adjust_path(south, path);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not align path to compass data" << endl;
		return 4;
	}

	/* export path to the output file */
	ret = export_path(args, path);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Unable to export path" << endl;
		return 5;
	}
	
	/* success */
	return 0;
}

/**
 * Imports all files from disk
 *
 * @param args     The parsed command-line arguments
 * @param path     Where to store the input path
 * @param ic4data  The reader for the input ic4 data
 * @param timesync The reader for the time synchronization file
 *
 * @return    Returns zero on success, non-zero on failure.
 */
int import_files(const align_path_run_settings_t& args,
			system_path_t& path,
			ic4_reader_t& ic4data,
			SyncXml& timesync)
{
	tictoc_t clk;
	int ret;

	/* begin timer */
	tic(clk);
	
	/* read input path file */
	ret = path.readmad(args.input_path);
	if(ret)
	{
		cerr << "[import_files]\tCould not read input mad file: "
		     << args.input_path << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* read configuration file */
	ret = path.parse_hardware_config(args.configfile);
	if(ret)
	{
		cerr << "[import_files]\tCould not read xml config file: "
		     << args.configfile << endl;
		return PROPEGATE_ERROR(-2, ret);
	}

	/* read the ic4 data */
	ret = ic4data.open(args.ic4file);
	if(ret)
	{
		cerr << "[import_files]\tCould not read ic4 data file: "
		     << args.ic4file << endl;
		return PROPEGATE_ERROR(-3, ret);
	}

	/* read the time synchronization data */
	if(!timesync.read(args.timefile))
	{
		cerr << "[import_files]\tCould not read timesync xml: "
		     << args.timefile << endl;
		return -4;
	}

	/* success */
	toc(clk, "Importing data");
	return 0;
}

/**
 * Will find the best-fit direction for magnetic south
 *
 * This computation is performed by looking through the 3D compass
 * readings from the IMU, aligning them the model coordinates using
 * the input path, and averaging them.
 *
 * @param south     Where to store the best estimate for magnetic south
 * @param path      The input path
 * @param ic4data   The imu data to process
 * @param timesync  The time synchronization for the imu
 *
 * @return          Returns zero on success, non-zero on failure.
 */
int find_magnetic_south(Eigen::Vector3d& south,
			const system_path_t& path,
			ic4_reader_t& ic4data,
			const SyncXml& timesync)
{
	FitParams timesync_params;
	ic4_frame_t frame;
	transform_t imu2world;
	Vector3d mag_body;
	tictoc_t clk;
	double ts;
	unsigned int i, num_scans, j;
	int ret;

	/* initialize */
	tic(clk);
	south << 0,0,0;
	num_scans = ic4data.get_num_scans();
	timesync_params = timesync.get(ic4data.get_serial_num());

	/* iterate through the scans for this sensor */
	for(i = 0; i < num_scans; i++)
	{
		/* get the next imu frame */
		ret = ic4data.get(i, frame);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);

		/* get the synchronized timestamp */
		ts = timesync_params.convert(frame.timestamp);

		/* get the world position of this frame */
		ret = path.compute_transform_for(imu2world, ts,
				ic4data.get_serial_num());
		if(ret)
			return PROPEGATE_ERROR(-2, ret);

		/* get the compass reading for this frame */
		for(j = 0; j < VECTOR_SIZE; j++)
			mag_body(j) = frame.mag_body_frame[j];
		mag_body.normalize();

		/* rotate this compass reading to model coordinates,
		 * and add to our sum of compass readings */
		south += imu2world.R * mag_body;
	}

	/* take the average compass reading over the entire dataset */
	south /= num_scans;

	/* success */
	toc(clk, "Estimating magnetic south");
	return 0;
}

/**
 * Will adjust the system path so that it is aligned with the specified
 * "south" vector.
 *
 * Given a directional vector and a system path, will adjust the poses
 * of the path so that the "south" vector is represented by south in
 * the map.
 *
 * @param south   The vector to align the path to
 * @param path    The path to modify
 *
 * @return        Returns zero on success, non-zero on failure.
 */
int adjust_path(const Eigen::Vector3d& south, system_path_t& path)
{
	Eigen::Quaternion<double> R;
	Eigen::Vector3d T, s, my;
	tictoc_t clk;
	int ret;

	/* initialize */
	tic(clk);
	T << 0,0,0; /* we aren't going to apply a translation */
	my << 0,-1,0; /* the minus-y direction, which is where
	               * the 'south' vector should ideally be pointing */

	/* get the rotation of the model in order to point the "south"
	 * vector in the -y direction.  This will orient the model
	 * in ENU (East-North-Up) coordinates */
	s = south;
	s(2) = 0; /* we only want to apply yaw rotation to the model */
	s.normalize(); /* we want this vector to be facing in -y dir */
	R.setFromTwoVectors(s, my); /* rotate from s to my */

	/* apply this rotation to all the poses in the model */
	ret = path.apply_transform(R, T);
	if(ret)
	{
		/* report error */
		cerr << "[adjust_path]\tUnable to apply transform to path"
		     << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	toc(clk, "Applying transform");
	return 0;
}

/**
 * Exports the path to the file specified in the command-line arguments
 *
 * @param args   The parsed command-line arguments
 * @param path   The path to export
 *
 * @return       Returns zero on success, non-zero on failure
 */
int export_path(const align_path_run_settings_t& args,
		const system_path_t& path)
{
	tictoc_t clk;
	int ret;

	/* export */
	tic(clk);
	ret = path.writemad(args.output_path);
	if(ret)
	{
		cerr << "[export_path]\t"
		     << "Could not export path to mad file: "
		     << args.output_path << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* success */
	toc(clk, "Exporting path");
	return 0;
}
