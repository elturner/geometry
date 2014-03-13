#include "floorplan.h"
#include <fstream>
#include <vector>
#include <string>

/**
 * @file floorplan_output.cpp
 * @author Eric Turner
 *
 * @section DESCRIPTION
 *
 * This file defines classes used to define
 * a 2D floorplan with extruded height information.
 * A floorplan is composed of a set of rooms, where each
 * room is a 2D triangulation with a set floor and ceiling
 * height.
 *
 * This file implements the file writing and exporting
 * helper functions.
 */

using namespace fp;
using namespace std;

int floorplan_t::export_to_obj(const string& filename) const
{
	vector<edge_t> edges;
	ofstream outfile;
	int i, num_verts, num_tris, num_edges;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write floor vertices */
	num_verts = this->verts.size();
	for(i = 0; i < num_verts; i++)
	{
		/* write out floor vertex */
		outfile << "v " << this->verts[i].x
		        <<  " " << this->verts[i].y
			<<  " " << this->verts[i].min_z
			<< endl;
	}

	/* write out ceiling vertices */
	for(i = 0; i < num_verts; i++)
	{
		/* write out floor vertex */
		outfile << "v " << this->verts[i].x
		        <<  " " << this->verts[i].y
			<<  " " << this->verts[i].max_z
			<< endl;
	}

	/* write out floor triangles */
	num_tris = this->tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out floor triangle, referencing
		 * floor vertices, which are indexed from 1 */
		outfile << "f " << (1+this->tris[i].verts[0])
		        <<  " " << (1+this->tris[i].verts[1])
			<<  " " << (1+this->tris[i].verts[2])
			<< endl;
	}
	
	/* write out ceiling triangles */
	num_tris = this->tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out ceiling triangle, referencing
		 * ceiling vertices, which are indexed from 
		 * (1+num_verts), and are downward facing */
		outfile << "f " << (1+num_verts+this->tris[i].verts[2])
		        <<  " " << (1+num_verts+this->tris[i].verts[1])
			<<  " " << (1+num_verts+this->tris[i].verts[0])
			<< endl;
	}

	/* write out walls */
	this->compute_edges(edges);
	num_edges = edges.size();
	for(i = 0; i < num_edges; i++)
	{
		/* print each edge as a rectangle */
		outfile << "f " << (1+edges[i].verts[0])
		        <<  " " << (1+num_verts+edges[i].verts[0])
			<<  " " << (1+num_verts+edges[i].verts[1])
			<< endl;
		outfile << "f " << (1+edges[i].verts[0])
			<<  " " << (1+num_verts+edges[i].verts[1])
		        <<  " " << (1+edges[i].verts[1])
			<< endl;
	}

	/* clean up */
	outfile.close();
	return 0;
}
