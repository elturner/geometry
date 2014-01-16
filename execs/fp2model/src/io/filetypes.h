#ifndef FILETYPES_H
#define FILETYPES_H

/* The following defines enums
 * and functions used to determine
 * the filetype of a filename,
 * based on its extension */

typedef enum FILETYPE
{
	mad_file,
	fp_file,
	obj_file,
	windows_file,
	unknown_file
} filetype_t;

/* filetype_of:
 *
 * 	Gets the filetype of the given path.
 */
filetype_t filetype_of(char* filename);

#endif
