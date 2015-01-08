#ifndef H_RECTILINEARFUNCTIONS_H
#define H_RECTILINEARFUNCTIONS_H

/*
	rectilinear_functions.h

	This header file provides the class for interfacting with rectilinear 
	calibration files and the needed projection functions.
*/

/* includes */
#include <string>

/* the rcam_model class */
class rcam_model
{

	// This holds the camera name
	std::string _camera_name;

	// This holds the K matrix of the camera in ROW MAJOR ordering
	double _K[9];

	// This holds the linear distortion coefficents
	double _kc[5];

public:

	// Static variable that specifies the magic number
	const static std::string magic_number;

	/*
	*	rcam_model()
	*
	*	Default constuctor for the class.  
	*/
	rcam_model();

	/*
	*	bool read(const std::string& filename);
	*
	*	Parses the given calibration file and loads it into the class.
	*	Returns true on success and false on failure
	*/
	bool read(const std::string& filename);

	/*
	*	std::string pretty_print() const
	*
	*	Serializes the class data into a nice to display string
	*/
	std::string pretty_print() const;

	/*
	*	void project_into_image(double* pt3d, double* ptImg) const
	*
	*	Projects a 3d point in camera coordinates into the image
	*
	*	pt3d should be at least a 3 element double array.
	*	pt2d should be at least a 2 element double array.
	*
	*	-> Camera Coordinates are defined such that +z is into the image and
	*	   +x to the right, and +y is downward.
	*
	*	-> Image coordinates are defined such that (0,0) is in the upper left
	*	   corner of the image and +x is to the right and +y is down
	*/
	inline void project_into_image(double *pt3d, double* ptImg) const
	{
		ptImg[0] = _K[2] + (_K[0]*pt3d[0])/pt3d[2] + (_K[1]*pt3d[1])/pt3d[2];
 		ptImg[1] = _K[5] + (_K[3]*pt3d[0])/pt3d[2] + (_K[4]*pt3d[1])/pt3d[2];
	};

	/*
	* 	Getters/Setters
	*/
	inline const std::string& camera_name() const
		{return _camera_name;};
	inline std::string& camera_name() 
		{return _camera_name;};
	inline const double& K(size_t i, size_t j) const
		{return _K[3*i+j];};
	inline double& K(size_t i, size_t j) 
		{return _K[3*i+j];};
	inline const double& kc(size_t i) const
		{return _kc[i];};
	inline double& kc(size_t i)
		{return _kc[i];};

};

#endif