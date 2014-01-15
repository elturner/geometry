#include "filetypes.h"
#include <stdlib.h>
#include <string.h>

filetype_t filetype_of(char* filename)
{
	unsigned int n;
	char* ext2, *ext3, *ext4;

	/* check argument */
	if(!filename)
		return unknown_file;
	
	/* get last three characters */
	n = strlen(filename);
	ext2 = (n >= 2) ? filename + n - 2 : NULL;
	ext3 = (n >= 3) ? filename + n - 3 : NULL;
	ext4 = (n >= 4) ? filename + n - 4 : NULL;

	/* check if mad */
	if(!strcmp(ext3, "mad"))
		return mad_file;

	/* check if dynamic-quadtree file */
	if(!strcmp(ext2, "dq"))
		return dq_file;

	/* check if obj */
	if(!strcmp(ext3, "obj"))
		return obj_file;

	/* check if floorplan file. */
	if(!strcmp(ext2, "fp"))
		return fp_file;
	
	/* check if ply */
	if(!strcmp(ext3, "ply"))
		return ply_file;

	/* check if idf */
	if(!strcmp(ext3, "idf"))
		return idf_file;

	/* check if voxel file */
	if(!strcmp(ext3, "vox"))
		return vox_file;

	/* check if backpack configuration file */
	if(!strcmp(ext4, "bcfg"))
		return bcfg_file;
	
	/* check if xyz */
	if(!strcmp(ext3, "xyz"))
		return xyz_file;

	/* check if edge */
	if(!strcmp(ext4, "edge"))
		return edge_file;

	/* don't know */
	return unknown_file;
}
