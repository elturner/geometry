/*
*	wifiProp.cpp
*
*	This class acts as a container for all the wifi properties
*	that are passed in via the xml config file
*/

#include "wifiProp.h"

using namespace std;

/*
	wifiProp()

	Construnctor.  Sets all the values to be the unitialized versions.
*/
wifiProp::wifiProp() {
	this->name = "";
	this->configFile = "";
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
int wifiProp::assign_props(std::map<std::string,std::string>& property_map) {

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

	// Extract the config file name
	it = property_map.find("configFile");
	if(it == property_map.end()) {
		ret_code = 1;
	}
	else {
		this->configFile = it->second;
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
std::string wifiProp::type_tag() const {
	return "wifi";
};


