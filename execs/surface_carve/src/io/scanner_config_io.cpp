#include "scanner_config_io.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "../structs/point.h"
#include "../util/error_codes.h"
#include "../util/parameters.h"

using namespace std;

/*** HELPER FUNCTIONS ***/

/* the following are tags used in the
 * file format for the scanner config */
#define COMMENT_SYMBOL '%'
#define WHITESPACE_SYMBOLS " \t\n"
#define ALL_SEPARATORS " \t\n,[]()"
#define LASER_TAG "#laser"
#define NAME_TAG "&name"
#define TYPE_TAG "&type"
#define SERIAL_NUM_TAG "&serialNum"
#define R_TO_COMMON_TAG "&rToCommon"
#define T_TO_COMMON_TAG "&tToCommon"
#define END_SENSOR_TAG "#endsensor"

/* trim_line:
 *
 * 	This function will remove
 * 	all trailing whitespace
 * 	and comments from a string.
 */
void trim_line(string& str)
{
	size_t ci;

	/* remove any comments */
	ci = str.find_first_of('%');
	if(ci != string::npos)
	{
		/* found the location of a comment,
		 * truncate the string */
		str = str.substr(0, ci);
	}

	/* check to see if there is any whitespace
	 * at the beginning of the string */
	ci = str.find_first_not_of(WHITESPACE_SYMBOLS);
	if(ci == string::npos)
		/* line is entirely whitespace */
		str.clear();
	else
		/* remove beginning whitespace */
		str = str.substr(ci);
	
	/* check to see if there is any trailing whitespace */
	ci = str.find_last_not_of(WHITESPACE_SYMBOLS);
	if(ci == string::npos)
		/* line is entirely whitespace */
		str.clear();
	else
		/* remove trailing whitespace */
		str = str.substr(0, ci+1);
}

/* parse_strtok_vector3
 *
 * 	Will use the string already being parsed by strtok()
 * 	to read three decimal values which are stored in a
 * 	list.
 *
 * arguments:
 *
 * 	x, y, z -	Where to store the values that are parsed
 *
 * return value:
 *
 * 	Returns 0 on success, non-zero on failure.
 */
int parse_strtok_vector3(double& x, double& y, double& z)
{
	char* tok, *endptr;
	
	/* get the next token */
	tok = strtok(NULL, ALL_SEPARATORS);
	x = strtod(tok, &endptr);
	if(endptr == tok)
		return -1;

	/* get the next token */
	tok = strtok(NULL, ALL_SEPARATORS);
	y = strtod(tok, &endptr);
	if(endptr == tok)
		return -2;

	/* get the next token */
	tok = strtok(NULL, ALL_SEPARATORS);
	z = strtod(tok, &endptr);
	if(endptr == tok)
		return -3;
	
	/* success */
	return 0;
}

/*** LASER PROPERTY FUNCTIONS ***/

laser_prop_t::laser_prop_t()
{
	this->clear();
}

void laser_prop_t::clear()
{
	/* clear all properties */
	this->name.clear();
	this->type.clear();
	this->serialNum.clear();
	this->roll = 0;
	this->pitch = 0;
	this->yaw = 0;
	this->pos.x = 0;
	this->pos.y = 0;
	this->pos.z = 0;
}

/*** SCANNER CONFIG FUNCTIONS ***/

int scanner_config_t::import(char* filename)
{
	ifstream infile;
	string line;
	char buf[LINE_BUFFER_SIZE];
	char* name, *tok;
	laser_prop_t las;
	bool laser_defined;
	int ret;

	/* check arguments */
	if(!filename)
		return -1;

	/* open file for reading */
	infile.open(filename);
	if(!infile.is_open())
		return -2;

	/* read each line of the file */
	while(!infile.eof())
	{
		/* read the next line */
		line.clear();
		getline(infile, line);

		/* trim this line of whitespace
		 * and comments */
		trim_line(line);

		/* check if the file is starting a laser block */
		if(line.compare(LASER_TAG))
			/* not a start of a laser block,
			 * ignore this line. */
			continue;

		/* keep reading until we reach the
		 * end of the scanner */
		las.clear();
		laser_defined = false;
		while(!infile.eof())
		{
			/* read the next line */
			line.clear();
			getline(infile, line);

			/* trim this line of whitespace
			 * and comments */
			trim_line(line);

			/* check if reached end of sensor information */
			if(!line.compare(END_SENSOR_TAG))
			{
				laser_defined = true;
				break;
			}

			/* check if trivial line */
			if(line.empty())
				continue;

			/* move remainder into buffer */
			if(line.size() >= LINE_BUFFER_SIZE)
				return -3;
			strcpy(buf, line.c_str());

			/* determine what property of 
			 * laser is being described by
			 * this line */
			name = strtok(buf, WHITESPACE_SYMBOLS);
			if(!strcmp(name, NAME_TAG))
			{
				/* ignore the '=' */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				
				/* store the name */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				las.name.assign(tok);
			}
			else if(!strcmp(name, TYPE_TAG))
			{
				/* ignore the '=' */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				
				/* store the type */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				las.type.assign(tok);
			}
			else if(!strcmp(name, SERIAL_NUM_TAG))
			{
				/* ignore the '=' */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				
				/* store the serial number */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				las.serialNum.assign(tok);
			}
			else if(!strcmp(name, R_TO_COMMON_TAG))
			{
				/* ignore the '=' */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				
				/* attempt to read in numbers from file */
				ret = parse_strtok_vector3(las.roll, 
						las.pitch, las.yaw);
				if(ret)
					return PROPEGATE_ERROR(-4, ret);

				/* convert to radians */
				las.roll = DEG2RAD(las.roll);
				las.pitch = DEG2RAD(las.pitch);
				las.yaw = DEG2RAD(las.yaw);
			}
			else if(!strcmp(name, T_TO_COMMON_TAG))
			{
				/* ignore the '=' */
				tok = strtok(NULL, WHITESPACE_SYMBOLS);
				
				/* attempt to read in numbers from file */
				ret = parse_strtok_vector3(las.pos.x, 
						las.pos.y, las.pos.z);
				if(ret)
					return PROPEGATE_ERROR(-5, ret);

				/* convert to meters */
				las.pos.x = MM2METERS(las.pos.x);
				las.pos.y = MM2METERS(las.pos.y);
				las.pos.z = MM2METERS(las.pos.z);
			}
			else
			{
				/* bad field */
				return -6;
			}
		}

		/* check if the laser was successfully defined */
		if(!laser_defined)
			return -7;
		
		/* add this laser property struct to our list */
		this->lasers.push_back(las);
	}

	/* clean up */
	infile.close();
	return 0;
}
	
int scanner_config_t::index_of_laser(char* str)
{
	char* sub;
	unsigned int i, n;

	/* for each laser, search str for serial number */
	n = this->lasers.size();
	for(i = 0; i < n; i++)
	{
		sub = strstr(str, this->lasers[i].serialNum.c_str());
		if(sub != NULL)
			return i;
	}

	/* couldn't find anything */
	return -1;
}

void scanner_config_t::print()
{
	unsigned int i, n;

	/* iterate through all lasers */
	n = this->lasers.size();
	for(i = 0; i < n; i++)
	{
		/* print information about the current laser */
		cout << "Laser #" << i << endl
		     << "------------------------" << endl
		     << "name:      " << this->lasers[i].name << endl
		     << "type:      " << this->lasers[i].type << endl
		     << "serialNum: " << this->lasers[i].serialNum << endl
		     << endl
		     << "roll:      " << this->lasers[i].roll << endl
		     << "pitch:     " << this->lasers[i].pitch << endl
		     << "yaw:       " << this->lasers[i].yaw << endl
		     << endl
		     << "x:         " << this->lasers[i].pos.x << endl
		     << "y:         " << this->lasers[i].pos.y << endl
		     << "z:         " << this->lasers[i].pos.z << endl
		     << endl;
	}
}
