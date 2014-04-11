#include "chunk_exporter.h"
#include <io/carve/chunk_io.h>
#include <geometry/octree/shape.h>
#include <geometry/octree/octree.h>
#include <geometry/octree/octnode.h>
#include <geometry/octree/octdata.h>
#include <geometry/shapes/carve_wedge.h>
#include <util/error_codes.h>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <Eigen/Dense>

/**
 * @file chunk_exporter.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief Classes to populate and export volume chunks
 *
 * @section DESCRIPTION
 *
 * This file implements the chunk_exporter_t class, which is a shape
 * that can be inserted into an octree.  By inserting this shape,
 * the tree's data elements will be created but not populated, and
 * the information about which shapes intersect which nodes will
 * be exported as chunk files.
 *
 * This class requires the Eigen framework.
 */

using namespace std;
using namespace Eigen;
using namespace chunk;

/* function implementations */

chunk_exporter_t::chunk_exporter_t()
{
	/* initialize default values */
	this->chunk_map.clear();
	this->reference_shape    = NULL;
	this->vals.clear();
	this->chunklist_filename = "";
	this->full_chunk_dir     = "";
	this->rel_chunk_dir      = "";
}

chunk_exporter_t::~chunk_exporter_t()
{
	/* check that everything has been properly closed */
	if(!(this->chunk_map.empty()))
	{
		/* Uh oh!  The user didn't call close! */
		cerr << "[chunk_exporter_t::~chunk_exporter_t]\t"
		     << "You forgot to call chunk_exporter_t::close()"
		     << endl << endl;

		/* call close with what info we have */
		this->close(0,0,0,-1);
	}
}

void chunk_exporter_t::set(shape_t* rs, const vector<point_index_t>& v)
{
	/* copy over shape */
	this->reference_shape = rs;

	/* copy over vals */
	vals.clear();
	vals.insert(vals.begin(), v.begin(), v.end());
}
		
#define FILE_SEP_CHARS "\\/"
void chunk_exporter_t::open(const string& clfile, const string& chunk_dir)
{
	size_t found;

	/* copy over file names */
	this->chunklist_filename = clfile;
	this->rel_chunk_dir = chunk_dir;

	/* determine path to chunk dir from working directory */
	found = clfile.find_last_of(FILE_SEP_CHARS);
	if(found == string::npos)
	{
		/* in working directory, no work needs to be done */
		this->full_chunk_dir = chunk_dir;
		return;
	}
	
	/* prepend path to chunklists directory to the chunk_dir */
	this->full_chunk_dir = clfile.substr(0, found+1) + chunk_dir;
	if(this->full_chunk_dir.find_last_of(FILE_SEP_CHARS)
			!= this->full_chunk_dir.size()-1)
	{	
		/* make sure the full chunk directory ends with
		 * a file separator, so we can easily generate
		 * the filepaths to chunk files */
		this->full_chunk_dir.push_back(FILE_SEPERATOR);
	}
}
		
int chunk_exporter_t::close(const octree_t& tree)
{
	octnode_t* root;
	int ret;

	/* get the root of the tree */
	root = tree.get_root();

	/* call other version of this function */
	if(root == NULL)
	{
		ret = this->close(0,0,0,0);
		if(ret)
			return PROPEGATE_ERROR(-1, ret);
	}
	else
	{
		ret = this->close(root->center(0), root->center(1),
		            root->center(2), root->halfwidth);
		if(ret)
			return PROPEGATE_ERROR(-2, ret);
	}

	/* success */
	return 0;
}
	
int chunk_exporter_t::close(double cx, double cy, double cz, double hw)
{
	chunklist_writer_t outfile;
	map<octdata_t*, chunk_writer_t*>::iterator mit;
	int ret;

	/* close all open chunk files */
	for(mit = this->chunk_map.begin();
			mit != this->chunk_map.end(); mit++)
		mit->second->close();

	/* prepare the chunklist file for writing */
	outfile.init(cx, cy, cz, hw, this->rel_chunk_dir,
	             this->chunk_map.size());

	/* open the file for writing */
	ret = outfile.open(this->chunklist_filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* write each chunk uuid to file */
	for(mit = this->chunk_map.begin();
			mit != this->chunk_map.end(); mit++)
	{
		/* write uuid to file */
		outfile.write(chunk_exporter_t::ptr_to_uuid(mit->first));
	}

	/* clean up */
	outfile.close();
	for(mit = this->chunk_map.begin();
			mit != this->chunk_map.end(); mit++)
	{
		/* free allocated writer */
		delete (mit->second);
		mit->second = NULL;
	}
	this->chunk_map.clear();

	/* success */
	return 0;
}

octdata_t* chunk_exporter_t::apply_to_leaf(const Vector3d& c,
                                           double hw, octdata_t* d)
{
	string filename;
	unsigned long long uuid;
	map<octdata_t*, chunk_writer_t*>::iterator mit;
	pair<map<octdata_t*, chunk_writer_t*>::iterator, bool> ins;
	vector<point_index_t>::iterator vit;
	int ret;

	/* check if chunk file already exists for this data */
	if(d == NULL)
	{
		/* prepare a new data object */
		d = new octdata_t();
	
		/* create a chunk file based on this data object */
		uuid = (unsigned long long) d;
		filename = this->full_chunk_dir 
				+ chunk_exporter_t::ptr_to_uuid(d)
				+ CHUNKFILE_EXTENSION;
		
		/* insert into map */
		ins = this->chunk_map.insert(pair<octdata_t*,
				chunk_writer_t*>(d, new chunk_writer_t()));
		if(!(ins.second))
		{
			/* We've already inserted this data pointer!?!?
			 * But we just created it!  This means we
			 * have duplicate memory issues going on.  Bad
			 * news!  Inform the user */
			cerr << "[chunk_exporter_t::apply_to_leaf]\t"
			     << "Found duplicate allocated pointers! "
			     << "That's bad news!" << endl << endl;
			return d;
		}

		/* keep the iterator */
		mit = ins.first;

		/* prepare file for writing */
		mit->second->init(uuid, c(0), c(1), c(2), hw);
		ret = mit->second->open(filename);
		if(ret)
		{
			/* unable to open file! */
			cerr << "[chunk_exporter_t::apply_to_leaf]\t"
			     << "Error " << ret << ": Unable to write "
			     << "to chunk file" << endl << endl;
			return d;
		}
	}
	else
	{
		/* find this data pointer in the map */
		mit = this->chunk_map.find(d);
		if(mit == this->chunk_map.end())
		{
			/* Oh uh.  Any data pointer should have already
			 * been inserted in the map when it was created.
			 * This is likely a symptom of misuse of this
			 * function.  But for now, add this pointer
			 * and continue. */
			cerr << "[chunk_exporter_t::apply_to_leaf]\t"
			     << "WARNING: need to insert pre-existing data"
			     << endl << endl;
			ins = this->chunk_map.insert(pair<octdata_t*,
				chunk_writer_t*>(d, new chunk_writer_t()));
			mit = ins.first;
		}
	}

	/* add all values to file */
	for(vit = this->vals.begin(); vit != this->vals.end(); vit++)
		mit->second->write(*vit);

	/* success */
	return d;
}

