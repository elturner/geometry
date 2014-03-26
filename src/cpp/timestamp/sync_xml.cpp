/*
	sync_xml.cpp

	This class provides a reader for the sync xml
	file produced by the timesync code.
*/

#include "sync_xml.h"

/* includes */
#include <string>
#include <map>
#include <iostream>

#include <xmlreader/tinyxml.h>
#include <xmlreader/tinyxmltools.h>

/* namespaces */
using namespace std;

/* function implementations */

/*
	SyncXml()

	Basic Constructor
*/
SyncXml::SyncXml() {
	_isRead = false;
	_filename = "";
}
SyncXml::SyncXml(const std::string& filename) {
	this->_filename = filename;
	if (!this->read(_filename)) {
		this->clear();
	}
}

/*
	void clear()

	Clears the data stored in the class
*/
void SyncXml::clear() {
	_isRead = false;
	_filename = "";
	_doc.Clear();
	_data.clear();
}

/*
	bool read(const std::string& filename);

	Reads the file provided into the class
*/
bool SyncXml::read(const std::string& filename)
{

	// Copy the filename into the class data
	_filename = filename;
	
	// Read in the xml file into the class data
	if (!_doc.LoadFile(_filename.c_str()))
	{
		cerr << "[SyncXml::read] - Error loading xml document : "
			<< _filename << endl;
		this->clear();
		return false;
	}

	// Check the top level node ptr to see if it says
	// exactly "timesync"
	TiXmlNode * node_ptr = _doc.FirstChildElement();
	if (node_ptr == NULL)
	{
		cerr << "[SyncXml::read] - Head node was NULL. "
		     << "Is the file empty?" << endl;
		this->clear();
		return false;
	}
	string headTag = node_ptr->Value();
	if (headTag.compare("timesync") != 0)
	{
		cerr << "[SyncXml::read] - Head node is <" << headTag
			<< "> instead of expected <timestamp>" << endl;
		this->clear();
		return false;
	}

	// For each of the children of the head node create an entry
	// into the data map
	FitParams fitParams;

	node_ptr = node_ptr->FirstChildElement();
	while (node_ptr != NULL)
	{
		// Get the scale
		TiXmlNode * slope_ptr 
			= node_ptr->FirstChildElement("slope");
		if (slope_ptr == NULL)
		{
			cerr << "[SyncXml::read] - Missing <scale> node "
			     << "in block: " 
			     << "<" << node_ptr->Value() << ">" << endl;
			this->clear();
			return false;
		}
		if (TiXmlTools::countChildTextElements(slope_ptr) != 1)
		{
			cerr << "[SyncXml::read] - Malformed <scale> node"
			     << " in block: "
			     << "<" << node_ptr->Value() << ">" << endl;
			this->clear();
			return false;
		}
		TiXmlTools::stringToNumber<double>(
				slope_ptr->FirstChild()->Value(),
				fitParams.slope);

		// Get the offset
		TiXmlNode * offset_ptr 
			= node_ptr->FirstChildElement("offset");
		if (offset_ptr == NULL) {
			cerr << "[SyncXml::read] - Missing <offset> node "
			     << "in block: "
			     << "<" << node_ptr->Value() << ">" << endl;
			this->clear();
			return false;
		}
		if (TiXmlTools::countChildTextElements(offset_ptr) != 1)
		{
			cerr << "[SyncXml::read] - Malformed <offset> "
			     << "node in block: "
			     << "<" << node_ptr->Value() << ">" << endl;
			this->clear();
			return false;
		}
		TiXmlTools::stringToNumber<double>(
				offset_ptr->FirstChild()->Value(),
				fitParams.offset);

		// Get the std. dev., if it is available
		TiXmlNode* stddev_ptr 
			= node_ptr->FirstChildElement("stddev");
		if(stddev_ptr == NULL 
			|| TiXmlTools::countChildTextElements(
				stddev_ptr) != 1)
		{
			/* set 'invalid' value for stddev */
			fitParams.stddev = -1;
		}
		else
		{
			// get the value
			TiXmlTools::stringToNumber<double>(
				stddev_ptr->FirstChild()->Value(),
				fitParams.stddev);
		}

		// Insert into data map
		_data[node_ptr->Value()] = fitParams;

		// Go to next ptr
		node_ptr = node_ptr->NextSiblingElement();
	}

	// Flag that we succeeded and return
	_isRead = true;
	return this->isRead();
}

/*
	bool isMember(const std::string& sensorName)

	Returns true if sensorName is located in the Xml and
	false if it is not.
*/
bool SyncXml::isMember(const std::string& sensorName) {
	return (_data.count(sensorName) != 0);
}

/*
	FitParams get(const std::string& sensorName);

	Returns the fit parameters for the sensor given by name.
	If this sensor does not appear then a blank copy of
	FitParams is returned
*/
FitParams SyncXml::get(const std::string& sensorName) const {

	map<string, FitParams>::const_iterator it =
		_data.find(sensorName);
	if (it == _data.end())
	{
		/* set default invalid values */
		FitParams blank;
		blank.offset = blank.slope = 0;
		blank.stddev = -1;
		return blank;
	}
	else
	{
		/* return the found data */
		return it->second;
	}
}

/*
	void print();

	Prints the data to standard out for visual inspection
*/
void SyncXml::print() {

	map<string, FitParams>::iterator it;
	for (it = _data.begin(); it != _data.end(); it++) {
		cout << it->first << ": "
			<< "slope - " << it->second.slope << " "
			<< "offset - " << it->second.offset << endl;
	}

}
