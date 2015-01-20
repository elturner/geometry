/*
	PointCloudReader.h

	This class provides an interface for reading point cloud files. The reading
	is based on the logic of reading a single point at a time from the 
	input file in a streaming fashion.  

	The interface provides a common means for reading all kinds of point cloud
	files with easy extensibility for adding new input types
*/
#include "PointCloudReader.h"

/* includes */
#include <string>
#include <memory>
#include <iostream>

#include "XYZReader.h"
#include "PTSReader.h"
#include "OBJReader.h"

#ifdef WITH_LAS_SUPPORT
	#include "LASReader.h"
#endif

/* namespaces */
using namespace std;

/* function definitions */
PointCloudReader PointCloudReader::create(POINTCLOUD_FILE_TYPE file_type)
{
	PointCloudReader reader;

	/* create the correct type based on what was passed */
	switch(file_type)
	{
		case XYZ:
			reader._impl = make_shared<XYZReader>();
			break;
		case PTS:
			reader._impl = make_shared<PTSReader>();
			break;
		case OBJ:
			reader._impl = make_shared<OBJReader>();
			break;
#ifdef WITH_LAS_SUPPORT
		case LAS:
		case LAZ:
			reader._impl = make_shared<LASReader>();
			break;
#endif
	}

	/* return the reader */
	return move(reader);
}

PointCloudReader PointCloudReader::create(const std::string& file_name)
{
	PointCloudReader reader;

	/* strip off the file extension */
	string ext;
	size_t pos = file_name.find_last_of(".");
	if(pos == string::npos)
		ext = "";
	else
		ext = file_name.substr(pos+1, string::npos);

	/* create the file reader based on known extensions */
	if(ext.compare("xyz") == 0)
		reader._impl = make_shared<XYZReader>();
	else if(ext.compare("pts") == 0)
		reader._impl = make_shared<PTSReader>();
	else if(ext.compare("obj") == 0)
		reader._impl = make_shared<OBJReader>();

#ifdef WITH_LAS_SUPPORT
	else if(ext.compare("las") == 0 || ext.compare("laz") == 0)
		reader._impl = make_shared<LASReader>();
#endif

	/* unknown file extension */
	else
		throw std::runtime_error("Unknown file extenstion \"" + ext + 
			"\" thrown from PointCloudReader::create");

	return move(reader);
}

