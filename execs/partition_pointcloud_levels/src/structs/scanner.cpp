#include "scanner.h"
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../math/transform.h"
#include "../util/parameters.h"
#include "../util/error_codes.h"

using namespace std;

/***** SCANNER_T FUNCTIONS *****/

scanner_t::scanner_t()
{
	/* set default values */
	this->serial_number = 0;
	this->num_scans = -1;
	this->infile = NULL;
	this->num_scans_read = -1;
}

scanner_t::~scanner_t()
{
	/* make sure to close infile */
	this->close();
}

int scanner_t::open_msd(char* filename)
{
	int i, ret;

	/* check arguments */
	if(!filename)
		return -1;

	/* make sure we are not already reading a file */
	this->close();

	/* open binary file for reading */
	this->infile = fopen(filename, "rb");
	if(!(this->infile))
		return -2; /* invalid file */
	if(feof(this->infile))
	{
		fclose(this->infile);
		this->infile = NULL;
		return -3; /* empty file */
	}

	/* read in header information about this scanner */

	/* read serial number */
	ret = fread(&i, sizeof(int), 1, this->infile);
	if(ret != 1)
	{
		/* could not read serial number integer */
		fclose(this->infile);
		return -4;
	}
	this->serial_number = i;

	/* read rotation matrix */
	ret = fread(this->rot, sizeof(double), ROTATION_MATRIX_SIZE,
							this->infile);
	if(ret != ROTATION_MATRIX_SIZE)
	{
		/* could not read all of rotation matrix */
		fclose(this->infile);
		this->infile = NULL;
		return -5;
	}

	/* read translation vector */
	ret = fread(this->trans, sizeof(double), TRANSLATION_VECTOR_SIZE, 
							this->infile);
	if(ret != TRANSLATION_VECTOR_SIZE)
	{
		/* could not read all of translation vector */
		fclose(this->infile);
		this->infile = NULL;
		return -6;
	}

	/* convert trans to meters (currently in millimeters) */
	for(i = 0; i < TRANSLATION_VECTOR_SIZE; i++)
		this->trans[i] = MM2METERS(this->trans[i]);

	/* read number of scan lines */
	ret = fread(&i, sizeof(int), 1, this->infile);
	if(ret != 1)
	{
		/* could not read number of scan lines */
		fclose(this->infile);
		this->infile = NULL;
		return -7;
	}
	this->num_scans = i;
	this->num_scans_read = 0;

	/* we have successfully read the header, and are now ready to
	 * read the scans */
	return 0;
}

int scanner_t::next_scan(scan_t& scan)
{
	int ret, i, n;

	/* verify that the file to process is valid */
	if(!(this->infile))
		return -1;

	/* attempt to read a scan from this file */
	ret = scan.read_from_stream(infile);
	if(ret)
		return PROPEGATE_ERROR(-2, ret);

	/* store metadata */
	scan.scan_num = this->num_scans_read;
	scan.serial_number = this->serial_number;

	/* next, we want to perform the rigid transform of this
	 * scanner to this scan, so that the scan produced is in
	 * coordinate system of the whole scanning system. */
	n = scan.pts.size();
	for(i = 0; i < n; i++)
		affine_transform(scan.pts[i],
			this->rot, scan.pts[i], this->trans);

	/* also transform the scanner position */
	affine_transform(scan.scanner_pos,
			this->rot, scan.scanner_pos, this->trans);

	/* success */
	this->num_scans_read++;
	return 0;
}

/***** SCAN_T FUNCTIONS *****/
	
scan_t::scan_t()
{
	/* initialize to blank scan */
	this->timestamp = -1;
	this->pts.clear();
}

scan_t::~scan_t()
{
	this->timestamp = -1;
	this->pts.clear();
}

int scan_t::read_from_stream(FILE* infile)
{
	int i, j, num_pts, ret;
	double p[NUM_DIMS];

	/* attempt to read from .msd file */
	ret = fread(&num_pts, sizeof(int), 1, infile);
	if(ret != 1)
		return -1;
	ret = fread(&(this->timestamp), sizeof(double), 1, infile);
	if(ret != 1)
		return -2;

	/* verify that file is not at the end */
	if(feof(infile))
		return -3;

	/* prepare to read points */
	if(num_pts < 0)
		return -4; /* invalid */
	this->pts.resize(num_pts);
	p[2] = 0; /* z-values implied to be zero */
	
	/* read list of points from .msd file */
	for(i = 0; i < num_pts; i++)
	{
		/* verify that file is not at the end */
		if(feof(infile))
			return -5;

		/* get next point, listed as x,y */
		ret = fread(p, sizeof(double), 2, infile);
		if(ret != 2)
			return -6;

		/* convert from millimeters to meters */
		for(j = 0; j < NUM_DIMS; j++)
			p[j] = MM2METERS(p[j]);

		/* store as point in this structure */
		this->pts[i].set(p);
	}

	/* scanner position is at the origin in this coordinate system */
	p[0] = 0; p[1] = 0; p[2] = 0;
	this->scanner_pos.set(p);

	/* success */
	return 0;
}
	
void scan_t::transform_from_pose(pose_t& p)
{
	int i, n;
	double p_trans[TRANSLATION_VECTOR_SIZE];

	/* store pose location */
	p_trans[0] = p.x;
	p_trans[1] = p.y;
	p_trans[2] = p.z;

	/* iterate over points */
	n = this->pts.size();
	for(i = 0; i < n; i++)
		affine_transform(this->pts[i],p.rot,this->pts[i],p_trans);

	/* transform the scanner pos */
	affine_transform(this->scanner_pos,p.rot,this->scanner_pos,p_trans);
}
