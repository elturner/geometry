/*
 *	xmlsettings.cpp
 *
 *	This file has the definitions for the XmlSettings class
 */

// The include for the XmlSettings class
#include "xmlsettings.h"

// All other required non system includes
#include "tinyxml.h"
#include "tinyxmltools.h"

// Namespaces
using namespace std;

/*
 *	Functions definded for the XmlSettings class
 */


/*
 *	XmlSettings();
 *	XmlSettings(const std::string& xmlSettingsFile);
 *
 *	Constructors.
 */
XmlSettings::XmlSettings() {
	
	//
	// Set everything to the default state
	//
	this->isRead = false;
	this->values.clear();
}
XmlSettings::XmlSettings(const std::string& xmlSettingsFile, 
							std::ostream& os) {

	// Read the settings file
	if(!this->read(xmlSettingsFile, os)) {
		this->isRead = false;
		this->values.clear();
	}
}

/*
 *	bool read(const string& xmlSettingsFile)
 *
 *	Reads in the contents of the xmlSettingsFile into the class.
 *	Returns false if the read operation failed
 *
 *	This function clears all existing settings before reading them
 */
bool XmlSettings::read(const string& xmlSettingsFile, 
							std::ostream& logger) {
	
	//
	// Set the class to the default state
	//
	this->clear();

	//
	// The frist thing we want to do is open the xml file and parse
	// it using the tinyxml framework
	//
	TiXmlDocument xmlDoc( xmlSettingsFile.c_str() );
	if( !xmlDoc.LoadFile() ) {
		logger << "[XmlSettings::read]\tERROR : Unable to load xml file: "
		       << xmlSettingsFile << endl;
		return false;
	}
	
	//
	// Check that the first child element of the xml file is 
	// <settings> ... </settings>.
	// This is the assumed layout of the settings files
	//
	TiXmlNode * nodePtr = xmlDoc.FirstChildElement("settings");
	if( nodePtr == NULL) {
		logger << "[XmlSettings::read]\t"
		       << "ERROR : Unable to locate <settings></settings> node"
		       << endl;
		return false;	
	}

	//
	// Now we iterate over the children of the settings node.  
	// For each node that 
	//
	TiXmlNode * elementNodePtr = nodePtr->FirstChildElement();
	while( elementNodePtr != NULL ) {

		//
		// For each of the elements make sure that it has only one 
		// text node under it.
		//
		string valueName = elementNodePtr->Value();
		if(TiXmlTools::countChildTextElements(elementNodePtr) !=  1 ||
			TiXmlTools::countChildNodes(elementNodePtr) != 1) {
			logger << "[XmlSettings::read]\t"
		           << "ERROR : <%s> does not contain a single text string"
				   << valueName << endl;
			this->clear();
			return false;
		}

		//
		// Now we can push the key value pair into the map provided
		// that the key does not already exist
		//
		if( this->values.count(valueName) != 0 ) {
			logger << "[XmlSettings::read]\t"
		           << "ERROR : <%s> is multiply defined!"
				   << valueName << endl;
			this->clear();
			return false;
		} 
		else {
			this->values[valueName] = elementNodePtr->FirstChild()->Value();
		}

		elementNodePtr = elementNodePtr->NextSiblingElement();
	}
	this->isRead = true;
	return true;
}

/*
 *	void clear()
 *
 *	Clears the class of any data
 */
void XmlSettings::clear() {
	this->isRead = false;
	this->values.clear();
}

/*
 *	bool is_read() const;
 *
 *	Returns the state of this->isRead.  Indicates if the class has valid
 *	data in it.
 */
bool XmlSettings::is_read() const {
	return this->isRead;
}

/*
 *	void print()
 *
 *	Prints the contents of the class to std::cout
 */
void XmlSettings::print() const {
	map<string,string>::const_iterator it = this->values.begin();
	for(; it != this->values.end(); it++) {
		cout << "[" << (*it).first << "] : " << (*it).second << endl;
	}
}

/*
 *	bool is_prop(const std::string& valueName) const;
 *
 *	Returns a boolean that denotes if a property is present in the settings
 */
bool XmlSettings::is_prop(const std::string& valueName) const {
	map<string,string>::const_iterator it = this->values.find(valueName);
	if(it == this->values.end()) {
		return false;
	}
	else {
		return true;
	}
}

/*
 *	std::string get(const std::string& valueName)
 *
 *	Returns the string that represents the property denoted by valueName.
 *	If valueName is not found in this->values, then a warning is displayed
 *	and "" is returned.
 */
string XmlSettings::get(const string& valueName) const {
	map<string,string>::const_iterator it = this->values.find(valueName);
	if(it == this->values.end()) {
		return "";
	}
	else {
		return (*it).second;
	}
}


/*
 *	int getAsInt(const std::string& valueName) const;
 *
 *	Returns the value of the property valueName assuming that the string
 *	represents an integer
 */
int XmlSettings::getAsInt(const std::string& valueName) const {
	
	int retVal;
	stringstream s;

	map<string,string>::const_iterator it = this->values.find(valueName);
	if(it == this->values.end()) {
		return INT_MAX;
	} 
	else {
		s << (*it).second;
		s >> retVal;
		return retVal;
	}
}

/*
 *	unsigned int getAsUint(const std::string& valueName) const;
 *
 *	Returns the value of the property valueName assuming that the string
 *	represents an unsigned integer
 */
unsigned int XmlSettings::getAsUint(const std::string& valueName) const {
	
	unsigned int retVal;
	stringstream s;

	map<string,string>::const_iterator it = this->values.find(valueName);
	if(it == this->values.end()) {
		return UINT_MAX;
	} 
	else {
		s << (*it).second;
		s >> retVal;
		return retVal;
	}
}

/*
 *	float getAsFloat(const std::string& valueName) const;
 *
 *	Returns the value of the property valueName assuming that the string
 *	represents a float.
 */
float XmlSettings::getAsFloat(const std::string& valueName) const {
	
	float retVal;
	stringstream s;

	map<string,string>::const_iterator it = this->values.find(valueName);
	if(it == this->values.end()) {
		return FLT_MAX;
	} 
	else {
		s << (*it).second;
		s >> retVal;
		return retVal;
	}
}

/*
 *	double getAsDouble(const std::string& valueName) const;
 *
 *	Returns the value of the property valueName assuming that the string
 *	represents a double
 */
double XmlSettings::getAsDouble(const std::string& valueName) const {
	
	double retVal;
	stringstream s;

	map<string,string>::const_iterator it = this->values.find(valueName);
	if(it == this->values.end()) {
		return DBL_MAX;
	} 
	else {
		s << (*it).second;
		s >> retVal;
		return retVal;
	}
}
