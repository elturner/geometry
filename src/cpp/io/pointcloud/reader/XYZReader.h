#ifndef H_XYZREADER_H
#define H_XYZREADER_H

/*
	XYZReader.h

	This class serves as an implementation of the PointCloudReaderImpl for
	reading XYZ files.

	The ASCII XYZ file format is defined where each point is its own line and 
	follows the form of :

		X Y Z R G B INDEX TIMESTAMP SERIALNUMBER
*/

/* includes */
#include <string>
#include <fstream>
#include "PointCloudReader.h"

/* the actual class */
class XYZReader : public PointCloudReaderImpl
{

private:

	/* This holds the ofstream used to read from */
	std::ifstream _inStream;

public:

	/* implementation of PointCloudReaderImpl abstract functions */

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
	bool open(const std::string& input_file_name);

	/*
	*	The close function.
	*
	*	This function performs all the needed tasks for closing out the input
	*	stream.  
	*
	*	After this function is called the class should not accept any more 
	*	requests to read points	
	*/
	void close();

	/*
	*	Checks if the input file is open and ready to receive read requests 
	*
	*	Returns true if the input file can receive points for reading and
	*	false if it can not.
	*/
	bool is_open() const;

	/*
	*	Checks if a specific attribute will be validly returned by the reader
	*	
	*	Returns true if the attribute is supported and false if it is not
	*/
	bool supports_attribute(PointCloudReaderImpl::POINT_ATTRIBUTES attribute) const;

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
	bool read_point(double& x, double& y, double& z,
		unsigned char& r, unsigned char& g, unsigned char& b,
		int& index, double& timestamp);

};

#endif