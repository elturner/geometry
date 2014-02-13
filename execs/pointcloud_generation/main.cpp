#include <iostream>
#include <string>
#include <vector>
#include <io/pointcloud/pointcloud_writer.h>
#include <util/cmd_args.h>
#include <util/progress_bar.h>
#include <util/tictoc.h>
#include <util/error_codes.h>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the main file for the pointcloud generation program,
 * which will read in a 3D pose file, laser scans, and system
 * information in order to generate a pointcloud file.
 */

using namespace std;

/* the following defines are used for parsing command arguments */
#define TIME_SYNC_FILE_FLAG       "-t"
#define HARDWARE_CONFIG_FILE_FLAG "-c"
#define PATH_FILE_FLAG            "-p"
#define LASER_FILE_FLAG           "-l"
#define D_IMAGER_FILE_FLAG        "-d"
#define FSS_FILE_FLAG             "--fss"
#define FISHEYE_CAMERA_FLAG       "-f"
#define UNITS_FLAG                "-u"
#define OUTPUT_FILE_FLAG          "-o"
#define COLOR_BY_HEIGHT_FLAG      "--color_by_height"
#define COLOR_BY_NOISE_FLAG       "--color_by_noise"

/* the following are helper functions for this program */
void init_args(cmd_args_t& args);
int init_writer(pointcloud_writer_t& writer, cmd_args_t& args);
int process_all_files(pointcloud_writer_t& writer, cmd_args_t& args);

/* This is the main function for this program */
int main(int argc, char** argv)
{
	pointcloud_writer_t writer;
	cmd_args_t args;
	tictoc_t clk;

	/* begin */
	tic(clk);

	/* parse arguments */
	init_args(args);
	if(args.parse(argc, argv))
		return 1;

	/* retrieve argument information */
	if(init_writer(writer, args))
		return 2;

	/* iterate through all scanner files, export to output */
	if(process_all_files(writer, args))
		return 3;

	/* success */
	toc(clk, "Generating Point Cloud");
	return 0;
}

/*** helper functions ***/

/**
 * Initializes the command-line usage structure
 *
 * Will populate the command-line usage structure
 * with the necessary values to search for from
 * the user.
 *
 * @param args   The structure to initialize
 */
void init_args(cmd_args_t& args)
{
	args.add(TIME_SYNC_FILE_FLAG, /* specifies time sync file (*.xml) */
	               "Specifies the time synchronization file to use.  "
	               "This should be a .xml file generated by the "
	               "time synchronization code.", false, 1);
	args.add(HARDWARE_CONFIG_FILE_FLAG, /* hardware config (.xml) */
	               "Specifies the hardware configuration .xml file. "
	               "This file should contain all the sensor "
	               "transformations.", false, 1);
	args.add(PATH_FILE_FLAG, /* specifies path (*.mad) */
	               "Specifies the 3D path file, generated by the "
	               "localization code.  This can be a *.mad file.",
	               false, 1);
	args.add(LASER_FILE_FLAG, /* specifies laser name and .dat file */
	               "Specifies two arguments:  <laser name> and <laser "
	               "data file>.  The laser name should be the same as "
	               "in the hardware configuration file.  The laser data"
	               " file should be what was originally exported during"
	               " the data acquisition.", true, 2);
	args.add(D_IMAGER_FILE_FLAG, /* specifies d-imager name/file */
	               "Specifies two arguments:  <d-imager name> and "
                       "<d-imager data file>.  The name should be the same "
                       "as in the hardware config file.  The d-imager data "
                       "file should be what was originally exported during "
                       "the data acquisition.", true, 2);
	args.add(FSS_FILE_FLAG, /* specifies fss file */
	               "Specifies one arguments:  <fss file path>.  This "
	               "file specifies a filtered range scan list.",true,1);
	args.add(FISHEYE_CAMERA_FLAG, /* specifies params for a camera */
	               "Specifies three arguments: <color "
	               "metadata file> <fisheye calibration file> <image "
	               "folder>.  The metadata file should be "
	               "the output file after bayer converting the images."
	               "  The calibration file should be a binary .dat file"
	               " representing the ocam calib results.  The image "
	               "directory should be the same one that is referenced"
	               " by the metadata file.", true, 3);
	args.add(UNITS_FLAG, /* specifies units to use in output file */
	               "Given floating-point value specifies the units to "
                       "use in the output file.  A value of 1.0 indicates "
                       "units of meters.  A value of 1000.0 indicates units"
                       " of millimeters.  Value of 3.28084 indicates "
                       "units of feet.  The default value is 1.0 (meters).",
                       true, 1);
	args.add(OUTPUT_FILE_FLAG, /* where to store the output */
	               "Specifies the file location of where to export the "
                       "generated pointcloud file.  Valid file formats are "
                       "any of:  *.txt, *.xyz, *.obj, *.pts", false, 1);
	args.add(COLOR_BY_HEIGHT_FLAG, /* colors pointcloud by height */
	               "If seen, will explicitly color the output points "
	               "based on their height, allowing for the geometry "
	               "to be easily observed.  This flag will override "
	               "coloring from images, even if cameras are provided."
	               );
	args.add(COLOR_BY_NOISE_FLAG, /* colors pointcloud by noise */
	               "If seen, will explicitly color the output points "
	               "based on their noise values, if such info is "
	               "provided.  Noise estimates only available from "
	               ".fss files.  This flag will override "
	               "coloring from images, even if cameras are provided."
	               );
}

/**
 * Parses the specified input files, and inits pointcloud_writer_t structure
 *
 * Will attempt to open and read the files specified on the command-
 * line arguments, and if successful, will store the result in the
 * given pointcloud_writer_t structure, opening the writer.
 *
 * @param writer   The writer to initialize and open
 * @param args     The parsed command-line arguments to use
 *
 * @return  Returns zero on success, non-zero on failure.
 */
int init_writer(pointcloud_writer_t& writer, cmd_args_t& args)
{
	string pathfile, conffile, timefile, outfile;
	vector<string> fisheye_tags;
	pointcloud_writer_t::COLOR_METHOD c;
	double units;
	int ret, i, n;
	tictoc_t clk;

	/* time this function */
	tic(clk);

	/* get the filenames */
	pathfile = args.get_val(PATH_FILE_FLAG);
	conffile = args.get_val(HARDWARE_CONFIG_FILE_FLAG);
	timefile = args.get_val(TIME_SYNC_FILE_FLAG);
	outfile  = args.get_val(OUTPUT_FILE_FLAG);

	/* get optional parameters */

	/* units */
	if(args.tag_seen(UNITS_FLAG))
		units = args.get_val_as<double>(UNITS_FLAG);
	else
		units = 1.0; /* default units of meters */

	/* get coloring method */
	if(args.tag_seen(COLOR_BY_HEIGHT_FLAG))
		/* use height of points to color */
		c = pointcloud_writer_t::COLOR_BY_HEIGHT;
	else if(args.tag_seen(COLOR_BY_NOISE_FLAG))
	{
		c = pointcloud_writer_t::COLOR_BY_NOISE;
	}
	else if(args.tag_seen(FISHEYE_CAMERA_FLAG, fisheye_tags))
	{
		/* use camera images to color */
		c = pointcloud_writer_t::NEAREST_IMAGE;
	}
	else
		c = pointcloud_writer_t::NO_COLOR;

	/* attempt to open file */
	ret = writer.open(outfile, pathfile, timefile, conffile, units, c);
	if(ret)
	{
		/* unable to initialize writer */
		cerr << "Error " << ret << ": Unable to initialze writer"
		     << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* read in camera information, if they are being used */
	if(c == pointcloud_writer_t::NEAREST_IMAGE)
	{
		/* iterate over command-line args */
		n = fisheye_tags.size() / 3;
		for(i = 0; i < n; i++)
		{
			/* add this camera */
			ret = writer.add_camera(
				fisheye_tags[3*i], /* metadata file */
				fisheye_tags[3*i + 1], /* calib file */
				fisheye_tags[3*i + 2]); /* img dir */
			if(ret)
			{
				cerr << "Error " << ret << ": Unable to "
				     << "initialize camera #" << i << endl;
				return PROPEGATE_ERROR(-2, ret);
			}
		}
	}

	/* success */
	toc(clk, "Initializing parameters");
	return 0;
}

/**
 * Will iterate over all provided sensor files, and export them to output
 *
 * The input files can be from any 3D scanner from the system.  They
 * will be parsed, converted to world coordinates, and written to the
 * specified output file.
 *
 * @param writer  The writer to use to export these files
 * @param args    The command-line arguments used to specify the input
 *
 * @return    Returns zero on success, non-zero on failure.
 */
int process_all_files(pointcloud_writer_t& writer, cmd_args_t& args)
{
	vector<string> laser_files;
	vector<string> d_imager_files;
	vector<string> fss_files;
	size_t i, n;
	int ret;

	/* get laser files */
	if(args.tag_seen(LASER_FILE_FLAG, laser_files))
	{
		/* iterate over laser files */
		n = laser_files.size();
		for(i = 0; i < n; i += 2)
		{
			/* parse the next laser and its file */
			ret = writer.export_urg(laser_files[i],
			                        laser_files[i+1]);
			if(ret)
			{
				/* report error */
				cerr << "Error " << ret 
				     << ": Unable to export "
				     << laser_files[i] << endl;
				return PROPEGATE_ERROR(-1, ret);
			}
		}
	}

	/* get d_imager data files, if provided */
	if(args.tag_seen(D_IMAGER_FILE_FLAG, d_imager_files))
	{
		/* iterate over d-imager files */
		n = d_imager_files.size();
		for(i = 0; i < n; i += 2)
		{
			/* parse next d-imager file */
			ret = writer.export_tof(d_imager_files[i],
			                        d_imager_files[i+1]);
			if(ret)
			{
				/* report error */
				cerr << "Error " << ret 
				     << ": Unable to export "
				     << d_imager_files[i] << endl;
				return PROPEGATE_ERROR(-2, ret);
			}
		}
	}
	
	/* get fss files, if provided */
	if(args.tag_seen(FSS_FILE_FLAG, fss_files))
	{
		/* iterate over fss files */
		n = fss_files.size();
		for(i = 0; i < n; i ++)
		{
			/* parse next fss file */
			ret = writer.export_fss(fss_files[i]);
			if(ret)
			{
				/* report error */
				cerr << "Error " << ret 
				     << ": Unable to export "
				     << fss_files[i] << endl;
				return PROPEGATE_ERROR(-3, ret);
			}
		}
	}

	/* success */
	return 0;
}
