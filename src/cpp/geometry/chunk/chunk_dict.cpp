#include "chunk_dict.h"
#include <io/carve/chunk_io.h>
#include <util/error_codes.h>
#include <Eigen/Dense>
#include <iostream>
#include <cmath>

/**
 * @file chunk_dict.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  The chunk dictionary class identifies chunk files via locations
 *
 * @section DESCRIPTION
 *
 * This file contains the chunk_dict_t class, which is used to identify
 * which chunks contain which points in an efficient manner.  Given a
 * point in 3D space, it is able to return the file path to the chunk
 * file that intersects that point.
 */

using namespace std;
using namespace Eigen;
using namespace chunk;

/*---------------------------------------*/
/* chunk_dict_t function implementations */
/*---------------------------------------*/

int chunk_dict_t::init(const std::string& filename)
{
	chunklist_reader_t infile;
	chunk_reader_t chunkreader;
	string chunkfile;
	Vector3d keypos;
	size_t i, n;
	int ret;

	/* attempt to access input file */
	ret = infile.open(filename);
	if(ret)
	{
		/* unable to access list file */
		ret = PROPEGATE_ERROR(-1, ret);
		cerr << "[chunk_dict_t::init]\tError " << ret << ": "
		     << "Unable to open chunklist: " << filename << endl;
		return ret;
	}

	/* store root geometry */
	this->center(0) = infile.center_x();
	this->center(1) = infile.center_y();
	this->center(2) = infile.center_z();
	this->width = 0; /* get this from first chunk */
	n = infile.num_chunks();

	/* iterate over file */
	for(i = 0; i < n; i++)
	{
		/* get chunk filename */
		ret = infile.next(chunkfile);
		if(ret)
		{
			/* error occurred */
			infile.close();
			ret = PROPEGATE_ERROR(-2, ret);
			cerr << "[chunk_dict_t::init]\tError " << ret << ":"
			     << " Unable to get id of chunk #" << i << endl;
			return ret;
		}

		/* load chunk info */
		ret = chunkreader.open(chunkfile);
		if(ret)
		{
			/* unable to open file */
			infile.close();
			ret = PROPEGATE_ERROR(-3, ret);
			cerr << "[chunk_dict_t::init]\tError " << ret << ":"
			     << " Unable to open chunkfile #" << i << ": "
			     << chunkfile << endl;
			return ret;
		}

		/* store chunk geometry */
		this->width = 2*chunkreader.halfwidth();
		keypos(0) = chunkreader.center_x();
		keypos(1) = chunkreader.center_x();
		keypos(2) = chunkreader.center_x();

		/* convert the chunk position into a key, which
		 * requires converting into normalized coordinates */
		this->dict.insert(pair<chunk_key_t, string>(
				this->genkey(keypos), chunkfile));

		/* we don't need any more info from this chunk...yet */
		chunkreader.close();
	}

	/* clean up */
	infile.close();
	return 0; /* success */
}
		
void chunk_dict_t::retrieve(const Vector3d& p, set<string>& ss) const
{
	pair<multimap<chunk_key_t, string>::const_iterator,
		multimap<chunk_key_t, string>::const_iterator > range;
	multimap<chunk_key_t, string>::const_iterator it;

	/* look up in map */
	range = this->dict.equal_range(this->genkey(p));

	/* add range to set */
	for(it = range.first; it != range.second; it++)
		ss.insert(it->second);
}

/*--------------------------------------*/
/* chunk_key_t function implementations */		
/*--------------------------------------*/
		
chunk_key_t::chunk_key_t(const Vector3d& p)
{
	/* discretize the position based on the width */
	this->x_ind = (long) floorl(p(0));
	this->y_ind = (long) floorl(p(1));
	this->z_ind = (long) floorl(p(2));
}
		
void chunk_key_t::print(ostream& os) const
{
	os << "("  << this->x_ind 
	   << ", " << this->y_ind 
	   << ", " << this->z_ind << ")";
}
