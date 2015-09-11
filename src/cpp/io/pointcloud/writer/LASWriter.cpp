/*
	LASWriter.h

	This class serves as an implementation of the PointCloudWriterImpl for
	writing LAS/LAZ pointcloud files.

	The binary LAS/LAZ point cloud files are documented via on the website
		http://www.liblas.org/
*/
#include "LASWriter.h"

/* includes */
#include <string>
#include <fstream>
#include <ctime>
#include <iostream>
#include <liblas/liblas.hpp>
#include "PointCloudWriter.h"

/* namespaces */
using namespace std;

/* function definitions */

/*
*	Constructor
*
*	Constructs the LAS file writer using either no-compression (LAS) or
*	compression (LAZ)
*/
LASWriter::LASWriter(bool compressed) : 
	_compressed(compressed) {}

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
bool LASWriter::open(const std::string& output_file_name)
{
	/* check if the output stream is already open and clean it up if it is */
	if(this->is_open())
		this->close();

	/* open the file in binary mode before we create a liblas header */
	_outStream.open(output_file_name, ios::binary);
	if(!_outStream.is_open())
		return false;

	/* 
	*	create the liblas header 
	*/

	/* set the compression status */
	_header.SetCompressed(_compressed);

	/* set the creation date */
	time_t t = time(0);
	struct tm * now = localtime( & t );
	_header.SetCreationDOY( (uint16_t)now->tm_yday );
	_header.SetCreationYear( (uint16_t)now->tm_year + 1900 );

	/* set the software id */
	_header.SetSoftwareId("VIPLAB-Berkeley");

	/* Set the point format to include timestamp AND color */
	_header.SetDataFormatId(liblas::ePointFormat3);

	/* Set the scale to have 3 decimal places */
	_header.SetScale(0.001, 0.001, 0.001);

	/* create the actual writer */
	_writer = make_shared<liblas::Writer>(_outStream, _header);

	/* reset the point type to have the correct header */
	this->_point = make_shared<liblas::Point>(&_header);

	/* return the file status */
	return this->is_open();
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
void LASWriter::close()
{
	_writer.reset();
	_outStream.close();
}

/*
*	Checks if the output file is open and ready to recieve points 
*
*	Returns true if the output file can recieve points for writing and
*	false if it can not.
*/
bool LASWriter::is_open() const
{
	return _outStream.is_open() && (_writer != nullptr);
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
bool LASWriter::write_point(double x, double y, double z,
	unsigned char r, unsigned char g, unsigned char b,
	int index, double timestamp)
{

	/* fill the point with the correct data */
	_point->SetX(x);
	_point->SetY(y);
	_point->SetZ(z);
	_point->SetColor(liblas::Color(r,g,b));
	_point->SetTime(timestamp);

	/* write the point and return the success code of the call */
	return _writer->WritePoint(*_point);
}