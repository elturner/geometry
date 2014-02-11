#include "filetypes.h"
#include <stdlib.h>
#include <string.h>

/* helper function */
bool endswith(const char* haystack, const char* needle);

filetype_t filetype_of(char* filename)
{
	/* check argument */
	if(!filename)
		return unknown_file;
	
	/* check if fp */
	if(endswith(filename, ".fp"))
		return fp_file;

	/* check if obj */
	if(endswith(filename, ".obj"))
		return obj_file;

	/* check if wrl */
	if(endswith(filename, ".wrl"))
		return wrl_file;

	/* check if mad */
	if(endswith(filename, ".mad"))
		return mad_file;

	/* check if windows */
	if(endswith(filename, ".windows"))
		return windows_file;

	/* don't know */
	return unknown_file;
}

/* endswith:
 *
 * 	Returns true iff haystack ends
 * 	with needle.
 */
bool endswith(const char* haystack, const char* needle)
{
	int nh, nn;

	/* get lengths */
	nh = strlen(haystack);
	nn = strlen(needle);

	/* check edge cases */
	if(nn == 0)
		return true;
	if(nh < nn)
		return false;

	/* compare strings */
	return !strcmp(haystack + nh - nn, needle);
}
