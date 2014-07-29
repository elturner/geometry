#include <iostream>
#include <vector>
#include <string>
#include <config/backpackConfig.h>
#include <config/laserProp.h>
#include <io/data/urg/urg_data_reader.h>
#include <io/data/msd/msd_io.h>
#include <timestamp/sync_xml.h>
#include <util/tictoc.h>
#include <util/cmd_args.h>
#include <util/error_codes.h>
#include <util/progress_bar.h>
#include <util/rotLib.h>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This is the main file for the generate_msd program
 *
 * @section DESCRIPTION
 *
 * This program is used to generate a .msd file from the binary .dat
 * files of the laser scanners.  The msd format is used by the gen-1
 * backpack software to house laser scans.
 */

using namespace std;
using namespace Eigen;

/*-------------------*/
/* command-line tags */
/*-------------------*/

#define CONFIG_FLAG      "-c"
#define TIMESYNC_FLAG    "-t"
#define INPUT_FLAG       "-i"
#define OUTPUT_FLAG      "-o"

/*-------------------------*/
/* helper function headers */
/*-------------------------*/

/**
 * Initializes the command-line args parser for this program
 *
 * Will populate this parser object with the user-interface usage
 * information, which includes help dialogs.
 *
 * @param args   The object to modify
 */
void init(cmd_args_t& args);

/**
 * Prepares the output msd file
 *
 * After this call, the file will be opened (with the header
 * already written), and ready to write frames.
 *
 * @param infile    The input urg file
 * @param conf      The backpack hardware config file
 * @param timesync  The time synchronization structure to search
 * @param filename  Where to write the output file
 * @param outfile   The output file to initialize
 * @param timefit   The time fit pararms for this scanner
 *
 * @return    Returns zero on success, non-zero on failure.
 */
int prepare_outfile(const urg_reader_t& infile, backpackConfig& conf,
		const SyncXml& timesync, const string& filename, 
		msd::writer_t& outfile, FitParams& timefit);

/**
 * Converts a urg frame to a msd frame
 *
 * Will convert the geometry information from the urg frame to
 * the given msd frame.
 *
 * @param infile    The input urg file to use
 * @param timefit   The timesync fit params to use
 * @param inframe   The frame to convert from
 * @param outframe  The frame to convert to
 */
void convert_frames(const urg_reader_t& infile, FitParams& timefit, 
		const urg_frame_t& inframe, msd::frame_t& outframe);

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int main(int argc, char** argv)
{
	cmd_args_t args;
	backpackConfig conf;
	SyncXml timesync;
	FitParams timefit;
	urg_reader_t infile;
	urg_frame_t inframe;
	msd::writer_t outfile;
	msd::frame_t outframe;
	vector<string> infiles;
	vector<string> outfiles;
	progress_bar_t progbar;
	size_t i, j, n, m;
	tictoc_t clk;
	int ret;

	/* initialize the command-line argument parser */
	init(args);
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse args */
		cerr << "[main]\tError " << ret << ": "
		     << "Could not parse command line args" << endl;
		return 1;
	}

	/* get backpack configuration file */
	if(!conf.read_config_file(args.get_val(CONFIG_FLAG)))
	{
		/* unable to access file */
		cerr << "[main]\tError: "
		     << "Unable to read/parse xml hardware config "
		     << "file" << endl;
		return 2;
	}

	/* get time synchronization file */
	if(!timesync.read(args.get_val(TIMESYNC_FLAG)))
	{
		/* unable to access file */
		cerr << "[main]\tError: "
		     << "Unable to read/parse xml timesync file" << endl;
		return 3;
	}

	/* get the input and output files */
	args.tag_seen(INPUT_FLAG, infiles);
	args.tag_seen(OUTPUT_FLAG, outfiles);
	if(infiles.size() != outfiles.size())
	{
		/* must be same number of inputs and outputs */
		cerr << "[main]\tError: "
		     << "Different number of input files and output files "
		     << "specified" << endl;
		return 4;
	}

	/* iterate through the given files */
	n = infiles.size();
	for(i = 0; i < n; i++)
	{
		/* open current input file */
		ret = infile.open(infiles[i]);
		if(ret)
		{
			/* unable to read file */
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to read input file: "
			     << infiles[i] << endl;
			return 5;
		}

		/* open current output file */
		ret = prepare_outfile(infile, conf, timesync, 
				outfiles[i], outfile, timefit);
		if(ret)
		{
			/* cannot open outfile */
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to open outfile for writing: "
			     << outfiles[i] << endl;
			return 6;
		}

		/* iterate over frames */
		progbar.set_name(infile.serialNum());
		tic(clk);
		m = infile.numScans();
		for(j = 0; j < m; j++)
		{
			/* inform user of progress */
			progbar.update(j, m);

			/* get the next frame */
			ret = infile.next(inframe);
			if(ret)
			{
				/* unable to read frame */
				progbar.clear();
				cerr << "[main]\tError " << ret << ": "
				     << "Unable to read frame #" << j
				     << " from infile " << infiles[i]
				     << endl;
				return 6;
			}

			/* convert from urg format to msd format */
			convert_frames(infile, timefit, inframe, outframe);

			/* export to the msd file */
			outfile.write(outframe);
		}

		/* clean up */
		progbar.clear();
		toc(clk, infile.serialNum().c_str());
		infile.close();
		outfile.close();
	}

	/* success */
	return 0;
}

void init(cmd_args_t& args)
{
	args.set_program_description("This program is used to generate a "
			".msd file from .dat urg laser file.  The .dat "
			"file contains raw laser scans collected off of "
			"gen-2+ backpacks, while the .msd file is how "
			"laser scans are stored in the gen-1 backpack.");
	args.add(CONFIG_FLAG, "Specifies the hardware xml configuration "
			"file used by this dataset.", false, 1);
	args.add(TIMESYNC_FLAG, "Specifies the time-sync xml file, which "
			"represents the conversion from the laser clock "
			"to the synchronized backpack system clock.",
			false, 1);
	args.add(INPUT_FLAG, "Specifies the location of the input .dat "
			"file to convert.  This flag can be given multiple "
			"times, which will convert each file in series.", 
			false, 1);
	args.add(OUTPUT_FLAG, "Specifies where to write the output .msd "
			"file.  This flag must be given the same number of "
			"times as the " INPUT_FLAG " flag.", false, 1);
}

int prepare_outfile(const urg_reader_t& infile, backpackConfig& conf,
		const SyncXml& timesync, const string& filename, 
		msd::writer_t& outfile, FitParams& timefit)
{
	Eigen::Vector3d T;
	Eigen::Matrix3d R;
	laserProp laser;
	int serial, ret;

	/* determine the fit params to use for this scanner */
	if(!(timesync.isMember(infile.serialNum())))
		return -1;
	timefit = timesync.get(infile.serialNum());

	/* get the serial number as an integer */
	serial = infile.serialNumAsInt();

	/* get the hardware extrinsics transform for this sensor */
	if(!(conf.get_props(infile.serialNum(), laser, false)))
		return -2;
	T(0) = laser.tToCommon[0]; /* keep this in millimeters */
	T(1) = laser.tToCommon[1]; 
	T(2) = laser.tToCommon[2];
	laser.toRadianMeters(); /* we want the angles in radians */
	rotLib::rpy2rot(laser.rToCommon[0], laser.rToCommon[1],
				laser.rToCommon[2], R);

	/* open the output file */
	outfile.init(serial, R, T, infile.numScans());
	ret = outfile.open(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

void convert_frames(const urg_reader_t& infile, FitParams& timefit, 
		const urg_frame_t& inframe, msd::frame_t& outframe)
{
	size_t i, n;

	/* prepare fields of output frame */
	outframe.num_points = inframe.num_points;
	outframe.timestamp = timefit.convert(inframe.timestamp);
	outframe.points.resize(2, outframe.num_points);
	
	/* copy over the point information */
	n = outframe.num_points;
	for(i = 0; i < n; i++)
	{
		/* rectify this point (both input and output have
		 * units of millimeters) */
		outframe.points(0,i) = cos(infile.angleMap()[i]) 
					* inframe.range_values[i];
		outframe.points(1,i) = sin(infile.angleMap()[i])
					* inframe.range_values[i];
	}
}
