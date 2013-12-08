/*
*	magnetometerProp.h
*
*	This class acts as a container for all the magnetometer sensors
*	properties that are passed in via xml configuration files
*/

#ifndef _H_MAGNETOMETERPROP_
#define _H_MAGNETOMETERPROP_

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

class magnetometerProp : public sensorProp {

public:

	/********************************************
	*	Public Data Members
	********************************************/

	// This is the name given to the gps sensor
	std::string name;

	// This is the configuration file to use
	// for the gps sensor.
	std::string configFile;

	// This is the name of the microcontorller
	// that the sensor is connected through
	std::string ucontroller;


	/********************************************
	*	Public Member Functions
	********************************************/

	/*
		magnetometerProp()

		Construnctor.  Sets all the values to be the unitialized versions.
	*/
	magnetometerProp();

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