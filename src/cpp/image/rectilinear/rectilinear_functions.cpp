/*
	rectilinear_functions.cpp

	This header file provides the class for interfacting with rectilinear 
	calibration files and the needed projection functions.
*/
#include "rectilinear_functions.h"

/* includes */
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

/* namespaces */
using namespace std;

/* static variables */
const std::string rcam_model::magic_number = "KCALIB\0";

/* function definitions */
rcam_model::rcam_model() :
	_camera_name("") 
{
	for(size_t i = 0; i < 9; i++)
		_K[i] = 0;
	for(size_t i = 0; i < 5; i++)
		_kc[i] = 0;
}

bool rcam_model::read(const std::string& filename)
{

	/* open the file in binary mode */
	ifstream inStream(filename, ios_base::binary);
	if(!inStream.is_open())
		return false;

	/* read in the magic number */
	char mnumbuf[7];
	inStream.read(mnumbuf, 7);

	/* check to make sure the magic number matches */
	for(size_t i = 0; i < 7; i++)
		if(mnumbuf[i] != rcam_model::magic_number[i])
		{
			cerr << "[rcam_model::read]\tNot a valid KCALIB file"
			     << ".  The start is: \"" << string(mnumbuf)
			     << "\"" << endl;
			return false;
		}

	/* read the camera name */
	getline(inStream, _camera_name, '\0');

	/* read in the K matrix */
	inStream.read((char *)_K, 9*sizeof(double));

	/* read in the distortion cooeficents */
	inStream.read((char *)_kc, 5*sizeof(double));

	/* return success */
	return true;
}

std::string rcam_model::pretty_print() const
{
	stringstream ss;
	ss << "Camera Name : " << this->_camera_name << "\n"
	   << "K : " << _K[0] << " " << _K[1] << " " << _K[2] << "\n"
	   << "    " << _K[3] << " " << _K[4] << " " << _K[5] << "\n"
	   << "    " << _K[6] << " " << _K[7] << " " << _K[8] << "\n"
	   << "kc : " << _kc[0] << " " << _kc[1] << " " << _kc[2] << " "
	   			  << _kc[3] << " " << _kc[4];
	return ss.str();
}
