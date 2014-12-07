#include "lights.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

/**
 * @file   lights.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Imports .lights files as part of a BIM object
 *
 * @section DESCRIPTION
 *
 * This file contains the lights_t class, which is used to import
 * files of type *.lights and to store their values.  These files
 * contain wattages for the ceiling lights for each room in a model.
 */

using namespace std;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int lights_t::import(const std::string& filename)
{
	ifstream infile;
	stringstream ss;
	string tline;
	double w;

	/* clear any existing information */
	this->clear();

	/* attempt to open this file */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
	{
		cerr << "[lights_t::import]\tError!  Unable to open file: "
		     << filename << endl;
		return -1; /* unable to open */
	}

	/* iterate through this file */
	while(!(infile.eof()))
	{
		/* get the next line of the file */
		std::getline(infile, tline);
		if(tline.empty())
			continue; /* skip empty lines */

		/* parse the first floating point from this line
		 * as the wattage for the next room */
		ss.clear();
		ss.str(tline);
		if(!(ss >> w))
		{
			cerr << "[lights_t::import]\tError!  Unable to "
			     << "parse line: \"" << tline << "\""
			     << endl;
			return -2;
		}

		/* store the value */
		this->wattages.push_back(w);
	}

	/* clean up and return */
	infile.close();
	return 0;
}
