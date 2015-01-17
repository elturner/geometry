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
#include "LASWriter.h"

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
	}

	/* return the writer object */
	return std::move(writer);
}

