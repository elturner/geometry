#ifndef H_PCDWRITER_H
#define H_PCDWRITER_H

/*
	PCDWriter.h

	This class serves as an implementation of the PointCloudWriterImpl for
	writing PCD pointcloud files.

	The ASCII PCD file format is written on the PCL website
*/

/* includes */
#include <string>
#include <fstream>
#include "PointCloudWriter.h"

/* the actual class */
class PCDWriter : public PointCloudWriterImpl
{

private:

	/* The filestream that is being written to */
	std::ofstream _outStream;

	/* This stores the number of points that have been written */
	size_t _numPointsWritten;

	/*
	*	Function that is responsible for writing the header information of the PCD
	*	file.
	*/
	bool write_header();

public:

	/*
	*	Destructor so the file can close out the header itself
	*/
	~PCDWriter();

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
	bool open(const std::string& output_file_name);

	/*
	*	The close function.
	*
	*	This function performs all the needed tasks for closing out the output
	*	stream.  
	*
	*	After this function is called the class should not accept any more 
	*	requests to write points	
	*/
	void close();

	/*
	*	Checks if the output file is open and ready to recieve points 
	*
	*	Returns true if the output file can recieve points for writing and
	*	false if it can not.
	*/
	bool is_open() const;

	/*
	*	The write point function.  
	*
	*	This is the main workhorse of the class. This point should serialize
	*	the point data into the output file.
	*
	*	Which values actually make it into the file is determined by the 
	*	actual file type.
	*/
	bool write_point(double x, double y, double z,
		unsigned char r, unsigned char g, unsigned char b,
		int index, double timestamp);

};



#endif