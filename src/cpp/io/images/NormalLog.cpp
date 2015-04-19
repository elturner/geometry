/*
	NormalLog.cpp

	Provides a class for interfacing with NormalLog files
*/
#include "NormalLog.h"

/* includes */
#include <string>
#include <vector>
#include <fstream>

/* namespaces */
using namespace std;

/* function definitions */
bool NormalLog::read(const std::string& filename)
{
	/* open the file */
	ifstream inStream(filename.c_str());
	if(!inStream.is_open())
		return false;

	/* read in the serial number */
	inStream >> _name;

	/* read in the number of images */
	inStream >> _num_images;

	/* read in the K matrix */
	for(size_t i = 0; i < 9; i++)
		inStream >> _K[i];

	/* read in the ds factor */
	inStream >> _dsFactor;

	/* make space for the times and file names */
	_file_names.resize(_num_images);
	_timestamps.resize(_num_images);

	/* read them in */
	for(size_t i = 0; i < _num_images; i++)
	{
		inStream >> _timestamps[i];
		inStream >> _file_names[i];
	}

	/* return success */
	return true;
}