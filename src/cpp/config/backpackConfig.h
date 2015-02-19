/*
*	backpackConfig.h
*
*	This class provides an easy method of importing and accessing the
*	hardware configuration numbers from a particular revision of the 
*	backpack.  This class expects to be initialized with an external
*	.xml file that holds the information
*/

#ifndef _H_BACKPACKCONFIG_
#define _H_BACKPACKCONFIG_

/*
*	Includes
*/

// System Includes
#include <vector>
#include <string>
#include <map>
#include <ostream>
#include <iostream>

#include <config/sensorProp.h>

// XML parser is included in this header file
#include <xmlreader/tinyxml.h>


// The actual config class
class backpackConfig {

	/********************************************
	*	Private Data Members
	********************************************/

	// This is an internal state flag which stores the 
	// 
	bool is_read;

	// Ostream for logging
	std::ostream * log_stream;

	// This is the TiXmlDocument that holds the 
	// xml data
	TiXmlDocument xml_doc;

	// This is a map between the sensor type strings
	// and the TiXmlNode pointers that corrispond to
	// those blocks in the XML file
	std::map<std::string, TiXmlNode *> pointer_map;

	// This is a map between the type strings and
	// a vector of TiXmlNode pointers that are
	// the instances of the scanners
	std::map<std::string, std::vector<TiXmlNode *> > instance_map;
	
	/********************************************
	*	Private Member Functions
	********************************************/

	/*
		bool build_property_map(TiXmlNode * instance_ptr,
								std::map<std::string,std::string>& property_map);

		Builds the property map for a sensor instance.  Stores it in the property_map.
	*/
	static bool build_property_map(TiXmlNode * instance_ptr,
			        			   std::map<std::string,std::string>& property_map);

public:

	/********************************************
	*	Public Member Functions
	********************************************/

	/*
		backpackConfig(std::ostream& logger = std::cout);
		backpackConfig(std::string config_file_name, std::ostream& logger = std::cout);

		Constructors.  Either an empty class can be constructed or a config XML 
		can be read into the class.
	*/
	backpackConfig(std::ostream& logger = std::cout);
	backpackConfig(std::string config_file_name, std::ostream& logger = std::cout);

	/*
		void clear()

		Clears the class data members to the state when they were created with
		the default constructor
	*/
	void clear();

	/*
		bool read_config_file(std::string config_file_name);

		This function parses the config file and reads in the data into the
		class.  Calling this function will first clear the class of any
		data by erasing the contents before reading in the new data.

		Returns true on success and false on failure
	*/
	bool read_config_file(std::string config_file_name);

	/*
		bool isRead()

		Inline function that returns the status of isRead
	*/
	inline bool isRead() const {
		return this->is_read;
	};

	/*
		template <typename T> void get_props(std::vector<T>& vector_of_props,
												bool trim_by_enable = false);

		Extracts a type of sensor props from the backpackConfig class.
		It will either return a constant reference to a vector of constant pointers
		or a copy of the properties.

		If this fails, an empty vector for the second version.
	*/
	template <typename T> void get_props(std::vector<T>& vector_of_props,
										 bool trim_by_enable = false);
	template <typename T> bool get_props(const std::string& sensor_name,
		T& out_prop,
		bool trim_by_enable = false);
};



// Template definitions

/*
	template <typename T> void get_props(std::vector<T>& vector_of_props,
									     bool trim_by_enable = false);

	Extracts a type of sensor props from the backpackConfig class.
	It will either return a constant reference to a vector of constant pointers
	or a copy of the properties.

	If this fails, an empty vector for the second version.
*/
template <typename T> void backpackConfig::get_props(
			std::vector<T>& vector_of_props,
			bool trim_by_enable) 
{

	// The first thing that we do is clear the output vector
	vector_of_props.clear();

	// Make an trash object so that we can get the type name
	T trash_object;
	std::string type_name = trash_object.type_tag();

	// Next we check if there are any type_name in our instance map
	// If we do not then just return the empty vector
	std::map<std::string, std::vector<TiXmlNode *> >::iterator it;
	it = this->instance_map.find(type_name);
	if( it == this->instance_map.end() ) {
		return;
	}

	/* If it does exist then we need to parse the blocks for 
	 * the property maps */
	for(size_t i = 0; i < it->second.size(); i++) {

		// Make a copy of the instance pointer
		TiXmlNode * instance_ptr = it->second[i];

		// Make a map to hold the mapping of the property 
		// names to the actual properties
		std::map<std::string,std::string> property_map;

		// Create the new structure
		T new_prop_struct;

		// Make the property_map
		if( ! backpackConfig::build_property_map( 
					instance_ptr, property_map) ) 
		{
			*this->log_stream 
				<< "[backpackConfig::get_props] - "
				<< "Unexpected failure building property "
				<< "map" << std::endl;
			vector_of_props.clear();
			return;
		}
		
		// Add the object to the output structure
		if( new_prop_struct.assign_props(property_map) != 0)
		{
			/* no values found for this property.  Just
			 * return an empty list */
			vector_of_props.clear();
			return;
		}

		// If required we check the enable bit
		std::map<std::string,std::string>::iterator it 
					= property_map.find("enable");
		if( it != property_map.end() ) 
		{
			bool sensor_enabled 
				= sensorProp::extract_as<bool>(it->second);
			if( !sensor_enabled && trim_by_enable) 
				continue;
		}
		
		// Pushback onto the output vector
		vector_of_props.push_back(new_prop_struct);
	}
	
}
// Get a specific property by name...
template <typename T> bool backpackConfig::get_props(const std::string& sensor_name,
	T& out_prop,
	bool trim_by_enable) {

	// Grab all the sensor of this type
	std::vector<T> props;
	this->get_props<T>(props, trim_by_enable);

	// iterate looking for name
	typename std::vector<T>::iterator it;
	for(it = props.begin(); it != props.end(); it++) {
		if( (*it).name.compare(sensor_name) == 0 ) {
			out_prop = *it;
			return true;
		}
	}

	// Return failure
	return false;

}

#endif
