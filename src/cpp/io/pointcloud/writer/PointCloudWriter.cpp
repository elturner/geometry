/*
	PointCloudWriter.cpp

	This class provides an interface for writing pointcloud files. The writing
	is based on the logic of writing a single point at a time into the 
	output file in a streaming fashion.  

	The interface provides a common means for writing all kinds of point cloud
	files with easy extensibility for adding new output types
*/
#include "PointCloudWriter.h"
#include "XYZWriter.h"
#include "OBJWriter.h"
#include "PTSWriter.h"

/* use if command line says to build with las */
#ifdef WITH_LAS_SUPPORT
	#include "LASWriter.h"
#endif

/* includes */
#include <memory>
#include <utility>

/* namespaces */
using namespace std;

/* function definitions */
PointCloudWriter PointCloudWriter::create(POINTCLOUD_FILE_TYPE file_type)
{
	/* Create the writer */
	PointCloudWriter writer;

	/* Switch on what to build */
	switch(file_type)
	{
		case XYZ :
			writer._impl = make_shared<XYZWriter>();
			break;
		case OBJ :
			writer._impl = make_shared<OBJWriter>();
			break;
		case PTS :
			writer._impl = make_shared<PTSWriter>();
			break;
#ifdef WITH_LAS_SUPPORT
		case LAS :
			writer._impl = make_shared<LASWriter>();
			std::dynamic_pointer_cast<LASWriter>(writer._impl)->compressed() 
				= false;
			break;
		case LAZ :
			writer._impl = make_shared<LASWriter>();
			std::dynamic_pointer_cast<LASWriter>(writer._impl)->compressed() 
				= true;
			break;
#endif
	}

	/* return the writer object */
	return std::move(writer);
}

PointCloudWriter PointCloudWriter::create(const std::string& file_name)
{
	PointCloudWriter writer;

	/* strip off the file extension */
	string ext;
	size_t pos = file_name.find_last_of(".");
	if(pos == string::npos)
		ext = "";
	else
		ext = file_name.substr(pos+1, string::npos);

	/* create the file writer based on known extensions */
	if(ext.compare("xyz") == 0)
		writer._impl = make_shared<XYZWriter>();
	else if(ext.compare("pts") == 0)
		writer._impl = make_shared<PTSWriter>();
	else if(ext.compare("obj") == 0)
		writer._impl = make_shared<OBJWriter>();

#ifdef WITH_LAS_SUPPORT
	else if(ext.compare("las") == 0)
	{
		writer._impl = make_shared<LASWriter>();
		std::dynamic_pointer_cast<LASWriter>(writer._impl)->compressed() 
				= false;
	}
	else if(ext.compare("laz") == 0)
	{
		writer._impl = make_shared<LASWriter>();
		std::dynamic_pointer_cast<LASWriter>(writer._impl)->compressed() 
				= true;
	}
#endif

	/* unknown file extension */
	else
		throw std::runtime_error("Unknown file extenstion \"" + ext + 
			"\" thrown from PointCloudWriter::create");

	return move(writer);
}
