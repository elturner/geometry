#include <iostream>
#include <vector>
#include <timestamp/sync_xml.h>
#include <util/cmd_args.h>
#include <util/error_codes.h>
#include "process_scan.h"

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 *
 * This is the main file for the urg filtering program.  It
 * reads in the raw urg scan data, filters the scans based
 * on known urg statistics, and exports the time-synchronized
 * and filtered scans to .fss files.
 */

using namespace std;

/* Function Implementations */

/**
 * The main function for this program
 *
 * This function will be called on the start of the program, and will
 * parse the input/output files from the command-line.  It will read in
 * and filter the urg scans, and export the synchronized and filtered
 * versions of those scans to the specified output location.
 */
int main(int argc, char** argv)
{
	cmd_args_t args;
	vector<string> time_sync_files;
	vector<string> scan_infiles;
	vector<string> scan_outfiles;
	SyncXml timesync;
	unsigned int i, n;
	int ret;

	/* give a basic description of this code */
	args.set_program_description("This program will convert raw "
		"URG scan files (.dat) into the filtered file format "
		"(.fss) for use in statistical processing.");

	/* as input, we require a .dat file (urg scans), and a
	 * xml file (for time synchronization) */
	args.add_required_file_type(TIME_SYNC_EXT, 1,
	                  "Time synchronization file.");
	args.add_required_file_type(URG_SCAN_EXT, 1,
	                  "Hokuyo URG scan data file.");
	args.add_required_file_type(FILTERED_SCAN_EXT, 1,
	                  "Filtered scan output file.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		cerr << "ERROR: invalid arguments (" << ret << ")" << endl;
		return 1; /* invalid command-line arguments */
	}

	/* get command-line files */
	args.files_of_type(TIME_SYNC_EXT, time_sync_files);
	args.files_of_type(URG_SCAN_EXT, scan_infiles);
	args.files_of_type(FILTERED_SCAN_EXT, scan_outfiles);

	/* check that appropriate number of files provided */
	if(time_sync_files.size() > 1)
	{
		/* too many time sync files provided */
		cerr << "ERROR: Only one time sync xml file "
		     << "should be given" << endl;
		return 3;
	}
	if(scan_infiles.size() != scan_outfiles.size())
	{
		/* should be a one-to-one correspondence between
		 * the input and output scan files */
		cerr << "ERROR: The same number of input scan files (."
		     << URG_SCAN_EXT << ") and output scan files (."
		     << FILTERED_SCAN_EXT << ") should be provided."
		     << endl
		     << "# input files given: " << scan_infiles.size()
		     << endl
		     << "# output files given: " << scan_outfiles.size()
		     << endl;
		return 4;
	}

	/* parse the metadata files */
	if(!(timesync.read(time_sync_files[0])))
	{
		/* error in reading time sync file */
		cerr << "ERROR: Could not parse time sync file: "
		     << time_sync_files[0] << endl;
		return 5;
	}

	/* process each input scan file */
	n = scan_infiles.size();
	for(i = 0; i < n; i++)
	{
		/* process each scan file */
		ret = process_scan(timesync,
		                   scan_infiles[i], scan_outfiles[i]);
		if(ret)
		{
			/* error encountered on this scan */
			cerr << "ERROR " << ret << ": "
			     << "Could not process provided scans" << endl;
			return 6;
		}
	}

	/* success */
	return 0;
}

