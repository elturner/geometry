#ifndef H_MCDFILE_H
#define H_MCDFILE_H

/*
	McdFile.h

	Provides a class for interfacing with mcd files
*/

/* includes */
#include <string>
#include <vector>

/* the McdFile class */
class McdFile
{
public:

	/* constructors */
	McdFile() {};
	McdFile(const std::string& filename)
		{read(filename);};

	/* read function */
	bool read(const std::string& filename);

	/* accessors */

	/* gets the serial number */
	inline const std::string& serial_num() const
		{return _serial_num;};

	/* gets the number of images */
	inline size_t num_images() const
		{return _num_images;};

	/* gets the pointer to the K matrix */
	inline const double * const K() const
		{return _K;};

	/* gets the pointer to the rotation matrix */
	inline const double * const r_cam_to_common() const
		{return _r_cam_to_common;};

	/* gets the translation vector */
	inline const double * const t_cam_to_common() const
		{return _t_cam_to_common;};

	/* gets the ith timestamp */
	inline double timestamp(size_t i) const
		{return _timestamps[i];};

	/* gets the ith image name */
	inline const std::string& file_name(size_t i) const
		{return _file_names[i];};

private:

	/* holds the serial number of the camera */
	std::string _serial_num;

	/* holds the number of images */
	size_t _num_images;

	/* holds the K matrix */
	double _K[9];

	/* holds the camera to common rotation matrix */
	double _r_cam_to_common[9];

	/* holds the camera to common translation */
	double _t_cam_to_common[3];

	/* holds all the timestamps */
	std::vector<double> _timestamps;

	/* holds all the image file names */
	std::vector<std::string> _file_names;
};

#endif