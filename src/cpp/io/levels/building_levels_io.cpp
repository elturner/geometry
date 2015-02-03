#include "building_levels_io.h"
#include <io/conf/conf_reader.h>
#include <util/error_codes.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

/**
 * @file    building_levels_io.cpp
 * @author  Eric Turner <elturner@eecs.berkeley.edu>
 * @brief   Reader/writer for building levels files
 *
 * @section DESCRIPTION
 *
 * This file contains the building_levels namespace, which has
 * classes used to read from and write to building levels files.
 * 
 * These files represent the horizontal partitioning of scanned buildings
 * into levels, or stories, based on floor and ceiling heights for each
 * level.
 */

using namespace std;
using namespace building_levels;

/* the following constants are used for file i/o literals */
#define MAGIC_NUMBER_TAG "levels"
#define VERSION_TAG      "version"
#define NUM_LEVELS_TAG   "num_levels"
#define NEW_LEVEL_TAG    "level"

/*--------------------------*/
/* function implementations */
/*--------------------------*/

void file_t::clear()
{
	/* reset header */
	this->header.major_version = MAJOR_VERSION;
	this->header.minor_version = MINOR_VERSION;
	this->header.num_levels    = 0; /* invalid */

	/* reset list */
	this->levels.clear();
}
			
int file_t::insert(const level_t& lev)
{
	/* check if level is valid */
	if(!(lev.is_valid()))
	{
		cerr << "[building_levels::file_t::insert]\tGiven invalid "
		     << "level!" << endl;
		return -1;
	}

	/* check if index is out-of-bounds */
	if(lev.index >= this->header.num_levels)
	{
		/* update list */
		this->header.num_levels = lev.index + 1;
		this->levels.resize(this->header.num_levels);
	}

	/* insert into list */
	this->levels[lev.index] = lev;

	/* success */
	return 0;
}
			
int file_t::parse(const std::string& filename)
{
	conf::reader_t reader;
	level_t level;
	size_t i, n;
	int ret;

	/* prepare the conf reader with valid keywords */
	reader.add_keyword(MAGIC_NUMBER_TAG, "", 0);
	reader.add_keyword(VERSION_TAG,      "", 2);
	reader.add_keyword(NUM_LEVELS_TAG,   "", 1);
	reader.add_keyword(NEW_LEVEL_TAG,    "", 3);

	/* clear any existing info */

	/* try to read the file */
	ret = reader.parse(filename);
	if(ret)
	{
		cerr << "[building_levels::file_t::parse]\tUnable to "
		     << "read levels file: " << filename << endl;
		return PROPEGATE_ERROR(-1, ret);
	}

	/* iterate through the lines */
	n = reader.size();
	if(n == 0)
	{
		cerr << "[building_levels::file_t::parse]\tFile has no "
		     << "useful info: " << filename << endl;
		return -2;
	}

	/* first, check that first line is the magic number */
	if(reader.get(0).get_keyword().compare(MAGIC_NUMBER_TAG))
	{
		cerr << "[building_levels::file_t::parse]\tFile has no "
		     << "magic number: " << filename << endl
		     << "\tfirst line keyword: " 
		     << reader.get(0).get_keyword() << endl;
		return -3;
	}

	/* read through the rest of the lines */
	for(i = 0; i < n; i++)
	{
		/* get the next line from the file */
		const conf::command_t& line = reader.get(i);
	
		/* switch based on the keyword */
		if(line.get_keyword().compare( VERSION_TAG ) == 0)
		{
			/* record the version */
			this->header.major_version 
				= line.get_arg_as<size_t>(0);
			this->header.minor_version
				= line.get_arg_as<size_t>(1);

			/* check that version makes sense */
			if(this->header.major_version != MAJOR_VERSION)
			{
				cerr << "[building_levels::file_t::parse]\t"
				     << "Error! Unsupported version "
				     << this->header.major_version << "."
				     << this->header.minor_version << endl;
				return -4;
			}
		}
		else if(line.get_keyword().compare( NUM_LEVELS_TAG ) == 0)
		{
			/* record the number of levels */
			this->header.num_levels
				= line.get_arg_as<size_t>(0);

			/* check validity */
			if(this->header.num_levels == 0)
			{
				cerr << "[building_levels::file_t::parse]\t"
				     << "Error! Input file claims no levels"
				     << endl;
				return -5;
			}
		}
		else if(line.get_keyword().compare( NEW_LEVEL_TAG ) == 0)
		{
			/* populate the level structure */
			level.index          = line.get_arg_as<size_t>(0);
			level.floor_height   = line.get_arg_as<double>(1);
			level.ceiling_height = line.get_arg_as<double>(2);

			/* check validity */
			if(!(level.is_valid()))
			{
				cerr << "[building_levels::file_t::parse]\t"
				     << "Error! Level #" << level.index
				     << " (defined on line #" << i << ") "
				     << "is invalid!" << endl;
				return -6;
			}

			/* check if level out-of-bounds */
			if(level.index >= this->header.num_levels)
			{
				cerr << "[building_levels::file_t::parse]\t"
				     << "Error!  Level out-of-bounds on "
				     << "line #" << i << "." << endl
				     << "\tIndex = " << level.index << endl
				     << "\tNum levels = " 
				     << this->header.num_levels << endl;
				return -7;
			}

			/* add the level */
			this->insert(level);
		}
		else
		{
			cerr << "[building_levels::file_t::parse]\tFeature "
			     << "not yet implemented: " 
			     << line.get_keyword() << endl;
			return -8;
		}
	}

	/* check that all levels are defined */
	for(i = 0; i < this->header.num_levels; i++)
		if(!(this->levels[i].is_valid()))
		{
			cerr << "[building_levels::file_t::parse]\tError!"
			     << "Input file has missing/invalid level #"
			     << i << endl;
			return -9;
		}

	/* success */
	return 0;
}
			
int file_t::write(const std::string& filename) const
{
	ofstream outfile;
	size_t i;

	/* first, check if we're valid */
	if(this->header.num_levels == 0)
	{
		cerr << "[building_levels::file_t::write]\tAttempted to "
		     << "write invalid level set (no levels in header, "
		     << this->levels.size() << " levels in body."
		     << endl;
		return -1;
	}
	if(this->header.num_levels != this->levels.size())
	{
		cerr << "[building_levels::file_t::write]\tInconsistent "
		     << "number of levels between header and body: "
		     << this->header.num_levels << " vs "
		     << this->levels.size() << endl;
		return -2;
	}

	/* attempt to open the file */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
	{
		cerr << "[building_levels::file_t::write]\tUnable to open "
		     << "file to write: " << filename << endl;
		return -3;
	}

	/* write some comments */
	outfile << "# File auto-generated by oct2dq program" << endl
	        << "# Written by Eric Turner " << endl
		<< "# <elturner@eecs.berkeley.edu>" << endl 
		<< "#" << endl
		<< "# Video and Image Processing Lab" << endl
		<< "# University of California Berkeley" << endl << endl;

	/* write the header */
	outfile << MAGIC_NUMBER_TAG << endl
		<< VERSION_TAG << " " << this->header.major_version
		<< " " << this->header.minor_version << endl
		<< NUM_LEVELS_TAG << " " << this->header.num_levels 
		<< endl << endl;

	/* write the body */
	for(i = 0; i < this->header.num_levels; i++)
	{
		/* check if level is in right spot */
		if(this->levels[i].index != i)
		{
			/* print warning, but continue anyway */
			cerr << "[building_levels::file_t::write]\t"
			     << "WARNING! "
			     << "Levels out of order!" << endl;
		}

		/* export */
		outfile << NEW_LEVEL_TAG << " " << i
		        << this->levels[i].floor_height << " "
			<< this->levels[i].ceiling_height << endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}
