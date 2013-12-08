#ifndef H_XMLSETTINGS_H
#define H_XMLSETTINGS_H

/*
 *	xmlsettings.h
 *
 *	This file declares the XmlSettings class.  This class parses settings xml files
 *	and stores the data in a resulting stl map.
 *
 */

// System Level Includes
#include <map>
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>
#include <limits.h>
#include <cfloat>

// Project Level Includes

// Namespaces

/*
 *	The actual XmlSettings class
 */
class XmlSettings {

	/*
	 *	Private Data Members
	 */

	// This is a flag for if the state of the class is read or unread
	bool isRead;

	// This is the map that holds values of the map
	std::map<std::string,std::string> values;

public:

	/*
	 *	XmlSettings();
	 *	XmlSettings(const std::string& xmlSettingsFile);
	 *	XmlSettings(const std::string& xmlSettingsFile, std::ostream& os);
	 *
	 *	Constructors.
	 */
	XmlSettings();
	XmlSettings(const std::string& xmlSettingsFile, 
					std::ostream& os=std::cerr);

	/*
	 *	bool read(const std::string& xmlSettingsFile)
	 *
	 *	Reads in the contents of the xmlSettingsFile into the class.
	 *	Returns false if the read operation failed
	 *
	 *	This function clears all existing settings before reading them
	 */
	bool read(const std::string& xmlSettingsFile, 
					std::ostream& logger=std::cerr);

	/*
	 *	void clear()
	 *
	 *	Clears the class of any data
	 */
	void clear();

	/*
	 *	bool is_read() const;
	 *
	 *	Returns the state of this->isRead.  Indicates if the class has valid
	 *	data in it.
	 */
	bool is_read() const;

	/*
	 *	void print()
	 *
	 *	Prints the contents of the class to std::cout
	 */
	void print() const;

	/*
	 *	bool is_prop(const std::string& valueName) const;
	 *
	 *	Returns a boolean that denotes if a property is present in the settings
	 */
	bool is_prop(const std::string& valueName) const;

	/*
	 *	std::string get(const std::string& valueName) const;
	 *
	 *	Returns the string that represents the property denoted by valueName.
	 *	If valueName is not found in this->values, then a warning is displayed
	 *	and "" is returned.
	 */
	std::string get(const std::string& valueName) const;

	/*
	 *	int getAsInt(const std::string& valueName) const;
	 *
	 *	Returns the value of the property valueName assuming that the string
	 *	represents an integer.  Returns INT_MAX if there is an error
	 */
	int getAsInt(const std::string& valueName) const;

	/*
	 *	unsigned int getAsUint(const std::string& valueName) const;
	 *
	 *	Returns the value of the property valueName assuming that the string
	 *	represents an unsigned integer.  Returns UINT_MAX if there is an error
	 */
	unsigned int getAsUint(const std::string& valueName) const;
	
	/*
	 *	float getAsFloat(const std::string& valueName) const;
	 *
	 *	Returns the value of the property valueName assuming that the string
	 *	represents a float.  Returns FLT_MAX if there is an error
	 */
	float getAsFloat(const std::string& valueName) const;
	
	/*
	 *	double getAsDouble(const std::string& valueName) const;
	 *
	 *	Returns the value of the property valueName assuming that the string
	 *	represents a double.  Returns DBL_MAX if there is an error
	 */
	double getAsDouble(const std::string& valueName) const;
};

#endif
