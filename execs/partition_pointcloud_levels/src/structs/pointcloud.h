#ifndef POINTCLOUD_H
#define POINTCLOUD_H

/* pointcloud.h:
 *
 * 	This class wraps a file reader/writer,
 * 	that will read and write points to
 * 	pointcloud files.
 */

#include <fstream>
#include "scanner.h"
#include "point.h"
#include "color.h"

using namespace std;

/* the following classes are defined in this file */
class pointcloud_reader_t;
class pointcloud_writer_t;

/* a pointcloud_reader_t will parse a .xyz file file into points */
class pointcloud_reader_t
{
	/*** parameters ***/
	private:

	/* the file being read from */
	ifstream infile;

	/* the units of the points defined in the file, expressed
	 * as a conversion from meters.  The points delivered by
	 * this class will be expressed in units of meters. */
	double units;

	/* metadata */
	int num_points_read;

	/*** functions ***/
	public:

	/* constructors */
	pointcloud_reader_t();
	~pointcloud_reader_t();
	
	/* open:
	 *
	 * 	Will open the specified .xyz file for reading
	 *
	 * arguments:
	 *
	 * 	filename -	The file to read from
	 *	u -		The units of the pointcloud
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int open(const char* filename, double u);

	/* eof:
	 *
	 * 	Returns true iff at the end of file.
	 */
	inline bool eof()
	{ return this->infile.eof(); };

	/* next_point:
	 *
	 * 	Will parse the next point from the file.
	 *
	 * arguments:
	 *
	 * 	p -	Where to store the point's position
	 * 	c -	Where to store the point's color
	 *	sn -	Where to store the scan number
	 *	ts -	Where to store the timestamp
	 *	ser -	Where to store the serial number
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int next_point(point_t& p, color_t& c, 
			int& sn, double& ts, int& ser);

	/* close:
	 *
	 * 	Gracefully close the point-cloud file.
	 */
	void close();
};

/* a pointcloud_writer_t will generate a .xyz file from scans */
class pointcloud_writer_t
{
	/*** parameters ***/
	private:

	/* the file being written to */
	ofstream outfile;

	/* the units to write the output points in,
	 * in the form of a conversion from meters */
	double units;

	/* metadata about what's written */
	int num_points_written;

	/*** functions ***/
	public:
	
	/* constructors */
	pointcloud_writer_t();
	~pointcloud_writer_t();

	/* open:
	 *
	 * 	Will open the specified .xyz file for writing
	 *
	 * arguments:
	 *
	 * 	filename -	The file to write to
	 *	u -		The units to use.
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int open(const char* filename, double u);

	/* write_scan:
	 *
	 * 	Writes the specified scan to the point-cloud,
	 * 	converting it to world coordinates.
	 *
	 * arguments:
	 *
	 * 	scan -	The scan to write to file, in world coordinates
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int write_scan(scan_t& scan);

	/* write_point:
	 *
	 * 	Writes a single point to the file.  Will convert
	 * 	the point into the proper units before exporting.
	 *
	 * arguments:
	 *
	 * 	p -	The point to write
	 *	c -	The color of the point
	 *	sn -	The scan number
	 *	ts -	The timestamp
	 *	ser -	The serial number for originating scanner
	 *
	 * return value:
	 *
	 * 	Returns zero on success, non-zero on failure.
	 */
	int write_point(point_t& p, color_t& c, int sn, double ts, int ser);

	/* close:
	 *
	 * 	Gracefully close the point-cloud file.
	 */
	void close();
};

#endif
