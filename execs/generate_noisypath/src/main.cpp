#include "noisypath_run_settings.h"
#include <io/carve/noisypath_io.h>
#include <geometry/system_path.h>
#include <util/rotLib.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <vector>
#include <iostream>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The main file for the noisypath generation program
 *
 * @section DESCRIPTION
 *
 * This program will import path and statistical information exported
 * by the localization process, and construct a noisypath file.
 */

using namespace std;
using namespace Eigen;

/* function headers */

int mad2noisy(const noisypath_run_settings_t& args);

/* function implementations */

/**
 * The main function for this program
 */
int main(int argc, char** argv)
{
	noisypath_run_settings_t args;
	int ret;

	/* parse the given parameters */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse parameters" << endl;
		return 1;
	}

	/* process:
	 *
	 * Determine what to do based on what was given */
	if(args.linear_sigma >= 0 
			&& args.rotational_sigma >= 0 
			&& !args.madfile.empty())
	{
		/* user wants to convert mad file to noisypath
		 * with constant uncertainties */
		ret = mad2noisy(args);
		if(ret)
		{
			/* report error */
			cerr << "[main]\tError " << ret << ": Unable to "
			     << "convert mad file to noisypath file."
			     << endl;
			return 2;
		}
	}
	else
	{
		/* don't know how to do anything else */
		cerr << "[main]\tError!  Not enough parameters were given "
		     << "to generate .noisypath file" << endl;
		return 3;
	}

	/* success */
	return 0;
}

/**
 * Will convert a mad file to a noisypath file, using constant
 * uncertainty values.
 *
 * @param args   The user-given arguments to use
 */
int mad2noisy(const noisypath_run_settings_t& args)
{
	noisypath_io::writer_t outfile;
	noisypath_io::pose_t poseout;
	system_path_t path;
	pose_t* posein;
	vector<pair<double, double> > zps; /* zupts as pairs */
	vector<noisypath_io::zupt_t> zupts;
	size_t i, n;
	int ret;

	/* import the mad file */
	ret = path.readmad(args.madfile);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* get list of zupts */
	path.get_zupts(zps);
	n = zps.size();
	zupts.resize(n);
	for(i = 0; i < n; i++)
	{
		zupts[i].start_time = zps[i].first;
		zupts[i].end_time = zps[i].second;
	}

	/* prepare to write noisypath file */
	ret = outfile.open(args.outfile, zupts);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* write each pose */
	n = path.num_poses();
	for(i = 0; i < n; i++)
	{
		/* get the next pose to export */
		posein = path.get_pose(i);
		if(posein == NULL)
		{
			/* invalid pose */
			outfile.close();
			return PROPEGATE_ERROR(-3, ret);
		}

		/* convert to noisypath pose */

		/* timestamp */
		poseout.timestamp = posein->timestamp;

		/* position mean and cov */
		poseout.position.mean = posein->T;
		poseout.position.cov = args.linear_sigma 
					* Matrix3d::Identity();
		
		/* rotation mean and cov */
		rotLib::rot2rpy(posein->R.toRotationMatrix(),
				poseout.rotation.mean);
		poseout.rotation.cov = args.rotational_sigma
					* Matrix3d::Identity();

		/* export to file */
		ret = outfile.write(poseout);
		if(ret)
		{
			/* unable to export */
			outfile.close();
			return PROPEGATE_ERROR(-4, ret);
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}
