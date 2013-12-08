/*
*	tinyxmltools.h
*
*	This file provides some utilities to working with the TinyXML data 
*	structures.
*/

#ifndef _H_TINYXMLTOOLS_
#define _H_TINYXMLTOOLS_

#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include "tinyxml.h"

namespace TiXmlTools {

	/*
		size_t countChildNodes(TiXmlNode* nodePtr)

		This function returns the number of nodes that are direct children of
		the node pointed to by nodePtr. Comments are not counted!
	*/
	size_t countChildNodes(TiXmlNode* nodePtr);

	/*
		size_t countChildElements(TiXmlNode* nodePtr)
		static size_t countChildElements(TiXmlNode* nodePtr, 
										 const std::string& value);

		This function returns the number of element nodes that are a child of
		nodePtr.  An optional parameter can be supplied which will only count 
		elements that have a specific value.
	*/
	size_t countChildElements(TiXmlNode* nodePtr);
	size_t countChildElements(TiXmlNode* nodePtr, 
									 const std::string& value);

	/*
		size_t countChildTextElements(TiXmlNode* nodePtr)

		This function returns the number of non comment children of this node.
		If the node is NULL then zero is returned.
	*/
	size_t countChildTextElements(TiXmlNode* nodePtr);

	/*
		bool stringToVector(const std::string& inputString,
								  std::vector<T>& outputVector);
		bool stringToVector(const std::string& inputString,
								  std::vector<T>& outputVector,
								  size_t numRequiredElements);

		This function converts a string of comma or space seperated numbers
		into a vector.  Optionally a required number of 
		elements can be specified and a check will be done if exactly that
		number was recovered.
	*/
	template<typename T> bool stringToVector(const std::string& inputString,
										     std::vector<T>& outputVector) {


	// Replace all commas in the input string with spaces
	std::string extractionString = inputString;
	size_t strPos = extractionString.find(',');
	while(strPos != std::string::npos) {
		extractionString[strPos] = ' ';
		strPos = extractionString.find(',',strPos);
	}

	// Count the number of words in the string
	size_t numWords = 0;
	strPos = extractionString.find_first_not_of(" \t\n");
	while(strPos != std::string::npos) {
		numWords++;
		strPos = extractionString.find_first_of(" \t\n", strPos);
		strPos = extractionString.find_first_not_of(" \t\n", strPos);
	}
	outputVector.resize(numWords);

	// Throw the string into a stringstream so that the ASCII -> Double
	// conversion can be done
	std::stringstream extractionStream;
	extractionStream.str(extractionString);
	for(size_t i = 0; i < numWords; i++) {
		extractionStream >> outputVector[i];
	}
	return true;
}
	template<typename T> bool stringToVector(const std::string& inputString,
											 std::vector<T>& outputVector,
											 size_t numRequiredElements) {
	
	// Do the parsing of the string
	TiXmlTools::stringToVector(inputString,outputVector);
	if(outputVector.size() != numRequiredElements) {
		return false;
	}
	return true;
}

	/*
		bool stringToNumber(const std::string& inputString,
							T& outputNumber)

		This function converts a string into a single number of type T.  If 
		we are unable to extract a single number then false is returned.
	*/
	template<typename T> bool stringToNumber(const std::string& inputString,
											 T& outputNumber) {
		std::vector<T> extractionVector;
		if(!TiXmlTools::stringToVector(inputString, extractionVector, 1)) {
			return false;
		}
		outputNumber = extractionVector[0];
		return true;
	}

	
}

#endif
