/*
	McdFile.cpp

	Provides a class for interfacing with mcd files
*/
#include "McdFile.h"

/* includes */
#include <string>
#include <vector>
#include <fstream>

/* namespaces */
using namespace std;

/* function definitions */
bool McdFile::read(const std::string& filename)
{
	/* open the file */
	ifstream inStream(filename.c_str());
	if(!inStream.is_open())
		return false;

	/* read in the serial number */
	inStream >> _serial_num;

	/* read in the number of images */
	inStream >> _num_images;

	/* read in the K matrix */
	for(size_t i = 0; i < 9; i++)
		inStream >> _K[i];

	/* read in the rotation matrix */
	for(size_t i = 0; i < 9; i++)
		inStream >> _r_cam_to_common[i];

	/* read in the translation vector */
	for(size_t i = 0; i < 3; i++)
		inStream >> _t_cam_to_common[i];

	/* make space for the times and file names */
	_file_names.resize(_num_images);
	_timestamps.resize(_num_images);

	/* read them in */
	for(size_t i = 0; i < _num_images; i++)
	{
		inStream >> _file_names[i];
		inStream >> _timestamps[i];
	}

	/* return success */
	return true;
}