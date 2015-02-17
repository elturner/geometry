#ifndef H_POINTCLOUD_WRITIER_H
#define H_POINTCLOUD_WRITIER_H

/*
	PointCloudWriter.h

	This class provides an interface for writing pointcloud files. The writing
	is based on the logic of writing a single point at a time into the 
	output file in a streaming fashion.  

	The interface provides a common means for writing all kinds of point cloud
	files with easy extensibility for adding new output types
*/

/* forward deceleration of the classes */
class PointCloudWriterImpl;
class PointCloudWriter;

/* includes */
#include <string>
#include <memory>

/* The actual class that writers will inherit from */
class PointCloudWriterImpl
{
public:

	/*
	 *  The destructor is declared virtual and empty so that inherited 
	 * 	classes can have their destructors called 
	 */
	virtual ~PointCloudWriterImpl() {};

	/*
	*	The open function.
	*
	*	This function performs all needed tasks to get the output file ready
	*	for writing.  
	*
	*	Returns true on success and false on error.
	*
	*	After this function is called, the output file should begin to accept
	*	calls to the write_point function.
	*/
	virtual bool open(const std::string& output_file_name) =0;

	/*
	*	The close function.
	*
	*	This function performs all the needed tasks for closing out the output
	*	stream.  
	*
	*	After this function is called the class should not accept any more 
	*	requests to write points	
	*/
	virtual void close() =0;

	/*
	*	Checks if the output file is open and ready to receive points 
	*
	*	Returns true if the output file can receive points for writing and
	*	false if it can not.
	*/
	virtual bool is_open() const =0;

	/*
	*	The write point function.  
	*
	*	This is the main workhorse of the class. This function should serialize
	*	the point data into the output file.
	*
	*	Which values actually make it into the file is determined by the 
	*	actual file type.
	*/
	virtual bool write_point(double x, double y, double z,
		unsigned char r, unsigned char g, unsigned char b,
		int index, double timestamp) =0;
};

/* The interface class for PointCloudWriterImpls */
class PointCloudWriter
{

public:

	/*
	*	Enumeration types for what type of files can be constructed
	*/
	enum POINTCLOUD_FILE_TYPE
	{
		XYZ,
		PTS,
		OBJ,
		PCD
#ifdef WITH_LAS_SUPPORT
		,
		LAS,
		LAZ
#endif
	};

private:

	/* 
	* 	This holds the actual implementation of the pointcloud writer 
	*
	* 	This is stored as a shared_ptr which will give shallow copy semantics 
	* 	for this object 
	*/
	std::shared_ptr<PointCloudWriterImpl> _impl;

public:

	/*
	*	Constructor.  Please do not call this unless REALLY needed.  Build 
	*	from the create method
	*/
	PointCloudWriter() {};

	/* 
	*	Checks if the class has a valid implementation
	*/
	inline bool is_valid() const
		{return (bool)_impl;};

	/*
	*	Creation function that generates a new PointCloudWriter object 
	*	with the correct file type.
	*/
	static PointCloudWriter create(POINTCLOUD_FILE_TYPE file_type);
	static PointCloudWriter create(const std::string& file_name);

	/*
	*	Wrapper around implementation functions
	*/

	/*
	*	The open function.
	*
	*	This function performs all needed tasks to get the output file ready
	*	for writing.  
	*
	*	Returns true on success and false on error.
	*
	*	After this function is called, the output file should begin to accept
	*	calls to the write_point function.
	*/
	inline bool open(const std::string& output_file_name)
		{return _impl->open(output_file_name); };

	/*
	*	The close function.
	*
	*	This function performs all the needed tasks for closing out the output
	*	stream.  
	*
	*	After this function is called the class should not accept any more 
	*	requests to write points	
	*/
	inline void close()
		{ _impl->close(); };

	/*
	*	Checks if the output file is open and ready to receive points 
	*
	*	Returns true if the output file can receive points for writing and
	*	false if it can not.
	*/
	inline bool is_open() const
		{return _impl->is_open(); };

	/*
	*	The write point function.  
	*
	*	This is the main workhorse of the class. This point should serialize
	*	the point data into the output file.
	*
	*	Which values actually make it into the file is determined by the 
	*	actual file type.
	*/
	inline bool write_point(double x, double y, double z,
			unsigned char r, unsigned char g, unsigned char b,
			int index, double timestamp)
		{ return _impl->write_point(x,y,z,r,g,b,index,timestamp);};

};

#endif