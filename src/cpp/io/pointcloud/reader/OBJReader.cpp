/*
	OBJReader.cpp

	This class serves as an implementation of the PointCloudReaderImpl for
	reading OBJ files.

	The ASCII OBJ file format is defined where each point is its own line and 
	follows the form of :

		v X Y Z R G B

	This reader only will pull the vertices out of the obj file.  All other 
	data is ignored.
*/
#include "OBJReader.h"

/* includes */
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "PointCloudReader.h"

/* namespaces */
using namespace std;

/* function definitions */
/*
*	The open function.
*
*	This function performs all needed tasks to get the input file ready
*	for reading.  
*
*	Returns true on success and false on error.
*
*	After this function is called, the input file should begin to accept
*	calls to the read_point function.
*/
bool OBJReader::open(const std::string& input_file_name)
{
	/* check if we area already open and then close */
	if(this->is_open())
		this->close();

	/* open the file and return the stream state as the return code */
	_inStream.open(input_file_name);
	return this->is_open();
}

/*
*	The close function.
*
*	This function performs all the needed tasks for closing out the input
*	stream.  
*
*	After this function is called the class should not accept any more 
*	requests to read points	
*/
void OBJReader::close()
{
	_inStream.close();
}

/*
*	Checks if the input file is open and ready to receive read requests 
*
*	Returns true if the input file can receive points for reading and
*	false if it can not.
*/
bool OBJReader::is_open() const
{
	return _inStream.is_open();
}

/*
*	Checks if a specific attribute will be validly returned by the reader
*	
*	Returns true if the attribute is supported and false if it is not
*/
bool OBJReader::supports_attribute(PointCloudReaderImpl::POINT_ATTRIBUTES attribute) const
{
	switch(attribute)
	{
		case POSITION:
		case COLOR:
			return true;
		case POINT_INDEX:
		case TIMESTAMP:
			return false;
	}
	return false;
}

/*
*	The read point function.  
*
*	This is the main workhorse of the class. This function should read the
*	next point from the file and return the relevant data in the passed
*	references
*
*	Which values are actually supported depend on the file type being read
*
*	This function will return true if a point was successfully read from
*	the file and false if a point is unable to be read from the file.
*/
bool OBJReader::read_point(double& x, double& y, double& z,
	unsigned char& r, unsigned char& g, unsigned char& b,
	int& index, double& timestamp)
{
	/* check if the stream can be read from */
	if(!this->is_open())
		return false;

	/* attempt to read a line */
	string line;
	size_t pos;
	while(_inStream.good() && line.empty())
	{
		getline(_inStream, line);
		if(line.empty())
			continue;
		
		/* check to make sure this is a vertex */
		pos = line.find_first_not_of(" \t");
		if(pos == string::npos)
		{
			line = "";
			continue;
		}
		if(line[pos] != 'v')
		{
			line = "";
			continue;
		}
	}

	/* we hit the end of the file without getting any lines to read */
	if(line.empty())
		return false;

	/* strip out the part of the line we care about */
	line = line.substr(pos+1, string::npos);
	
	/* extract the data from the string */
	stringstream ss(line);
	unsigned int color;
	if(!(ss >> x))
		return false;
	if(!(ss >> y))
		return false;
	if(!(ss >> z))
		return false;
	if(!(ss >> color))
		return false;
	else
		r = (unsigned char)color;
	if(!(ss >> color))
		return false;
	else
		g = (unsigned char)color;
	if(!(ss >> color))
		return false;
	else
		b = (unsigned char)color;

	/* set unread data to 0 */
	index = 0;
	timestamp = 0;

	/* return success */
	return true;
}