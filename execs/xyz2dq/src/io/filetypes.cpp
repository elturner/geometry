#include "filetypes.h"
#include <stdlib.h>
#include <string.h>

filetype_t filetype_of(char* filename)
{
	unsigned int n;
	char* ext3, *ext2;

	/* check argument */
	if(!filename)
		return unknown_file;
	
	/* get last three characters */
	n = strlen(filename);
	ext3 = (n >= 3) ? filename + n - 3 : NULL;
	ext2 = (n >= 2) ? filename + n - 2 : NULL;

	/* check if xyz */
	if(!strcmp(ext3, "xyz") || !strcmp(ext3, "txt"))
		return xyz_file;
	
	/* check if mad */
	if(!strcmp(ext3, "mad"))
		return mad_file;

	/* check if dq file */
	if(!strcmp(ext2, "dq"))
		return dq_file;

	/* don't know */
	return unknown_file;
}
