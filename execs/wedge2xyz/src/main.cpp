#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <io/carve/wedge_io.h>
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
 * Given a .wedge file, which defines the probabilities and geometry of
 * carve wedges from a set of input scans, will export these wedges as
 * points in a point-cloud.
 */

using namespace std;

/* file types */

#define WEDGE_FILE_EXT   "wedge"
#define XYZ_FILE_EXT     "xyz"

/* function implementations */

int main(int argc, char** argv)
{
	cmd_args_t args;
	progress_bar_t progbar;
	vector<string> xyzfiles, wedgefiles;
	wedge::reader_t infile;
	ofstream outfile;
	carve_wedge_t w;
	tictoc_t clk;
	unsigned int i, j, n, m;
	int ret;

	/* initialize the arguments for this class */
	args.set_program_description("This program will import a .wedge "
			"file and convert its contents to a .xyz file "
			"for visualization purposes.");
	args.add_required_file_type(WEDGE_FILE_EXT, 1,
			"The input files that contain probability models "
			"for each scan point");
	args.add_required_file_type(XYZ_FILE_EXT, 1,
			"The output point-cloud file");

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
	args.files_of_type(WEDGE_FILE_EXT, wedgefiles);
	args.files_of_type(XYZ_FILE_EXT, xyzfiles);

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
	n = wedgefiles.size();
	for(i = 0; i < n; i++)
	{
		/* open file for reading */
		ret = infile.open(wedgefiles[i]);
		if(ret)
		{
			cerr << "[main]\tError " << ret << ": "
			     << "Unable to open file for reading: "
			     << wedgefiles[i] << endl;
			outfile.close();
			return 3;
		}

		/* iterate through file */
		progbar.set_name("converting wedges");
		m = infile.num_wedges();
		for(j = 0; j < m; j++)
		{
			/* read in next wedge */
			progbar.update(j, m);
			ret = infile.get(w, j);
			if(ret)
			{
				cerr << "[main]\tError " << ret << ": "
				     << "Could not get wedge #" << j
				     << endl;
				infile.close();
				outfile.close();
				return 4;
			}

			/* export to outfile */
			w.writexyz(outfile);
			w.free_maps();
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
