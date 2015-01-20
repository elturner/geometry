#ifndef H_POINTCLOUDREADER_H
#define H_POINTCLOUDREADER_H

/*
	PointCloudReader.h

	This class provides an interface for reading point cloud files. The reading
	is based on the logic of reading a single point at a time from the 
	input file in a streaming fashion.  

	The interface provides a common means for reading all kinds of point cloud
	files with easy extensibility for adding new input types
*/

/* forward deceleration of the classes */
class PointCloudReaderImpl;
class PointCloudReader;

/* includes */
#include <string>
#include <memory>

/* The actual class that readers will inherit from */
class PointCloudReaderImpl
{
public:

	/*
	*	Enumeration holding the types of point attributes that can be
	*	supported by different file readers
	*/
	enum POINT_ATTRIBUTES
	{
		POSITION,
		COLOR,
		POINT_INDEX,
		TIMESTAMP
	};

	/*
	 *  The destructor is declared virtual and empty so that inherited 
	 * 	classes can have their destructors called 
	 */
	virtual ~PointCloudReaderImpl() {};

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
	virtual bool open(const std::string& input_file_name) =0;

	/*
	*	The close function.
	*
	*	This function performs all the needed tasks for closing out the input
	*	stream.  
	*
	*	After this function is called the class should not accept any more 
	*	requests to read points	
	*/
	virtual void close() =0;

	/*
	*	Checks if the input file is open and ready to receive read requests 
	*
	*	Returns true if the input file can receive points for reading and
	*	false if it can not.
	*/
	virtual bool is_open() const =0;

	/*
	*	Checks if a specific attribute will be validly returned by the reader
	*	
	*	Returns true if the attribute is supported and false if it is not
	*/
	virtual bool supports_attribute(POINT_ATTRIBUTES attribute) const =0;

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
	virtual bool read_point(double& x, double& y, double& z,
		unsigned char& r, unsigned char& g, unsigned char& b,
		int& index, double& timestamp) =0;
};


/* The interface class for PointCloudReaderImpls */
class PointCloudReader
{

public:

	/*
	*	Enumeration types for what type of files can be constructed
	*/
	enum POINTCLOUD_FILE_TYPE
	{
		XYZ,
		PTS,
		OBJ
#ifdef WITH_LAS_SUPPORT
		,
		LAS,
		LAZ
#endif
	};

	/*
	*	Enumeration holding the types of point attributes that can be
	*	supported by different file readers
	*/
	enum POINT_ATTRIBUTES
	{
		POSITION,
		COLOR,
		POINT_INDEX,
		TIMESTAMP
	};

private:

	/* 
	* 	This holds the actual implementation of the point cloud reader 
	*
	* 	This is stored as a shared_ptr which will give shallow copy semantics 
	* 	for this object 
	*/
	std::shared_ptr<PointCloudReaderImpl> _impl;

public:

	/*
	*	Constructor.  Please do not call this unless REALLY needed.  Build 
	*	from the create method
	*/
	PointCloudReader() {};

	/* 
	*	Checks if the class has a valid implementation
	*/
	inline bool is_valid() const
		{return (bool)_impl;};

	/*
	*	Creation function that generates a new PointCloudReader object 
	*	with the correct file type.
	*/
	static PointCloudReader create(POINTCLOUD_FILE_TYPE file_type);

	/*
	*	Wrapper around implementation functions
	*/

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
	inline bool open(const std::string& input_file_name)
		{return _impl->open(input_file_name); };

	/*
	*	The close function.
	*
	*	This function performs all the needed tasks for closing out the input
	*	stream.  
	*
	*	After this function is called the class should not accept any more 
	*	requests to read points	
	*/
	inline void close()
		{ _impl->close(); };

	/*
	*	Checks if the input file is open and ready to receive read requests 
	*
	*	Returns true if the input file can receive points for reading and
	*	false if it can not.
	*/
	inline bool is_open() const
		{return _impl->is_open(); };

	/*
	*	Checks if a specific attribute will be validly returned by the reader
	*	
	*	Returns true if the attribute is supported and false if it is not
	*/
	inline bool supports_attribute(POINT_ATTRIBUTES attribute) const 
	{
		switch(attribute)
		{
		case POSITION :
			return _impl->supports_attribute(PointCloudReaderImpl::POSITION);
		case COLOR :
			return _impl->supports_attribute(PointCloudReaderImpl::COLOR);
		case POINT_INDEX :
			return _impl->supports_attribute(PointCloudReaderImpl::POINT_INDEX);
		case TIMESTAMP :
			return _impl->supports_attribute(PointCloudReaderImpl::TIMESTAMP);
		}
	};

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
	inline bool read_point(double& x, double& y, double& z,
			unsigned char& r, unsigned char& g, unsigned char& b,
			int& index, double& timestamp)
		{ return _impl->read_point(x,y,z,r,g,b,index,timestamp);};

};

#endif