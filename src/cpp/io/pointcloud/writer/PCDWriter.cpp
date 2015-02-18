/*
	PCDWriter.cpp

	This class serves as an implementation of the PointCloudWriterImpl for
	writing PCD pointcloud files.

	The ASCII PCD file format is written on the PCL website
*/
#include "PCDWriter.h"

/* includes */
#include <string>
#include <fstream>
#include <iostream>

/* namespaces */
using namespace std;

/* function definitions */

/*
*	Function that is responsible for writing the header information of the PCD
*	file.
*/
bool PCDWriter::write_header()
{
	/* check if the stream is open */
	if(!_outStream.is_open())
		return false;

	/* write the version of the pcd file format that this supports */
	_outStream << "VERSION 0.7" << '\n';

	/* specify that we will have an XYZ RGB point cloud */
	_outStream << "FIELDS x y z rgb" << '\n'
			   << "SIZE 4 4 4 4" << "\n"
			   << "TYPE F F F I" << '\n'
			   << "COUNT 1 1 1 1" << "\n";

	/* the width holds a redundant copy of the number of points. we dont */
	/* know this til the end of the writers lifetime so set it to the current */
	/* count as this function is used at both creation and destruction */
	_outStream << "WIDTH " << _numPointsWritten << '\n'
			   << "HEIGHT 1" << '\n';

	/* set the viewpoint to the default value */
	_outStream << "VIEWPOINT 0 0 0 1 0 0 0" << '\n';

	/* set another redundant copy of the number of points */
	_outStream << "POINTS " << _numPointsWritten << '\n';

	/* we only support ASCII PCD */
	_outStream << "DATA ascii" << endl;

	/* return success */
	return true;
}

PCDWriter::~PCDWriter()
{
	this->close();
}

/*
*	The open function.
*
*	This function performs all needed tasks to get the output file ready
*	for writing.  
*
*	Returns true on success and false on error.
*
*	After this function is called, the output file should begin to accept
*	calls to the write_point funciton.
*/
bool PCDWriter::open(const std::string& output_file_name)
{
	/* check if the output stream is already open and clean it up if it is */
	if(this->_outStream.is_open())
		this->close();

	/* reset the number of points */
	_numPointsWritten = 0;

	/* open the file and return the open state to indicate success or failure */
	_outStream.open(output_file_name);

	/* then we need to write the header */
	if(!write_header())
		return false;

	return _outStream.is_open();
}

/*
*	The close function.
*
*	This function performs all the needed tasks for closing out the output
*	stream.  
*
*	After this function is called the class should not accept any more 
*	requests to write points	
*/
void PCDWriter::close()
{
	if(_outStream.is_open())
	{
		_outStream.seekp(0,ios::beg);
		write_header();
		_outStream.close();
	}
}

/*
*	Checks if the output file is open and ready to recieve points 
*
*	Returns true if the output file can recieve points for writing and
*	false if it can not.
*/
bool PCDWriter::is_open() const
{
	return _outStream.is_open();
}

/*
*	The write point function.  
*
*	This is the main workhorse of the class. This point should serialize
*	the point data into the output file.
*
*	Which values actually make it into the file is determined by the 
*	actual file type.
*/
bool PCDWriter::write_point(double x, double y, double z,
	unsigned char r, unsigned char g, unsigned char b,
	int index, double timestamp)
{
	index = index; /* paramter not actually used */
	
	/* pack the color a single int32 */
	int rgb = ((int)r) << 16 | ((int)g) << 8 | ((int)b);

	/* write to the file */
	_outStream << x << " " << y << " " << z << " " << rgb << "\n";
	_numPointsWritten++;

	/* return the state of the stream */
	return !(_outStream.bad() || _outStream.fail());
}
