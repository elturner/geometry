#include "pointcloud.h"
#include <fstream>
#include <string>
#include <string.h>
#include "../structs/point.h"
#include "../structs/scanner.h"
#include "../util/error_codes.h"

/********************* POINTCLOUD READER ***********************/

pointcloud_reader_t::pointcloud_reader_t()
{

}

pointcloud_reader_t::~pointcloud_reader_t()
{
	this->close();
}

int pointcloud_reader_t::open(const char* filename, double u)
{
	/* check if a file is already open */
	if(this->infile.is_open())
		this->infile.close();

	/* attempt to open a new ascii file for reading */
	this->infile.open(filename);
	if(!(this->infile.is_open()))
		return -1;

	/* record desired units */
	this->units = u;
	
	/* reset counter */
	this->num_points_read = 0;

	/* success */
	return 0;
}

int pointcloud_reader_t::next_point(point_t& p, color_t& c, 
				int& sn, double& ts, int& ser)
{
	string line;
	int ret;
	double x,y,z;
	int r,g,b;

	/* first, verify file is open */
	if(!(this->infile.is_open()))
		return -1;

	ret = 0;
	while(ret == 0 && !(this->infile.eof())) /* ignore blank lines */
	{
		/* read next line from file */
		getline(this->infile, line);

		/* parse line */
		ret = sscanf(line.c_str(), 
			"%lf %lf %lf %d %d %d %d %lf %d",
			&x, &y, &z, &r, &g, &b, &sn, &ts, &ser);
	}

	/* check if at end of file */
	if(this->infile.eof())
		return -2;

	/* check that we parsed the correct number of items */
	if(ret != 9)
	{
		PRINT_ERROR("[pointcloud_reader_t::next_point]\tBAD LINE");
		LOGI("\tLine #%d:", this->num_points_read);
		LOGI("\t\"%s\"\n\n\n\n", line.c_str());
		return -3;
	}

	/* store parsed items */
	p.set(0, x / this->units);
	p.set(1, y / this->units);
	p.set(2, z / this->units);
	c.set((unsigned char) r, (unsigned char) g, (unsigned char) b);

	/* success */
	this->num_points_read++;
	return 0;
}

void pointcloud_reader_t::close()
{
	/* check if we have an open file */
	if(this->infile.is_open())
		this->infile.close();
}

/********************* POINTCLOUD WRITER ***********************/

pointcloud_writer_t::pointcloud_writer_t()
{
}

pointcloud_writer_t::~pointcloud_writer_t()
{
	this->close();
}

int pointcloud_writer_t::open(const char* filename, double u)
{
	/* check if a file is already open */
	if(this->outfile.is_open())
		this->outfile.close();

	/* attempt to open a new ascii file for writing */
	this->outfile.open(filename);
	if(!(this->outfile.is_open()))
		return -1;

	/* record desired units */
	this->units = u;
	
	/* reset counter */
	this->num_points_written = 0;

	/* success */
	return 0;
}

int pointcloud_writer_t::write_scan(scan_t& scan)
{
	int i, n;

	/* verify that file is open */
	if(!(this->outfile.is_open()))
		return -1;

	/* iterate over the points, write them to file */
	n = scan.pts.size();
	for(i = 0; i < n; i++)
	{
		this->outfile
			<< scan.pts[i].get(0)*(this->units) << " "
			<< scan.pts[i].get(1)*(this->units) << " "
			<< scan.pts[i].get(2)*(this->units) << " "
			<< 255 << " " << 255 << " " << 255 << " "
			<< (scan.get_scan_num()+1) << " "
			<< scan.get_timestamp() << " "
			<< scan.get_serial_number() << endl;
		this->num_points_written++;
	}
	
	/* success */
	return 0;
}
	
int pointcloud_writer_t::write_point(point_t& p, color_t& c, 
					int sn, double ts, int ser)
{
	/* verify that file is open */
	if(!(this->outfile.is_open()))
		return -1;

	/* write point */
	this->outfile << p.get(0)*(this->units) << " "
	              << p.get(1)*(this->units) << " "
		      << p.get(2)*(this->units) << " "
		      << ((int) c.red)   << " "
		      << ((int) c.green) << " "
		      << ((int) c.blue)  << " "
		      << sn  << " "
		      << ts  << " "
		      << ser << endl;

	/* success */
	return 0;
}

void pointcloud_writer_t::close()
{
	/* check if we have an open file */
	if(this->outfile.is_open())
		this->outfile.close();
}
