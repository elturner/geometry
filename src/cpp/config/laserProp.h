/*
*	laserProp.h
*
*	This class acts as a container for all the laser properties
*	that are passed in via the .bcfg file
*/

#ifndef _H_LASERPROP_
#define _H_LASERPROP_

/*
*	Includes
*/
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#define _USE_MATH_DEFINES
#include <math.h>

#include "sensorProp.h"

class laserProp : public sensorProp {

public:

	/********************************************
	*	Public Data Members
	********************************************/

	// This is the name given to the laser
	std::string name;

	// This is the type of scanner
	std::string type;

	// This is the serial number of the laser
	// It corrisponds to the &serialNum tag
	std::string serialNum;

	// This is the configuration file to use
	// for the laser.
	std::string configFile;

	// This is the euler angle rotations from the laser frame
	// to a common frame of reference.  It is in degrees.
	std::vector<double> rToCommon;

	// This is the translation vector between the laser frame
	// and a common frame of reference.  It is in millimeters.
	std::vector<double> tToCommon;


	/********************************************
	*	Public Member Functions
	********************************************/

	/*
		laserProp()

		Construnctor.  Sets all the values to be the unitialized versions.
	*/
	laserProp();

	/*
		void toRadianMeters()

		This function converts the values from millimeters and degrees to 
		radians and meters
	*/
	void toRadianMeters();

	/**
	 * virtual int assign_props(std::map<std::string,std::string>) =0;
	 *
  	 * This function will assign the props from the internal string map 
	 * into the internal variables of the class.  The given map is a
	 * mapping from property names to string representations of the
	 * property values.  This is the only user function that should
	 * need to be defined.
  	 *
  	 * This should return:
	 *		0 - Success
	 *		1 - Missing Property
  	 */
	 int assign_props(std::map<std::string,std::string>& property_map);

   /**
	* virtual std::string type_tag() =0 const;
	*
	* This function returns the type_tag for the particular
	* instance of the sensorProp.  This MUST match the name
	* given to the sensor type in the config XML file.
	* For example this should return "lasers" for the 
	* laser type objects.
	*/
	std::string type_tag() const;


};

#endif