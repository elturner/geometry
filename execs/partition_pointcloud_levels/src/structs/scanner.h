#ifndef SCANNER_H
#define SCANNER_H

/* scanner.h:
 *
 * 	This file contains classes that represent
 * 	the information seen by one laser scanner
 * 	over time.  The scanner will have a rigid
 * 	transformation with respect to the system
 * 	origin at each time step.
 */

#include <vector>
#include <string>
#include <stdio.h>
#include "point.h"
#include "path.h"
#include "../math/transform.h"

using namespace std;

/* the classes defined in this file */
class scanner_t;
class scan_t;

/* the scanner class, which stores scans over time */
class scanner_t
{
	/*** properties ***/
	private:

	/* the unique serial number for this scanner */
	int serial_number;

	/* the rotation and translation of this scanner with
	 * respect to system origin
	 *
	 *	p_global = rot * p_local + trans;
	 */
	double rot[ROTATION_MATRIX_SIZE]; /* row-major order */
	double trans[TRANSLATION_VECTOR_SIZE]; /* units: meters */

	/* meta data about scans */
	int num_scans; /* total number of scans during data collect */
	FILE* infile; /* the msd file pointer of this scanner */
	int num_scans_read; /* how far are we through the file? */

	/*** functions ***/
	public:

	/* constructors */
	scanner_t();
	~scanner_t();

	/* initialization */

	/* open_msd:
	 *
	 * 	Will initialize this scanner from the
	 * 	specified .msd file.
	 *
	 * 	Once this function is called, scans can be read
	 * 	off from the file, using the next_scan() function.
	 *
	 * arguments:
	 *
	 * 	filename -	The file to parse
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int open_msd(char* filename);

	/* next_scan:
	 *
	 * 	Reads the next scan from the opened .msd file.
	 *
	 * arguments:
	 *
	 * 	scan -	Where to store the parsed scan.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int next_scan(scan_t& scan);

	/* eof:
	 *
	 * 	Returns true iff a valid file is being parsed and
	 * 	the end has been reached.
	 */
	inline bool eof() 
	{ 
		if(!(this->infile))
			return true;

		return (this->num_scans_read >= this->num_scans)
			|| feof(this->infile);
	};

	/* amount_read:
	 *
	 * 	Returns the fraction of the file that has been read.
	 */
	inline double amount_read()
	{ return (1.0 * this->num_scans_read) / (this->num_scans); };

	/* close:
	 *
	 * 	Will close any open files for this scanner
	 */
	inline void close()
	{
		if(this->infile)
		{
			fclose(this->infile);
			this->infile = NULL;
		}
	};
};

class scan_t
{
	/* allow the scanner class to modify these fields */
	friend class scanner_t;
	
	/*** parameters ***/
	private:

	/* metadata about the scan */
	int scan_num; /* scan number in its file */
	double timestamp; /* timestamp of this scan */
	int serial_number; /* serial num of originating scanner */

	public:

	/* the points of this scan */
	vector<point_t> pts;

	/* the position of the scanner */
	point_t scanner_pos;

	/*** functions ***/
	public:

	/* constructors */
	scan_t();
	~scan_t();

	/* read_from_stream:
	 *
	 * 	Will populate this struct
	 * 	with the next lines from
	 * 	a .msd file.
	 *
	 * arguments:
	 *
	 * 	infile -	The filestream
	 * 			to read from.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int read_from_stream(FILE* infile);

	/* accessors */

	inline int get_scan_num()      { return scan_num; };
	inline double get_timestamp()  { return timestamp; };
	inline int get_serial_number() { return serial_number; };

	/* transforms */

	/* transform_from_pose:
	 *
	 * 	Converts from system to world coordinates, given
	 * 	the appropriate pose.
	 */
	void transform_from_pose(pose_t& p);
};

#endif
