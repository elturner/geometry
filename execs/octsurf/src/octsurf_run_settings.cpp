#include "octsurf_run_settings.h"
#include <xmlreader/xmlsettings.h>
#include <util/cmd_args.h>
#include <util/tictoc.h>
#include <util/error_codes.h>
#include <iostream>
#include <string>
#include <vector>

/**
 * @file   octsurf_run_settings.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Gets user-defined run settings for octsurf program
 *
 * @section DESCRIPTION
 *
 * This file contains classes used to parse and store
 * user-defined run parameters and settings for the
 * octsurf program.  This is a wrapper class around
 * cmd_args_t, which is used to parse command-line
 * arguments.
 */

using namespace std;

/* the command-line flags to check for */

#define SETTINGS_FLAG     "-s" /* program-specific settings (.xml) */
#define OUTPUT_FLAG       "-o" /* where to store the output file */
#define EXPORT_LEAFS_FLAG "-l" /* export leafs to OBJ */
#define EXPORT_FACES_FLAG "--node_faces" /* export node faces to OBJ */
#define EXPORT_OBJECTS_FLAG "--objects"  /* export objects only */
#define EXPORT_ROOM_FLAG    "--room"     /* export rooms only */
#define EXPORT_REGIONS_FLAG "--regions"  /* export planar region geometry */
#define EXPORT_CORNERS    "--corners" /* export node corners */

/* file extensions to check for */

#define OCT_FILE_EXT   "oct"
#define PLY_FILE_EXT   "ply"
#define VOX_FILE_EXT   "vox"
#define OBJ_FILE_EXT   "obj"
#define TXT_FILE_EXT   "txt"
#define SOF_FILE_EXT   "sof"
#define SOG_FILE_EXT   "sog"

/* function implementations */
		
octsurf_run_settings_t::octsurf_run_settings_t()
{
	/* set default values for this program's files */
	this->octfiles.clear();
	this->outfile = "";
	this->output_format = FORMAT_UNKNOWN;
}

int octsurf_run_settings_t::parse(int argc, char** argv)
{
	cmd_args_t args;
	XmlSettings settings;
	string settings_file;
	tictoc_t clk;
	int ret;

	/* populate args with what we expect on the command-line */
	tic(clk);
	args.set_program_description("This program generates meshed surface"
			" reconstructions from an input .oct file.  The "
			"input file should be generated using the procarve"
			" program.");
	args.add(SETTINGS_FLAG, "A .xml settings file for this program.  "
			"This file should contain run parameters for how "
			"to generate chunks and where to store them on "
			"disk.", true, 1);
	args.add(OUTPUT_FLAG, "Where to store the output file, which "
			"represents the meshed surface of the volume "
			"described by the input .oct files.  This program "
			"supports multiple output file formats, including: "
			".vox, .obj, .ply, .sof, .sog, .txt", false, 1);
	args.add(EXPORT_LEAFS_FLAG, "If present, this flag indicates that "
			"all leaf centers of the octree should be exported "
			"to the specified OBJ file.  This flag will be "
			"ignored if the output file is not .obj.  If this "
			"flag is not present, then a mesh will be exported "
			"to the file.", true, 0);
	args.add(EXPORT_FACES_FLAG, "If present, this flag indicates that "
			"the output mesh should be the boundary leaf node "
			"faces without any surface reconstruction.  This "
			"flag will be ignored if the output file is not "
			".obj or .ply.  If this flag is not present, then "
			"the mesh will be processed normally.", true, 0);
	args.add(EXPORT_REGIONS_FLAG, "If present, this flag indicates "
			"that the output mesh should be of region geometry."
			"  This means that the output will be the boundary "
			"node faces, but colored based on their region.  "
			"This flag is only valid when the output is to an "
			".obj file.  If this flag is not present, then the "
			"output will be processed normally.",
			true, 0);
	args.add(EXPORT_OBJECTS_FLAG, "If present, then will only export "
			"geometry that represents objects within the rooms "
			"of the model, such as furniture.  The output model"
			" will not contain the room geometry itself, such "
			"as floors, walls, and ceilings.", true, 0);
	args.add(EXPORT_ROOM_FLAG, "If present, then will only export "
			"geometry that represents the rooms of the "
			"environment, such as floors, walls, and ceilings. "
			"Will not export the object geometry, such as "
			"furniture in those rooms.", true, 0);
	args.add(EXPORT_CORNERS, "If present, this flag indicates "
			"that the output should be a set of verticess that "
			"represent the corners of the tree nodes.",true,0);
	args.add_required_file_type(OCT_FILE_EXT, 1,
			"The input octree files.  These represent the "
			"volume information of the scanned environment, and"
			" are processed at a given resolution.");

	/* parse the command-line arguments */
	ret = args.parse(argc, argv);
	if(ret)
	{
		/* unable to parse command-line arguments.  Inform
		 * user and quit. */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[octsurf_run_settings_t::parse]\t"
		     << "Unable to parse command-line arguments:  "
		     << "Error " << ret << endl;
		return ret;
	}

	/* populate this object with what was parsed from
	 * the command-line */
	this->outfile           = args.get_val(OUTPUT_FLAG);
	this->output_format 
		= octsurf_run_settings_t::get_format(this->outfile);
	this->export_obj_leafs  = args.tag_seen(EXPORT_LEAFS_FLAG);
	this->export_node_faces = args.tag_seen(EXPORT_FACES_FLAG);
	this->export_objects    = args.tag_seen(EXPORT_OBJECTS_FLAG);
	this->export_room       = args.tag_seen(EXPORT_ROOM_FLAG);
	this->export_regions    = args.tag_seen(EXPORT_REGIONS_FLAG);
	this->export_corners    = args.tag_seen(EXPORT_CORNERS);
	args.files_of_type(OCT_FILE_EXT, this->octfiles);

	/* check if a settings xml file was specified */
	if(args.tag_seen(SETTINGS_FLAG))
	{
		/* retrieve the specified file */
		settings_file = args.get_val(SETTINGS_FLAG);
	
		/* attempt to open and parse the settings file */
		if(!settings.read(settings_file))
		{
			/* unable to open or parse settings file.  Inform
			 * user and quit. */
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[octsurf_run_settings_t::parse]\t"
			     << "Error " << ret << ":  Unable to parse "
			     << "settings file: " << settings_file << endl;
			return ret;
		}
	
		/* read in settings from file.  If they are not in the given
		 * file, then the default settings that were set in this
		 * object's constructor will be used. */

		// No settings needed
	}

	/* we successfully populated this structure, so return */
	toc(clk, "Importing settings");
	return 0;
}
		
OUTPUT_FILE_FORMAT octsurf_run_settings_t::get_format(const std::string& fn)
{
	string ext;
	size_t dotpos;

	/* find the position of the file extension */
	dotpos = fn.find_last_of('.');
	if(dotpos == string::npos)
		return FORMAT_UNKNOWN;
	
	/* get extension as string */
	ext = fn.substr(dotpos+1);

	/* determine format */
	if(!ext.compare(VOX_FILE_EXT))
		return FORMAT_VOX;
	if(!ext.compare(OBJ_FILE_EXT))
		return FORMAT_OBJ;
	if(!ext.compare(PLY_FILE_EXT))
		return FORMAT_PLY;
	if(!ext.compare(TXT_FILE_EXT))
		return FORMAT_TXT;
	if(!ext.compare(SOF_FILE_EXT))
		return FORMAT_SOF;
	if(!ext.compare(SOG_FILE_EXT))
		return FORMAT_SOG;
	return FORMAT_UNKNOWN; /* unknown file format */
}
