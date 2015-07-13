#include "generate_scanorama_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   generate_scanorama_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for generate_scanorama program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * generate_scanorama program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define SETTINGS_FILE     "-s"
#define CONFIGFILE_FLAG   "-c"
#define PATHFILE_FLAG     "-p"
#define MODELFILE_FLAG    "-m"
#define FISHEYE_FLAG      "-f"
#define RECTILINEAR_FLAG  "-r"
#define OUTFILE_FLAG      "-o"
#define BEGIN_IDX_FLAG    "-b"
#define END_IDX_FLAG      "-e"
#define META_OUTFILE_FLAG "--meta"

/* the xml parameters to look for */

#define XML_NUM_ROWS           "scanorama_num_rows"
#define XML_NUM_COLS           "scanorama_num_cols"
#define XML_BLENDWIDTH         "scanorama_blendwidth"
#define XML_MIN_SPACING_DIST   "scanorama_min_spacing_dist"
#define XML_MAX_SPACING_DIST   "scanorama_max_spacing_dist"

/*--------------------------*/
/* function implementations */
/*--------------------------*/
		
generate_scanorama_run_settings_t::generate_scanorama_run_settings_t()
{
	/* initialize fields to default values */
	this->xml_config       = "";
	this->pathfile         = "";
	this->modelfile        = "";
	this->fisheye_cam_metafiles.clear();
	this->fisheye_cam_calibfiles.clear();
	this->fisheye_cam_imgdirs.clear();
	this->rectilinear_cam_metafiles.clear();
	this->rectilinear_cam_calibfiles.clear();
	this->rectilinear_cam_imgdirs.clear();
	this->num_rows         = 1000;
	this->num_cols         = 2000;
	this->min_spacing_dist = 2.0;
	this->max_spacing_dist = 3.0;
	this->ptx_outfile      = "";
	this->meta_outfile     = "";
	this->begin_idx        = 0;
	this->end_idx          = -1;
}

int generate_scanorama_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	XmlSettings settings;
	vector<string> files;
	size_t i, num_cams;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates scanoramas "
			"for camera positions in the specified dataset.  "
			"Scanoramas are a point cloud representation that "
			"is used to indicate a panoramic image with depth "
			"at each pixel.");
	args.add(SETTINGS_FILE, "The xml settings file that defines "
			"parameters used for this scanorama generation.",
			false, 1);
	args.add(CONFIGFILE_FLAG, "The hardware configuration .xml file "
			"for this dataset.", false, 1);
	args.add(PATHFILE_FLAG, "The path trajectory file (either .mad or "
			".noisypath) for this dataset.", false, 1);
	args.add(MODELFILE_FLAG, "The model geometry file (.obj, .ply) for "
			"this dataset.", false, 1);
	args.add(FISHEYE_FLAG, "Specifies a set of fisheye images to use "
			"to color the output.  Expects three arguments:"
			"\n\n\t"
			"<color metadata file> <fisheye calib file> "
			"<image folder>\n\nThe metadata file should be "
			"the output file after bayer converting the images."
			"  The calibration file should be a binary .dat "
			"file representing the ocam calib results.  The "
			"image directory should be the same on that is "
			"referenced by the metadata file.\n\n"
			"Use this flag multiple times to specify multiple "
			"sets of images from different cameras.", true, 3);
	args.add(RECTILINEAR_FLAG, 
			"Specifies a set of rectilinear images to use "
			"to color the output.  Expects three arguments:"
			"\n\n\t"
			"<color metadata file> <rectilinear calib file> "
			"<image folder>\n\nThe metadata file should be "
			"the output file after bayer converting the images."
			"  The calibration file should be a binary .dat "
			"file representing the K-matrix.  The "
			"image directory should be the same on that is "
			"referenced by the metadata file.\n\n"
			"Use this flag multiple times to specify multiple "
			"sets of images from different cameras.", true, 3);
	args.add(OUTFILE_FLAG, "The prefix file path of where to store the "
			"output scanorama files (.ptx).  So, if the value "
			"specified is:\n\n\t\"foo/bar/scan_\"\n\n"
			"then the exported files will be:\n\n"
			"\tfoo/bar/scan_00000000.ptx\n"
			"\tfoo/bar/scan_00000001.ptx\n"
			"\t...",false,1);
	args.add(META_OUTFILE_FLAG, "Specifies where to store the output "
			"metadata associated with each generated scanorama "
			"pose, including file paths and timestamps.  If "
			"specified, this file will be formatted as an "
			"ASCII .scanolist file.", true, 1);
	args.add(BEGIN_IDX_FLAG, "If specified, then only the subset of"
			" scanoramas starting at this index (inclusive) "
			"will be exported.  This value is useful if a "
			"previous run was prematurely terminated, and you "
			"want to start where you left off.  The index "
			"specified is in the output indexing, NOT the "
			"input pose indices.", true, 1);
	args.add(END_IDX_FLAG, "If specified, then only the subset of "
			"scanoramas before this index (exclusive) will be "
			"exported.  This value is useful if you only want "
			"to export a subset of the total scanoramas for a "
			"dataset.  If a negative value is specified, then "
			"all indices until the end of the dataset will be "
			"exported.  The index specified is in the output "
			"indexing, NOT the input pose indices.", true, 1);

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[generate_scanorama_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */
	this->xml_config  = args.get_val(CONFIGFILE_FLAG);
	this->pathfile    = args.get_val(PATHFILE_FLAG);
	this->modelfile   = args.get_val(MODELFILE_FLAG);
	this->ptx_outfile = args.get_val(OUTFILE_FLAG);
	
	/* sort the files associated with the fisheye camera imagery */
	files.clear(); args.tag_seen(FISHEYE_FLAG, files);
	num_cams = files.size() / 3;
	for(i = 0; i < num_cams; i++)
	{
		this->fisheye_cam_metafiles.push_back(  files[3*i]     );
		this->fisheye_cam_calibfiles.push_back( files[3*i + 1] );
		this->fisheye_cam_imgdirs.push_back(    files[3*i + 2] );
	}

	/* sort the files associated with the rectilinear camera imagery */
	files.clear(); args.tag_seen(RECTILINEAR_FLAG, files);
	num_cams = files.size() / 3;
	for(i = 0; i < num_cams; i++)
	{
		this->rectilinear_cam_metafiles.push_back(  files[3*i]   );
		this->rectilinear_cam_calibfiles.push_back( files[3*i+1] );
		this->rectilinear_cam_imgdirs.push_back(    files[3*i+2] );
	}

	/* get the optional arguments */
	if(args.tag_seen(BEGIN_IDX_FLAG))
		this->begin_idx = args.get_val_as<int>(BEGIN_IDX_FLAG);
	else
		this->begin_idx = 0;
	if(args.tag_seen(END_IDX_FLAG))
		this->end_idx = args.get_val_as<int>(END_IDX_FLAG);
	else
		this->end_idx = -1;
	if(args.tag_seen(META_OUTFILE_FLAG))
		this->meta_outfile = args.get_val(META_OUTFILE_FLAG);
	else
		this->meta_outfile = "";

	/* import settings from xml settings file */
	if(!settings.read(args.get_val(SETTINGS_FILE)))
	{
		/* unable to parse settings file */
		ret = -2;
		cerr << "[generate_scanorama_run_settings_t::parse]\t"
		     << "Error " << ret << ": Unable to parse xml "
		     << "settings file for this program." << endl;
		return ret;
	}

	/* read in values from settings file */
	if(settings.is_prop(XML_NUM_ROWS))
		this->num_rows     = settings.getAsUint(XML_NUM_ROWS);
	if(settings.is_prop(XML_NUM_COLS))
		this->num_cols     = settings.getAsUint(XML_NUM_COLS);
	if(settings.is_prop(XML_BLENDWIDTH))
		this->blendwidth   = settings.getAsDouble(XML_BLENDWIDTH);
	if(settings.is_prop(XML_MIN_SPACING_DIST))
		this->min_spacing_dist 
			= settings.getAsDouble(XML_MIN_SPACING_DIST);
	if(settings.is_prop(XML_MAX_SPACING_DIST))
		this->max_spacing_dist 
			= settings.getAsDouble(XML_MAX_SPACING_DIST);

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
