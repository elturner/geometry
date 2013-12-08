/*
*	ListFile.cpp
*	
*	This class provides a wrapper for listing all files in a directory
*	given a certain extension.  The purpose of this is to remove the reliance
*	on boost::filesystem
*
*	Author: Nicholas Corso
*/

#include "ListFiles.h"

/*
	static bool findFiles(vector<string>& foundFiles,
						  const string& inputDirectory,
						  const string& extension);

	This function finds all the files in inputDirectory with the extension
	extension.  The result is returned in the input/output parameter
	foundFiles
*/
bool ListFiles::findFiles(list<string>& foundFiles,
						  const string& inputDirectory,
						  const string& extension) {

	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;
	
	// First ensure that the inputDirectory is not longer than MAX_PATH-3
	// This is so we do not do some kind of buffer overflow
	if(inputDirectory.size() > MAX_PATH-3) {
		cerr << "ERROR: " <<  __FILE__ << endl;
		cerr << "ERROR : Input Directory Is Too Long!" << endl;
		return false;
	}

	// Force the last characters of the string to be '\\*'
	string dirToSearch = inputDirectory;
	bool appendSep;
	if(dirToSearch[dirToSearch.size()-1] != '\\') {
		dirToSearch.append("\\*");
		appendSep = true;
	}
	else {
		dirToSearch.append("*");
		appendSep = false;
	}
	
	// Copy the string to windows internal representations
	for(size_t i = 0; i < dirToSearch.size(); i++) {
		szDir[i] = dirToSearch[i];
	}
	szDir[dirToSearch.size()] = NULL;

	// Find all of the files
	hFind = FindFirstFile(szDir, &ffd);
	if(hFind == INVALID_HANDLE_VALUE) {
		
		// If there was an invalid handle just stop trying
		foundFiles.clear();
		return false;
	}

	// Iterate the files and if any of the files match the required pattern
	// then we push them to the output list
	foundFiles.clear();
	char ch[260];
    char DefChar = ' ';
	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue;
		}
		else {
			// Convert from WCHAR to CHAR so it can be passed into an
			// std::string
			WideCharToMultiByte(CP_ACP, 0, ffd.cFileName, -1, 
				                ch, 260, &DefChar, NULL);
			string foundFile(ch);
			
			size_t pos = foundFile.find_last_of('.');
			if(pos == string::npos) {
				continue;
			}
			if(extension.compare(foundFile.substr(pos+1)) != 0) {
				continue;
			}
			
			// Make the full path and push it to the output structure
			string nameToPush = inputDirectory;
			if(appendSep) {
				nameToPush.append("\\");
			}
			nameToPush = nameToPush + foundFile;
			foundFiles.push_back(nameToPush);

		}
    }
	while (FindNextFile(hFind, &ffd) != 0);
 
    // Check to make sure that we ended with no more files instead of some
    // other error
    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
		cerr << "ERROR : ListFiles ended with code: " << dwError << endl;
		FindClose(hFind);
		return false;
    }

    // Close the context and exit
	FindClose(hFind);
    return true;
}