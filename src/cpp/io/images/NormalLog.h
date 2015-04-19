#ifndef H_NORMALLOG_H
#define H_NORMALLOG_H

/*
	NormalLog.h

	Provides a class for interfacing with NormalLog files
*/

/* includes */
#include <string>
#include <vector>

/* the McdFile class */
class NormalLog
{
public:

	/* constructors */
	NormalLog() {};
	NormalLog(const std::string& filename)
		{read(filename);};

	/* read function */
	bool read(const std::string& filename);

	/* accessors */

	/* gets the name  */
	inline const std::string& name() const
		{return _name;};

	/* gets the dsFactor */
	inline double dsFactor() const
		{return _dsFactor;};

	/* gets the number of images */
	inline size_t num_images() const
		{return _num_images;};

	/* gets the pointer to the K matrix */
	inline const double * K() const
		{return _K;};

	/* gets the ith timestamp */
	inline double timestamp(size_t i) const
		{return _timestamps[i];};

	/* gets the ith image name */
	inline const std::string& file_name(size_t i) const
		{return _file_names[i];};

private:

	/* holds the name of the camera */
	std::string _name;

	/* holds the number of images */
	size_t _num_images;

	/* holds the K matrix */
	double _K[9];

	/* the scale factor */
	double _dsFactor;

	/* holds all the timestamps */
	std::vector<double> _timestamps;

	/* holds all the image file names */
	std::vector<std::string> _file_names;
};

#endif
