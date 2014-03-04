#include <iostream>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the main file for the probability-carving program (procarve),
 * which will generate a surface reconstruction of a building interior
 * environment from range scans from a mobile scanning system.
 */

using namespace std;

/* function implementations */

#include <Eigen/Dense>
#include <string>
#include <geometry/probability/noise_model.h>

using namespace Eigen;

/**
 * The main function for this program
 */
int main()
{
	int ret;
	
	/* set input files */
	string dataset = "/home/elturner/Desktop/data/20131204-16/";
	string confile = dataset + "config/backpack_config.xml";
	string madfile = dataset + "Localization/Magneto_TEST_OL_LC_3D.mad";
	string fssfile = dataset
			+ "data/urg/H1214157/urg_H1214157_scandata.fss";
	
	/* read in scans */
	fss::reader_t lasers;
	fss::frame_t frame;
	lasers.set_correct_for_bias(true);
	lasers.set_convert_to_meters(true);
	ret = lasers.open(fssfile);
	if(ret)
	{
		cerr << "Unable to open scan file: " << ret << endl;
		return 1;
	}
	ret = lasers.get(frame, lasers.num_frames()/2);
	if(ret)
	{
		cerr << "Unable to get a frame: " << ret << endl;
		return 2;
	}

	/* prepare structures */
	noise_model_t model;
	ret = model.set_path(madfile, confile);
	if(ret)
	{
		cerr << "madfile: " << madfile << endl;
		cerr << "confile: " << confile << endl;
		cerr << "Unable to set path: " << ret << endl;
		return 3;
	}
	ret = model.set_sensor(lasers.scanner_name());
	if(ret)
	{
		cerr << "Unable to set sensor: " << ret << endl;
		return 3;
	}
	model.set_timestamp(frame.timestamp, 0.001);
	model.set_scan(frame.points[frame.points.size()/2]);

	/* make some samples of this point */
	for(unsigned int i = 0; i < 50; i++)
	{
		/* make a sample */
		Vector3d sensor_pos, scan_pos;
		ret = model.generate_sample(sensor_pos, scan_pos);
		if(ret)
		{
			cerr << "Unable to make a sample: " << ret << endl;
			return 4;
		}

		/* export */
		cout << sensor_pos.transpose() 
		     << " " << scan_pos.transpose() << endl;
	}

	/* success */
	return 0;
}
