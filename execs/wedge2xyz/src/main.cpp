#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <io/carve/wedge_io.h>
#include <io/carve/carve_map_io.h>
#include <geometry/carve/gaussian/carve_map.h>
#include <geometry/shapes/carve_wedge.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/progress_bar.h>

/**
 * @file main.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  This program will import a .wedge file and export an .xyz file
 *
 * @section DESCRIPTION
 *
 * Given a .wedge and a .carvemap file, which defines the probabilities 
 * and geometry of carve wedges from a set of input scans, will export 
 * these carvemap distributions as points in a point-cloud.
 */

using namespace std;

/* file types */

#define WEDGE_FILE_EXT    "wedge"
#define CARVEMAP_FILE_EXT "carvemap"
#define XYZ_FILE_EXT      "xyz"

/* function implementations */

int main(int argc, char** argv)
{
	cmd_args_t args;
	progress_bar_t progbar;
	vector<string> xyzfiles, cmfiles, wedgefiles;
	cm_io::reader_t infile;
	ofstream outfile;
	carve_map_t cm;
	tictoc_t clk;
	unsigned int i, j, k, num_files, num_frames, num_pts;
	int ret;
	
	/* initialize the arguments for this class */
	args.set_program_description("This program will import a .wedge "
			"file and convert its contents to a .xyz file "
			"for visualization purposes.");
	args.add_required_file_type(CARVEMAP_FILE_EXT, 1,
			"The input files that contain probability models "
			"for each scan point.");
	args.add_required_file_type(WEDGE_FILE_EXT, 0,
			"The input files that contain the indices of which "
			"carve map objects are used by each wedge object. "
			"Wedges allow for interpolating between scans "
			"during the carving process.");
	args.add_required_file_type(XYZ_FILE_EXT, 1,
			"The output point-cloud file.");

	/* retrieve arguments */
	tic(clk);
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* error occurred */
		cerr << "[main]\tCould not initialize from arguments: "
		     << ret << endl;
		return 1;
	}
	args.files_of_type(CARVEMAP_FILE_EXT, cmfiles);
	args.files_of_type(WEDGE_FILE_EXT,    wedgefiles);
	args.files_of_type(XYZ_FILE_EXT,      xyzfiles);

	/* open the outfile for writing */
	outfile.open(xyzfiles[0].c_str());
	if(!(outfile.is_open()))
	{
		/* error occurred */
		cerr << "[main]\tUnable to open file for writing: "
		     << xyzfiles[0] << endl;
		return 2;
	}

	/* iterate through input files */
	num_files = cmfiles.size();
	for(i = 0; i < num_files; i++)
	{
		/* open file for reading */
		ret = infile.open(cmfiles[i]);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to open file for reading: "
			     << cmfiles[i] << endl;
			outfile.close();
			return 3;
		}

		/* iterate through file */
		progbar.set_name("converting carve maps");
		num_frames = infile.num_frames();
		for(j = 0; j < num_frames; j++)
		{
			/* inform user of progress */
			progbar.update(j, num_frames);
			
			/* read in next frame */
			num_pts = infile.num_points_in_frame(j);
			for(k = 0; k < num_pts; k++)
			{
				/* get the next point */
				ret = infile.read(cm, j, k);
				if(ret)
				{
					cerr << "[main]\tError " << ret 
					     << ": "
					     << "Could not get carvemap #"
					     << j << endl;
					infile.close();
					outfile.close();
					return 4;
				}

				/* export to outfile */
				cm.writexyz(outfile);
			}
		}

		/* close the file */
		infile.close();
		progbar.clear();
	}

	/* clean up */
	toc(clk, "Writing all points");
	outfile.close();
	return 0;
}
