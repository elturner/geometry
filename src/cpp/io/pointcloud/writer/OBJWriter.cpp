/*
	OBJWriter.h

	This class serves as an implementation of the PointCloudWriterImpl for
	writing OBJ pointcloud files.

	The ASCII OBJ file format is defined where each point is its own line and 
	follows the form of :

		v X Y Z R G B
*/
#include "OBJWriter.h"

/* includes */
#include <string>
#include <fstream>

/* namespaces */
using namespace std;

/* function definitions */
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
bool OBJWriter::open(const std::string& output_file_name)
{
	/* check if the output stream is already open and clean it up if it is */
	if(this->_outStream.is_open())
		this->close();

	/* open the file and return the open state to indicate success or failure */
	_outStream.open(output_file_name);
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
void OBJWriter::close()
{
	_outStream.close();
}

/*
*	Checks if the output file is open and ready to recieve points 
*
*	Returns true if the output file can recieve points for writing and
*	false if it can not.
*/
bool OBJWriter::is_open() const
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
bool OBJWriter::write_point(double x, double y, double z,
	unsigned char r, unsigned char g, unsigned char b,
	int index, double timestamp)
{

	/* silence compiler warnings */
	(void)index; (void)timestamp;

	/* Write the data to file */
	_outStream << "v " 
			   << x << " "
			   << y << " "
			   << z << " "
			   << (unsigned int)r << " "
			   << (unsigned int)g << " "
			   << (unsigned int)b << "\n";

	/* return the state of the stream */
	return !(_outStream.bad() || _outStream.fail());
}