/*
*	imuProp.h
*
*	This class acts as a container for all the imu properties
*	that are passed in via the config XML file
*/

#ifndef _H_IMUPROP_
#define _H_IMUPROP_

#include <vector>
#include <string>
#define _USE_MATH_DEFINES
#include <math.h>
#include "sensorProp.h"

class imuProp : public sensorProp {

public:

	/********************************************
	*	Public Data Members
	********************************************/

	// The name of the IMU
	std::string name;

	// This is the configuration file to use
	// for the camera.
	std::string configFile;
	
	// The transformation of the IMU into the common frame of reference
	// This is represented as a [dx,dy,dx] in millimeters.
	std::vector<double> tToCommon;

	// The rotational parameters of the IMU into the common frame of reference
	// This is represented as a 3-2-1 Euler Rotation [droll,dpitch,dyaw] in
	// units of degrees
	std::vector<double> rToCommon;

	/********************************************
	*	Public Member Functions
	********************************************/

	/*
		ImuProp();

		Constructor.  Initializes the class to empty 3 vectors and a blank
		string name
	*/
	imuProp();

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

	/*
		void toRadianMeters()

		This function converts the values from millimeters and degrees to 
		radians and meters
	*/
	void toRadianMeters();

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
