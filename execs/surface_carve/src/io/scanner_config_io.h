#ifndef SCANNER_CONFIG_IO_H
#define SCANNER_CONFIG_IO_H

/* scanner_config_io.h:
 *
 * This file defines functions used
 * to read *.bcfg files, which are
 * used to specify the relative
 * geometry and transformations of
 * equipment on a scanning system. */

#include <vector>
#include <string>
#include "../structs/point.h"

using namespace std;

/* the following class is used to represent
 * a laser scanner on the scanning system */
class laser_prop_t
{
	/*** parameters ***/
	public:

	/* the following fields are tags in a sensor
	 * block in the scanner system configuration
	 * file */
	string name;
	string type;
	string serialNum;

	/* The following values represent the rotation
	 * from the laser frame to the common frame
	 * of reference.  Each value is stored in
	 * radians. */
	double roll;
	double pitch;
	double yaw;

	/* the following represents the translation
	 * from the laser to the scanning system's
	 * pose frame of reference. 
	 *
	 * This value stored in meters. */
	point_t pos;

	/*** functions ***/
	
	/* constructors */
	laser_prop_t();
	~laser_prop_t() {};

	/* initialization */

	/* clear:
	 *
	 * 	Removes all information
	 *	from this structure.
	 */
	void clear();
};

/* The following class is used to represent
 * the scanning system described in the input
 * file.  Note that a scanning system is
 * a collection of scanners.  Currently,
 * we only care about the laser scanners on
 * the system. */
class scanner_config_t
{
	/*** parameters ***/
	public:

	/* This list represents all
	 * the lasers on the scanning
	 * system. */
	vector<laser_prop_t> lasers;

	/*** functions ***/
	public:

	/* constructors */
	scanner_config_t() {};
	~scanner_config_t() {};

	/* import:
	 *
	 * 	This function will import the
	 * 	configuration information stored in
	 * 	the specified file into this scanner
	 * 	configuration structure.
	 *
	 * arguments:
	 *
	 * 	filename -	The location of the
	 * 			file to parse.
	 *
	 * return value:
	 *
	 * 	Returns 0 on success, non-zero on failure.
	 */
	int import(char* filename);

	/* index_of_laser:
	 *
	 * 	Given a string, will search the string for the
	 * 	serial numbers of lasers.  If one is found,
	 * 	the index of that laser will be returned.
	 *
	 * arguments:
	 *
	 * 	str -	The string to analyze
	 *
	 * return value:
	 *
	 * 	If laser serial number found as substring of str, then
	 * 	will return index of laser.  If no serial number found,
	 * 	returns negative value.
	 */
	int index_of_laser(char* str);

	/* print:
	 *
	 * 	Prints all relevent scanner information
	 * 	to the screen.  Useful for debugging.
	 */
	void print();
};


#endif
