/*
*	ListFile.h
*	
*	This class provides a wrapper for listing all files in a directory
*	given a certain extension.  The purpose of this is to remove the reliance
*	on boost::filesystem.  Windows Platform only.
*
*	Author: Nicholas Corso
*/

#ifndef _H_LISTFILES_
#define _H_LISTFILES_

// System Includes
#include <windows.h>
#include <tchar.h> 
#include <strsafe.h>
#include <list>
#include <string>
#include <iostream>

// Comment that this requires a call to User32.lib
#pragma comment(lib, "User32.lib")

// Namespaces
using namespace std;

// The actual class
class ListFiles {

public:

	/********************************************
	*	Public Member Function
	********************************************/

	/*
		static bool findFiles(vector<string>& foundFiles,
							  const string& inputDirectory,
							  const string& extension);

		This function finds all the files in inputDirectory with the extension
		extension.  The result is returned in the input/output parameter
		foundFiles
	*/
	static bool findFiles(list<string>& foundFiles,
						  const string& inputDirectory,
						  const string& extension);

};

#endif