#ifndef SENSORPROP_H
#define SENSORPROP_H

/**
 * @file sensorProp.h
 * @author Nicholas Corso <ncorso@eecs.berkeley.edu>
 *
 * @section DESCRIPTION
 * 
 *	This file defines an abstract
 *	class that is used to describe the sensor
 *	properties.  This class is inherited by
 *	all types of sensor property classes
 *	so that backpackConfig can store and 
 *  generate them without any knowledge
 *	of their type.
 *
 */
 
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

/* The following class should be inherited
 * by any class used to interface sensor 
 * properties with backpackConfig */
class sensorProp
{
  	/* functions */
  	public:

	  /*
	   * Template function for extracting values from strings 
	   * Extracts using a string stream 
	   */
	  template <typename T> static T extract_as(std::string& s) {
		  std::stringstream ss;
		  ss << s;
		  T extracted;
		  ss >> extracted;
		  return extracted;
	  }
	  template <typename T> static std::vector<T> extract_as_csv_vector(std::string& s, 
								size_t num_elements) {
		  std::vector<T> ret_vect;
	
		  
		  // Extract the portions from the sentance
		  std::stringstream s_stream;
		  s_stream << s;
		  std::vector<std::string> tokens;
		  std::string token;
		  while(std::getline(s_stream, token, ',')) {
			  tokens.push_back(token);
		  }

		  // Check to make sure it isnt broken
		  if(tokens.size() != num_elements) {
			  return ret_vect;
		  }
		  
		  // Extract the lements
		  for(size_t i = 0; i < num_elements; i++) {
			  std::stringstream ss;
			  ss << tokens[i];
			  T extracted;
			  ss >> extracted;
			  ret_vect.push_back(extracted);
		  }
		  
		  // Return success
		  return ret_vect;
	  }


  	  /* the following represent virtual functions that
  		 * must be implemented by inheriting subclasses */

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
  		virtual int assign_props(std::map<std::string,std::string>& property_map) =0;

		/**
		 * virtual std::string type_tag() const=0;
		 *
		 * This function returns the type_tag for the particular
		 * instance of the sensorProp.  This MUST match the name
		 * given to the sensor type in the config XML file.
		 * For example this should return "lasers" for the 
		 * laser type objects.
		 */
		 virtual std::string type_tag() const=0;
  };

#endif
