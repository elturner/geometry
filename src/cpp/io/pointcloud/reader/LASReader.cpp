/*
	LASReader.cpp

	This class serves as an implementation of the PointCloudReaderImpl for
	reading LAS files.

	The binary LAS/LAZ point cloud files are documented via on the website
		http://www.liblas.org/
*/
#include "LASReader.h"

/* includes */
#include <string>
#include <sstream>
#include <fstream>
#include <liblas/liblas.hpp>
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
bool LASReader::open(const std::string& input_file_name)
{
	/* check if the file is open */
	if(this->is_open())
		this->close();

	/* attempt to open the file */
	_inStream.open(input_file_name, std::ios::binary);
	if(!_inStream.is_open())
		return false;

	/* then we create the las reader from the factory */
	_reader = _readerCreationFactory.CreateWithStream(_inStream);

	/* return true because we were successful */
	return true;
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
void LASReader::close()
{
	_inStream.close();
}

/*
*	Checks if the input file is open and ready to receive read requests 
*
*	Returns true if the input file can receive points for reading and
*	false if it can not.
*/
bool LASReader::is_open() const
{
	return _inStream.is_open();
}

/*
*	Checks if a specific attribute will be validly returned by the reader
*	
*	Returns true if the attribute is supported and false if it is not
*/
bool LASReader::supports_attribute(PointCloudReaderImpl::POINT_ATTRIBUTES attribute) const
{
	switch(attribute)
	{
		case POSITION:
		case COLOR:
		case TIMESTAMP :
			return true;
		case POINT_INDEX:
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
bool LASReader::read_point(double& x, double& y, double& z,
	unsigned char& r, unsigned char& g, unsigned char& b,
	int& index, double& timestamp)
{
	/* check if the stream can be read from */
	if(!this->is_open())
		return false;

	/* Check if we can read a point */
	if(!_reader.ReadNextPoint())
	{
		x = _reader.GetPoint().GetX();
		y = _reader.GetPoint().GetX();
		z = _reader.GetPoint().GetX();
		r = (unsigned char)_reader.GetPoint().GetColor().GetRed();
		g = (unsigned char)_reader.GetPoint().GetColor().GetGreen();
		b = (unsigned char)_reader.GetPoint().GetColor().GetBlue();
		timestamp = _reader.GetPoint().GetTime();
		index = 0;
		return true;
	}

	/* failure to read a point and or the file is over */
	else
	{
		return false;
	}
}