#include "people.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

/**
 * @file   people.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Imports .people files as part of a BIM object
 *
 * @section DESCRIPTION
 *
 * This file contains the people_t class, which is used to import
 * files of type *.people and to store their values.  These files
 * contains number of people for each room in a model.
 */

using namespace std;

/*--------------------------*/
/* function implementations */
/*--------------------------*/

int people_t::import(const std::string& filename)
{
	ifstream infile;
	stringstream ss;
	string tline;
	size_t linenum, c;

	/* clear any existing information */
	this->clear();

	/* attempt to open this file */
	infile.open(filename.c_str());
	if(!(infile.is_open()))
	{
		cerr << "[people_t::import]\tError!  Unable to open file: "
		     << filename << endl;
		return -1; /* unable to open */
	}

	/* iterate through this file */
	linenum = 0;
	while(!(infile.eof()))
	{
		/* get the next line of the file */
		std::getline(infile, tline);
		linenum++;
		if(tline.empty())
			continue; /* skip empty lines */

		/* parse the first floating point from this line
		 * as the wattage for the next room */
		ss.clear();
		ss.str(tline);
		if(!(ss >> c))
		{
			cerr << "[people_t::import]\tError!  Unable to "
			     << "parse line #" << linenum 
			     << ": \"" << tline << "\""
			     << endl;
			return -2;
		}

		/* store the value */
		this->counts.push_back(c);
	}

	/* clean up and return */
	infile.close();
	return 0;
}
