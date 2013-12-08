/*
*	tinyxmltools.cpp
*
*	This file provides some utilities to working with the TinyXML data 
*	structures.
*/

#include "tinyxmltools.h"

/*
	size_t countChildNodes(TiXmlNode* nodePtr)

	This function returns the number of nodes that are direct children of
	the node pointed to by nodePtr. Comments are not counted!
*/
size_t TiXmlTools::countChildNodes(TiXmlNode* nodePtr) {

	// First check that 
	if(nodePtr == NULL) {
		return 0;
	}

	// Count the number of non-comment nodes
	size_t numChildren = 0;
	TiXmlNode* childNodePtr = nodePtr->FirstChild();
	while(childNodePtr != NULL) {
		if(childNodePtr->Type() != TiXmlNode::TINYXML_COMMENT) {
			numChildren++;
		}
		childNodePtr = childNodePtr->NextSibling();
	}

	return numChildren;
}

/*
	size_t countChildElements(TiXmlNode* nodePtr)

	This function returns the number of non comment children of this node.
	If the node is NULL then zero is returned.
*/
size_t TiXmlTools::countChildElements(TiXmlNode* nodePtr) {

	// First check that the node isnt a NULL pointer
	if(nodePtr == NULL) {
		return 0;
	}

	// Iterate over the children counting the elements
	size_t numChildren = 0;
	TiXmlNode* childNodePtr = nodePtr->FirstChildElement();
	while(childNodePtr != NULL) {
		numChildren++;
		childNodePtr = childNodePtr->NextSiblingElement();
	}

	return numChildren;
}
size_t TiXmlTools::countChildElements(TiXmlNode* nodePtr,
										  const std::string& value) {

	// First check that the node isnt a NULL pointer
	if(nodePtr == NULL) {
		return 0;
	}

	// Iterate over the children counting the elements
	size_t numChildren = 0;
	TiXmlNode* childNodePtr = nodePtr->FirstChildElement(value.c_str());
	while(childNodePtr != NULL) {
		numChildren++;
		childNodePtr = childNodePtr->NextSiblingElement(value.c_str());
	}

	return numChildren;
}

/*
	size_t countChildTextElements(TiXmlNode* nodePtr)

	This function returns the number of non comment children of this node.
	If the node is NULL then zero is returned.
*/
size_t TiXmlTools::countChildTextElements(TiXmlNode* nodePtr) {
	
	// First check that nodePtr is not null
	if(nodePtr == NULL) {
		return 0;
	}

	size_t numTextChildren = 0;
	TiXmlNode* childNodePtr = nodePtr->FirstChild();
	while(childNodePtr != NULL) {
		if(childNodePtr->Type() == TiXmlNode::TINYXML_TEXT) {
			numTextChildren++;
		}
		childNodePtr = childNodePtr->NextSibling();
	}
	return numTextChildren; 
}
