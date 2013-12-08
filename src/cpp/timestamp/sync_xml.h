#ifndef H_SYNCXML_H
#define H_SYNCXML_H

/*
	sync_xml.h

	This class provides a reader for the sync xml
	file produced by the timesync code.
*/

/* includes */
#include <string>
#include <map>
#include <iostream>
#include <vector>

#include <xmlreader/tinyxml.h>
#include <xmlreader/tinyxmltools.h>

/* struct */
class FitParams 
{
	/* parameters */
	public:
	
		/* linear fit coefficients from sensor
		 * clock to system windows clock */
		double slope;
		double offset;

	/* functions */
	public:

		/**
		 * Converts timestamps to system clock.
		 *
		 * Converts a sensor-clock timestamp to the
		 * equivalent system windows clock timestamp.
		 *
		 * @param t   The input sensor-specific timestamp
		 *
		 * @return    Returns the corresponding windows timestamp
		 */
		inline double convert(double t) const
		{
			return ((this->slope) * t) + this->offset;
		};
		template <typename T> inline double convert(T t) const {
			return ((this->slope) * (double)t) + this->offset;
		};
		inline void convert(std::vector<double>& t) const {
			for(size_t i = 0; i < t.size(); i++) {
				t[i] *= this->slope;
				t[i] += this->offset;
			}
		};
};

/* the SyncXml class */
class SyncXml {

	/* private data members */

	// This is a bool that tells if the class is read
	bool _isRead;

	// This a copy of the filename that was used in reading
	// in the data
	std::string _filename;

	// This is the Xml
	TiXmlDocument _doc;
	
	// This is the map between the sensor names and the 
	// fit param structures
	std::map<std::string, FitParams> _data;

public:

	/* public member functions */

	/*
		SyncXml()
		SyncXml(const std::string& filename);

		Constructor.  Either build with nothing or automatically
		read in a file while constructing.  If the read fails during
		construction then isRead() will be false
	*/
	SyncXml();
	SyncXml(const std::string& filename);

	/*
		void clear()

		Clears the data stored in the class
	*/
	void clear();

	/*
		bool read(const std::string& filename);

		Reads the file provided into the class
	*/
	bool read(const std::string& filename);

	/*
		bool isRead();

		Returns a boolean that denotes if the file has been read
	*/
	inline bool isRead() {
		return _isRead;
	}

	/*
		bool isMember(const std::string& sensorName)

		Returns true if sensorName is located in the Xml and
		false if it is not.
	*/
	bool isMember(const std::string& sensorName);

	/*
		FitParams get(const std::string& sensorName);

		Returns the fit parameters for the sensor given by name.
		If this sensor does not appear then a blank copy of 
		FitParams is returned
	*/
	FitParams get(const std::string& sensorName);

	/*
		void print();

		Prints the data to standard out for visual inspection
	*/
	void print();



};


#endif
