#include "building_model.h"
#include "window.h"
#include <mesh/floorplan/floorplan.h>
#include <util/error_codes.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cmath>

using namespace std;
using namespace fp;

building_model_t::building_model_t()
{
	/* each element's constructor is called */
}

building_model_t::~building_model_t()
{
	/* each element's destructor is called */
}

void building_model_t::clear()
{
	/* clear each element */
	this->floorplan.clear();
	this->level_name.clear();
	this->windows.clear();
}
	
int building_model_t::import_floorplan(const string& filename)
{
	int ret;

	/* assume filename is a .fp file, import it */
	ret = this->floorplan.import_from_fp(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

int building_model_t::import_windows(const string& filename)
{
	int ret;

	/* assume window list file */
	ret = this->windows.import_from_file(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* success */
	return 0;
}

int building_model_t::import_lights(const string& filename)
{
	int ret;

	/* assume lights list file */
	ret = this->lights.import(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* check that number of rooms are consistent */
	if(this->lights.size() != this->floorplan.rooms.size())
	{
		cerr << "[building_model_t::import_lights]\tThe "
		     << "imported lights file contains a different "
		     << "number of rooms than the floorplan." << endl;
		return -2;
	}

	/* success */
	return 0;
}

int building_model_t::import_people(const string& filename)
{
	int ret;

	/* assume people list file */
	ret = this->people.import(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);

	/* check that number of rooms are consistent */
	if(this->people.size() != this->floorplan.rooms.size())
	{
		cerr << "[building_model_t::import_people]\tThe "
		     << "imported people file contains a different "
		     << "number of rooms than the floorplan." << endl;
		return -2;
	}

	/* success */
	return 0;
}

int building_model_t::import_plugloads(const string& filename)
{
	int ret;

	/* assume plug loads list file */
	ret = this->plugloads.import(filename);
	if(ret)
		return PROPEGATE_ERROR(-1, ret);
	
	/* check that number of rooms are consistent */
	if(this->plugloads.size() != this->floorplan.rooms.size())
	{
		cerr << "[building_model_t::import_plugloads]\tThe "
		     << "imported plug-loads file contains a different "
		     << "number of rooms than the floorplan." << endl;
		return -2;
	}

	/* success */
	return 0;
}

int building_model_t::export_obj(const string& filename) const
{
	vector<edge_t> edges;
	vector<window_t> ws;
	ofstream outfile;
	unsigned int i, j, num_verts, num_tris, num_edges, num_window_verts;
	double wx[NUM_VERTS_PER_RECT];
	double wy[NUM_VERTS_PER_RECT];
	double wz[NUM_VERTS_PER_RECT];
	double min_x, min_y, max_x, max_y;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* compute bounds of floorplan (used for texturing) */
	this->floorplan.compute_bounds(min_x, min_y, max_x, max_y);

	/* specify texture source */
	outfile << "mtllib texture.mtl" << endl << endl;

	/* write floor vertices */
	num_verts = this->floorplan.verts.size();
	for(i = 0; i < num_verts; i++)
	{
		/* write out floor vertex */
		outfile << "v " << this->floorplan.verts[i].x
		        <<  " " << this->floorplan.verts[i].y
			<<  " " << this->floorplan.verts[i].min_z
			<< endl;

		/* write texture coordinate */
		outfile << "vt " << (this->floorplan.verts[i].x - min_x)
		        <<   " " << (this->floorplan.verts[i].y - min_y)
			<< endl;
	}

	/* write out ceiling vertices */
	for(i = 0; i < num_verts; i++)
	{
		/* write out ceiling vertex */
		outfile << "v " << this->floorplan.verts[i].x
		        <<  " " << this->floorplan.verts[i].y
			<<  " " << this->floorplan.verts[i].max_z
			<< endl;
	}

	/* set texture for floor triangles */
	outfile << endl << "usemtl Floor" << endl << endl;

	/* write out floor triangles */
	num_tris = this->floorplan.tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out floor triangle, referencing
		 * floor vertices, which are indexed from 1 */
		outfile << "f " << (1+this->floorplan.tris[i].verts[0])
		        <<  "/" << (1+this->floorplan.tris[i].verts[0])
		        <<  " " << (1+this->floorplan.tris[i].verts[1])
		        <<  "/" << (1+this->floorplan.tris[i].verts[1])
			<<  " " << (1+this->floorplan.tris[i].verts[2])
		        <<  "/" << (1+this->floorplan.tris[i].verts[2])
			<< endl;
	}
	
	/* set texture for ceiling triangles */
	outfile << endl << "usemtl Ceiling" << endl << endl;

	/* write out ceiling triangles */
	num_tris = this->floorplan.tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out ceiling triangle, referencing
		 * ceiling vertices, which are indexed from 
		 * (1+num_verts), and are downward facing */
		outfile << "f " << (1 + num_verts
				+ this->floorplan.tris[i].verts[2])
		        <<  "/" << (1+this->floorplan.tris[i].verts[2])
		        <<  " " << (1 + num_verts
				+ this->floorplan.tris[i].verts[1])
		        <<  "/" << (1+this->floorplan.tris[i].verts[1])
			<<  " " << (1 + num_verts
				+ this->floorplan.tris[i].verts[0])
		        <<  "/" << (1+this->floorplan.tris[i].verts[0])
			<< endl;
	}

	/* set texture for walls, and define rectangular texture
	 * coordinates */
	outfile << endl << "usemtl Wall" << endl << endl
	        << "vt 1 0" << endl
		<< "vt 1 1" << endl
		<< "vt 0 1" << endl
		<< "vt 0 0" << endl
		<< endl;

	/* write out walls */
	this->floorplan.compute_edges(edges);
	num_edges = edges.size();
	num_window_verts = 1+num_verts+num_verts;
	for(i = 0; i < num_edges; i++)
	{
		/* check if this wall has windows */
		ws.clear();
		this->windows.get_windows_for(edges[i], ws);
		
		/* simple case of no windows */
		if(ws.empty())
		{
			/* print edge as a rectangle */
			outfile << "f " << (1+edges[i].verts[0])
			        <<  "/" << (num_verts+1)
			        <<  " " << (1+num_verts+edges[i].verts[0])
			        <<  "/" << (num_verts+2)
				<<  " " << (1+num_verts+edges[i].verts[1])
			        <<  "/" << (num_verts+3)
				<< endl;
			outfile << "f " << (1+edges[i].verts[0])
			        <<  "/" << (num_verts+1)
				<<  " " << (1+num_verts+edges[i].verts[1])
			        <<  "/" << (num_verts+3)
			        <<  " " << (1+edges[i].verts[1])
			        <<  "/" << (num_verts+4)
				<< endl;
		}
		else
		{
			/* currently unable to handle multiple windows */
			if(ws.size() > 1)
			{
				cerr << "[building_model.export_obj]\t"
					"Unable to handle walls that "
					"have multiple windows defined. "
					"Ignoring extra windows." << endl;
			}

			/* create wall with single window */

			/* make new vertices to define window geometry */
			ws[0].get_world_coords(wx, wy, wz, this->floorplan);
			for(j = 0; j < NUM_VERTS_PER_RECT; j++)
				outfile << "v " << wx[j] 
				        <<  " " << wy[j] 
					<<  " " << wz[j] 
					<< endl;

			/* one window:  cut a hole in wall for window */
			outfile << "f " << (1+edges[i].verts[0])
			        <<  " " << (1+num_verts+edges[i].verts[0])
				<<  " " << (num_window_verts+0)
				<< endl
			        << "f " << (num_window_verts+0)
			        <<  " " << (1+num_verts+edges[i].verts[0])
				<<  " " << (num_window_verts+1)
				<< endl;
			outfile << "f " << (1+num_verts+edges[i].verts[0])
			        <<  " " << (1+num_verts+edges[i].verts[1])
				<<  " " << (num_window_verts+1)
				<< endl
			        << "f " << (num_window_verts+1)
			        <<  " " << (1+num_verts+edges[i].verts[1])
				<<  " " << (num_window_verts+2)
				<< endl;
			outfile << "f " << (1+num_verts+edges[i].verts[1])
			        <<  " " << (1+edges[i].verts[1])
				<<  " " << (num_window_verts+2)
				<< endl
			        << "f " << (num_window_verts+2)
			        <<  " " << (1+edges[i].verts[1])
				<<  " " << (num_window_verts+3)
				<< endl;
			outfile << "f " << (1+edges[i].verts[1])
			        <<  " " << (1+edges[i].verts[0])
				<<  " " << (num_window_verts+3)
				<< endl
			        << "f " << (num_window_verts+3)
			        <<  " " << (1+edges[i].verts[0])
				<<  " " << (num_window_verts+0)
				<< endl;

			/* set window texture */
			outfile << endl << "usemtl Window" << endl << endl;

			/* add geometry for window */
			outfile << "f " << (num_window_verts+0)
			        <<  "/" << (1+num_verts+0)
			        <<  " " << (num_window_verts+1)
			        <<  "/" << (1+num_verts+1)
				<<  " " << (num_window_verts+2)
			        <<  "/" << (1+num_verts+2)
				<< endl
				<< "f " << (num_window_verts+0)
			        <<  "/" << (1+num_verts+0)
				<<  " " << (num_window_verts+2)
			        <<  "/" << (1+num_verts+2)
				<<  " " << (num_window_verts+3)
			        <<  "/" << (1+num_verts+3)
				<< endl;
	
			/* reset wall texture */
			outfile << endl << "usemtl Wall" << endl << endl;
				
			/* update counters */
			num_window_verts += 4;
		}
	}

	/* clean up */
	outfile.close();
	return 0;
}
