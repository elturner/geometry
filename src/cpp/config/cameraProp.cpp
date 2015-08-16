/*
*	cameraProp.cpp
*
*	This class acts as a container for all the camera properties
*	that are passed in via the .bcfg file
*/

#include "cameraProp.h"

using namespace std;

/*
	cameraProp()

	Construnctor.  Sets all the values to be the unitialized versions.
*/
cameraProp::cameraProp() {
	this->name = "";
	this->serialNum = "";
	this->virtualSerialNumber = "";
	this->configFile = "";
	this->rToCommon.clear();
	this->tToCommon.clear();
}

/*
	void toRadianMeters()

	This function converts the values from millimeters and degrees to 
	radians and meters
*/
void cameraProp::toRadianMeters() {
	for(size_t i = 0; i < this->rToCommon.size(); i++) {
		this->rToCommon[i] *= M_PI/180.0;
	}
	for(size_t i = 0; i < this->tToCommon.size(); i++) {
		this->tToCommon[i] *= 1.0/1000.0;
	}
	return;
}

/**
 * int assign_props(std::map<std::string,std::string>);
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
int cameraProp::assign_props(std::map<std::string,std::string>& property_map) {

	int ret_code = 0;
	map<string,string>::iterator it;

	// Extract the string name
	it = property_map.find("name");
	if(it == property_map.end()) {
		ret_code = 1;
	}
	else {
		this->name = it->second;
	}

	// Extract the string name
	it = property_map.find("serialNum");
	if(it == property_map.end()) {
		ret_code = 1;
	}
	else {
		this->serialNum = it->second;
	}

	// Extract the string name
	it = property_map.find("virtualSerialNum");
	if(it == property_map.end()) {
		this->virtualSerialNumber = ""; /* optional */
	}
	else {
		this->virtualSerialNumber = it->second;
	}

	// Extract the config file name
	it = property_map.find("configFile");
	if(it == property_map.end()) {
		ret_code = 1;
	}
	else {
		this->configFile = it->second;
	}
	
	// Extract the rotation to common
	it = property_map.find("rToCommon");
	if(it == property_map.end()) {
		ret_code = 1;
	}
	else {
		this->rToCommon = sensorProp::extract_as_csv_vector<double>(it->second,3);
	}

	// Extract the translation to common
	it = property_map.find("tToCommon");
	if(it == property_map.end()) {
		ret_code = 1;
	}
	else {
		this->tToCommon = sensorProp::extract_as_csv_vector<double>(it->second,3);
	}

	return ret_code;
}

/**
* virtual std::string type_tag() =0 const;
*
* This function returns the type_tag for the particular
* instance of the sensorProp.  This MUST match the name
* given to the sensor type in the config XML file.
* For example this should return "lasers" for the 
* laser type objects.
*/
std::string cameraProp::type_tag() const {
	return "cameras";
}
