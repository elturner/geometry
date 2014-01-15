#include "filetypes.h"
#include <stdlib.h>
#include <string.h>

filetype_t filetype_of(char* filename)
{
	unsigned int n;
	char* ext1, *ext3;

	/* check argument */
	if(!filename)
		return unknown_file;
	
	/* get last three characters */
	n = strlen(filename);
	ext1 = (n >= 2) ? filename + n - 2 : NULL;
	ext3 = (n >= 4) ? filename + n - 4 : NULL;

	/* check if mad */
	if(!strcmp(ext3, ".mad"))
		return mad_file;

	/* check if msd */
	if(!strcmp(ext3, ".msd"))
		return msd_file;

	/* check if xyz */
	if(!strcmp(ext3, ".xyz"))
		return xyz_file;

	/* check if matlab */
	if(!strcmp(ext1, ".m"))
		return m_file;

	/* don't know */
	return unknown_file;
}
