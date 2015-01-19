/*
	XYZReader.cpp

	This class serves as an implementation of the PointCloudReaderImpl for
	reading XYZ files.

	The ASCII XYZ file format is defined where each point is its own line and 
	follows the form of :

		X Y Z R G B INDEX TIMESTAMP SERIALNUMBER
*/
#include "XYZReader.h"

/* includes */
#include <string>
#include <sstream>
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
bool XYZReader::open(const std::string& input_file_name)
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
void XYZReader::close()
{
	_inStream.close();
}

/*
*	Checks if the input file is open and ready to receive read requests 
*
*	Returns true if the input file can receive points for reading and
*	false if it can not.
*/
bool XYZReader::is_open() const
{
	return _inStream.is_open();
}

/*
*	Checks if a specific attribute will be validly returned by the reader
*	
*	Returns true if the attribute is supported and false if it is not
*/
bool XYZReader::supports_attribute(PointCloudReaderImpl::POINT_ATTRIBUTES attribute) const
{
	switch(attribute)
	{
		case POSITION:
		case COLOR:
		case POINT_INDEX:
		case TIMESTAMP:
			return true;
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
bool XYZReader::read_point(double& x, double& y, double& z,
	unsigned char& r, unsigned char& g, unsigned char& b,
	int& index, double& timestamp)
{
	/* check if the stream can be read from */
	if(!this->is_open())
		return false;

	/* attempt to read a line */
	string line;
	while(_inStream.good() && line.empty())
	{
		getline(_inStream, line);
		if(line.empty())
			continue;
	}
	
	/* we hit the end of the file without getting any lines to read */
	if(line.empty())
		return false;

	
	/* extract the data from the string */
	stringstream ss(line);
	if(!(ss >> x))
		return false;
	if(!(ss >> y))
		return false;
	if(!(ss >> z))
		return false;
	if(!(ss >> r))
		return false;
	if(!(ss >> g))
		return false;
	if(!(ss >> b))
		return false;
	if(!(ss >> index))
		return false;
	if(!(ss >> timestamp))
		return false;
	/* dont care about serial number */

	/* return success */
	return true;
}