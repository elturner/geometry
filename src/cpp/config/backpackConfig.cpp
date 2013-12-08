/*
*	backpackConfig.cpp
*
*	This class provides an easy method of importing and accessing the
*	hardware configuration numbers from a particular revision of the 
*	backpack.  This class expects to be initialized with an external
*	.xml file that holds the information
*/

#include "backpackConfig.h"

// Includes
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <ostream>
#include <config/sensorProp.h>
#include <xmlreader/tinyxml.h>
#include <xmlreader/tinyxmltools.h>

// Namespace
using namespace std;

/*
	backpackConfig(std::ostream& logger = std::cout);
	backpackConfig(std::string config_file_name, std::ostream& logger = std::cout);

	Constructors.  Either an empty class can be constructed or a config XML 
	can be read into the class.
*/
backpackConfig::backpackConfig(std::ostream& logger) {
	this->clear();
	this->log_stream = &logger;
}
backpackConfig::backpackConfig(std::string config_file_name,
							   std::ostream& logger) {
	this->log_stream = &logger;
	if( !this->read_config_file(config_file_name) ) {
		this->clear();
	}
}

/*
	void clear()

	Clears the class data members to the state when they were created with
	the default constructor
*/
void backpackConfig::clear() {

	// Maps from label names to pointers
	this->pointer_map.clear();
	this->instance_map.clear();

	// Set to not read state
	this->is_read = false;

}

/*
	bool read_config_file(std::string config_file_name);

	This function is the main function called to read a backpackConfig XML
	files data into the classes data structure elements
*/
bool backpackConfig::read_config_file(std::string config_file_name) {

	// The first thing we need to do is clear the class of any
	// data
	this->clear();

	// Next we need to open and parse the xml documenta
	if( !this->xml_doc.LoadFile(config_file_name.c_str()) ) {
		*(this->log_stream) << "[backpackConfig::read_config_file] - "
			<< "Unable to parse xml file." << endl;
		return false;
	}

	// If we were able to successfully parse and validate the xml
	// file for correct xml structure, then we need to validate
	// that this file 
	// 1) has the correct top level nametag
	// 2) that tag is unique
	//
	// First we check 1)
	TiXmlNode * top_level_ptr = this->xml_doc.FirstChild("sensors");
	if( top_level_ptr == NULL ) {
		*this->log_stream << "[backpackConfig::read_config_file] - "
			<< "Top level tag should be <sensors>!" << endl;
		return false;
	}
	// Check 2)
	if( top_level_ptr != this->xml_doc.LastChild("sensors") ) {
		*this->log_stream << "[backpackConfig::read_config_file] - "
			<< "<sensors> tag is multiply defined." << endl;
		return false;
	}
	
	// Next what we need to do is locate all sensor types that
	// are in this file
	TiXmlNode * sensor_type_ptr = top_level_ptr->FirstChildElement();
	while(sensor_type_ptr != NULL) {

		// Copy out the value name
		string type_name = sensor_type_ptr->Value();

		// Check if it is in the map already...  it shouldnt be.
		// If it is then the tag is multiply defined and we should
		// throw an error
		if( this->pointer_map.count(type_name) != 0 ) {
			*this->log_stream << "[backpackConfig::read_config_file] - "
			<< "<" << type_name << "> tag is multiply defined." << endl;
			return false;
		}

		// Insert it into the map
		this->pointer_map[type_name] = sensor_type_ptr;

		// Iterate to next
		sensor_type_ptr = sensor_type_ptr->NextSiblingElement();
	}

	// Next for each type of scanner we want to make a mapping between
	// the type names and the actual instances of the sensor
	map<string, TiXmlNode *>::iterator type_iterator;	
	for( type_iterator = this->pointer_map.begin(); 
		type_iterator != this->pointer_map.end(); type_iterator++) {

			// Make a copy for easy usage
			string type_name = type_iterator->first;
			sensor_type_ptr =  type_iterator->second;

			// Loop over the number of each instance and insert the
			// pointers into the instance_map
			TiXmlNode * instance_pointer = sensor_type_ptr->FirstChildElement();
			while(instance_pointer != NULL) {
				this->instance_map[type_name].push_back(instance_pointer);
				instance_pointer = instance_pointer->NextSiblingElement();
			}
	}

	// If we were able to do all of this successfully,
	// Then we can return success
	this->is_read = true;
	return true;
}



/*
	bool build_property_map(TiXmlNode * instance_ptr,
							std::map<std::string,std::string>& property_map);

	Builds the property map for a sensor instance.  Stores it in the property_map.
*/
bool backpackConfig::build_property_map(TiXmlNode * instance_ptr,
									    std::map<std::string,std::string>& property_map) {

	// Clear the map of any previous elements
	property_map.clear();

	// Extract the elements
	TiXmlNode * element_ptr = instance_ptr->FirstChildElement();
	while(element_ptr != NULL) {

		// Find element name
		string element_name = element_ptr->Value();

		// Check to see if we have a single text element underneath it
		if( TiXmlTools::countChildTextElements(element_ptr) != 1 ||
		    TiXmlTools::countChildNodes(element_ptr) != 1 )  {
			property_map.clear();
			return false;
		}

		// Find the element string
		string element_text = element_ptr->FirstChild()->Value();

		// Add to the map
		property_map[element_name] = element_text;

		// Move on
		element_ptr = element_ptr->NextSiblingElement();
	}

	

	// Return Success
	return true;
}

