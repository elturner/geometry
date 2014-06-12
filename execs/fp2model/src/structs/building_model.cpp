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

int building_model_t::export_wrl(const string& filename) const
{
	ofstream outfile;

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;
	
	/* write some comments at the top */
	outfile << "#VRML V2.0 utf8" << endl
	        << "#Auto-generated by Eric Turner's fp2model program"
		<< endl;
	
	/* write navigation info */
	outfile << "NavigationInfo {" << endl
	        << "\ttype [ \"EXAMINE\", \"ANY\" ]" << endl
		<< "}" << endl;

	/* open a transform that describes this model */
	outfile << "Transform {" << endl
	        << "\tscale 1 1 1" << endl
		<< "\ttranslation 0 0 0" << endl
		<< "\tchildren" << endl
		<< "\t[" << endl;

	/* create a shape for the floor */
	this->write_floor_to_wrl(outfile);

	/* create a shape for ceiling */
	this->write_ceiling_to_wrl(outfile);
	
	/* write the walls */
	this->write_wall_to_wrl(outfile);

	/* clean up */
	outfile << "\t]" << endl
	        << "}" << endl;
	outfile.close();
	return 0;
}

void building_model_t::write_floor_to_wrl(std::ostream& outfile) const
{
	double min_x, min_y, max_x, max_y;
	unsigned int i, num_verts, num_tris;
	
	/* compute bounds of floorplan (used for texturing) */
	this->floorplan.compute_bounds(min_x, min_y, max_x, max_y);

	/* create a shape for the floor */
	outfile << "\t\tShape" << endl
	        << "\t\t{" << endl
		<< "\t\t\tgeometry IndexedFaceSet" << endl
		<< "\t\t\t{" << endl
		<< "\t\t\t\tcreaseAngle .5" << endl /* affects smoothing */
		<< "\t\t\t\tsolid FALSE" << endl; /* double-sided polys */

	/* write floor vertices */
	outfile << "\t\t\t\tcoord Coordinate" << endl
		<< "\t\t\t\t{" << endl
		<< "\t\t\t\t\tpoint" << endl
		<< "\t\t\t\t\t[" << endl;
	num_verts = this->floorplan.verts.size();
	for(i = 0; i < num_verts; i++)
	{
		/* write out floor vertex */
		outfile << "\t\t\t\t\t\t" << this->floorplan.verts[i].x
		        << " " << this->floorplan.verts[i].y
			<< " " << this->floorplan.verts[i].min_z
			<< "," << endl;
	}
	outfile << "\t\t\t\t\t]" << endl
	        << "\t\t\t\t}" << endl;
	
	/* write out floor colors */
	outfile << "\t\t\t\tcolor Color" << endl
	        << "\t\t\t\t{" << endl
		<< "\t\t\t\t\tcolor" << endl
		<< "\t\t\t\t\t[" << endl;
	for(i = 0; i < num_verts; i++)
	{
		/* write out color for each vertex */
		outfile << "\t\t\t\t\t\t0 1 0," << endl;
	}
	outfile << "\t\t\t\t\t]" << endl
	        << "\t\t\t\t}" << endl;

	/* write out floor triangles */
	outfile << "\t\t\t\tcoordIndex" << endl
	        << "\t\t\t\t[" << endl;
	num_tris = this->floorplan.tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out floor triangle, referencing
		 * floor vertices, which are indexed from 0 */
		outfile << "\t\t\t\t\t" 
		        << (this->floorplan.tris[i].verts[0]) << ","
		        << (this->floorplan.tris[i].verts[1]) << ","
			<< (this->floorplan.tris[i].verts[2]) << ","
			<< "-1, " << endl;
	}
	outfile << "\t\t\t\t]" << endl;

	/* write texture coordinates */
	outfile << "\t\t\t\ttexCoord TextureCoordinate" << endl
	        << "\t\t\t\t{" << endl
		<< "\t\t\t\t\tpoint" << endl
		<< "\t\t\t\t\t[" << endl;
	for(i = 0; i < num_verts; i++)
	{
		/* texture coordinate */
		outfile << "\t\t\t\t\t\t"
		        << std::fmod(this->floorplan.verts[i].x - min_x,1.0)
		        << ","
			<< std::fmod(this->floorplan.verts[i].y - min_y,1.0)
			<< "," << endl;
	}
	outfile << "\t\t\t\t\t]" << endl
	        << "\t\t\t\t}" << endl
		<< "\t\t\t\ttexCoordIndex" << endl
		<< "\t\t\t\t[" << endl;
	for(i = 0; i < num_verts; i++)
	{
		/* texture coordinate index */
		outfile << "\t\t\t\t\t"
		        << (this->floorplan.tris[i].verts[0]) << ","
		        << (this->floorplan.tris[i].verts[1]) << ","
			<< (this->floorplan.tris[i].verts[2]) << ","
			<< "-1, " << endl;
	}
	outfile << "\t\t\t\t]" << endl;

	/* write texture url */
	outfile << "\t\t\t\tappearance Appearance" << endl
	        << "\t\t\t\t{" << endl
		<< "\t\t\t\t\tmaterial Material" << endl
		<< "\t\t\t\t\t{" << endl
		<< "\t\t\t\t\t\tambientIntensity 0.2" << endl
		<< "\t\t\t\t\t\tdiffuseColor 0.9 0.9 0.9" << endl
		<< "\t\t\t\t\t\tspecularColor 0.1 0.1 0.1" << endl
		<< "\t\t\t\t\t\tshininess 0.5" << endl
		<< "\t\t\t\t\t}" << endl
		<< "\t\t\t\t\ttexture ImageTexture { url \"carpet.jpg\" }"
		<< endl
		<< "\t\t\t\t}" << endl
		<< "\t\t\t}" << endl
		<< "\t\t}" << endl;
}

void building_model_t::write_ceiling_to_wrl(std::ostream& outfile) const
{
	double min_x, min_y, max_x, max_y;
	unsigned int i, num_verts, num_tris;
	
	/* compute bounds of floorplan (used for texturing) */
	this->floorplan.compute_bounds(min_x, min_y, max_x, max_y);

	/* create a shape for the floor */
	outfile << "\t\tShape" << endl
	        << "\t\t{" << endl
		<< "\t\t\tgeometry IndexedFaceSet" << endl
		<< "\t\t\t{" << endl
		<< "\t\t\t\tcreaseAngle .5" << endl /* affects smoothing */
		<< "\t\t\t\tsolid FALSE" << endl; /* double-sided polys */

	/* write floor vertices */
	outfile << "\t\t\t\tcoord Coordinate" << endl
		<< "\t\t\t\t{" << endl
		<< "\t\t\t\t\tpoint" << endl
		<< "\t\t\t\t\t[" << endl;
	num_verts = this->floorplan.verts.size();
	for(i = 0; i < num_verts; i++)
	{
		/* write out floor vertex */
		outfile << "\t\t\t\t\t\t" << this->floorplan.verts[i].x
		        << " " << this->floorplan.verts[i].y
			<< " " << this->floorplan.verts[i].max_z
			<< "," << endl;
	}
	outfile << "\t\t\t\t\t]" << endl
	        << "\t\t\t\t}" << endl;
	
	/* write out floor colors */
	outfile << "\t\t\t\tcolor Color" << endl
	        << "\t\t\t\t{" << endl
		<< "\t\t\t\t\tcolor" << endl
		<< "\t\t\t\t\t[" << endl;
	for(i = 0; i < num_verts; i++)
	{
		/* write out color for each vertex */
		outfile << "\t\t\t\t\t\t1 0.5 0," << endl;
	}
	outfile << "\t\t\t\t\t]" << endl
	        << "\t\t\t\t}" << endl;

	/* write out floor triangles */
	outfile << "\t\t\t\tcoordIndex" << endl
	        << "\t\t\t\t[" << endl;
	num_tris = this->floorplan.tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out floor triangle, referencing
		 * floor vertices, which are indexed from 0 */
		outfile << "\t\t\t\t\t" 
		        << (this->floorplan.tris[i].verts[2]) << ","
		        << (this->floorplan.tris[i].verts[1]) << ","
			<< (this->floorplan.tris[i].verts[0]) << ","
			<< "-1, " << endl;
	}
	outfile << "\t\t\t\t]" << endl;

	/* write texture coordinates */
	outfile << "\t\t\t\ttexCoord TextureCoordinate" << endl
	        << "\t\t\t\t{" << endl
		<< "\t\t\t\t\tpoint" << endl
		<< "\t\t\t\t\t[" << endl;
	for(i = 0; i < num_verts; i++)
	{
		/* texture coordinate */
		outfile << "\t\t\t\t\t\t"
		        << std::fmod(this->floorplan.verts[i].x - min_x,1.0)
		        << " "
			<< std::fmod(this->floorplan.verts[i].y - min_y,1.0)
			<< "," << endl;
	}
	outfile << "\t\t\t\t\t]" << endl
	        << "\t\t\t\t}" << endl
		<< "\t\t\t\ttexCoordIndex" << endl
		<< "\t\t\t\t[" << endl;
	for(i = 0; i < num_verts; i++)
	{
		/* texture coordinate index */
		outfile << "\t\t\t\t\t"
		        << (this->floorplan.tris[i].verts[2]) << ","
		        << (this->floorplan.tris[i].verts[1]) << ","
			<< (this->floorplan.tris[i].verts[0]) << ","
			<< "-1, " << endl;
	}
	outfile << "\t\t\t\t]" << endl;

	/* write texture url */
	outfile << "\t\t\t\tappearance Appearance" << endl
	        << "\t\t\t\t{" << endl
		<< "\t\t\t\t\tmaterial Material" << endl
		<< "\t\t\t\t\t{" << endl
		<< "\t\t\t\t\t\tambientIntensity 0.2" << endl
		<< "\t\t\t\t\t\tdiffuseColor 0.9 0.9 0.9" << endl
		<< "\t\t\t\t\t\tspecularColor 0.1 0.1 0.1" << endl
		<< "\t\t\t\t\t\tshininess 0.5" << endl
		<< "\t\t\t\t\t}" << endl
		<< "\t\t\t\t\ttexture ImageTexture { url \"ceiling.jpg\" }"
		<< endl
		<< "\t\t\t\t}" << endl
		<< "\t\t\t}" << endl
		<< "\t\t}" << endl;
}
	
void building_model_t::write_wall_to_wrl(ostream& outfile) const
{
	int i, num_edges;
	vector<edge_t> edges;
	vector<window_t> ws;

	/* check arguments */
	if(!outfile.good())
		return;

	/* set texture for walls, and define rectangular texture
	 * coordinates */
	
	/* write out walls */
	this->floorplan.compute_edges(edges);
	num_edges = edges.size();
	for(i = 0; i < num_edges; i++)
	{
		/* make a shape for this wall */
		// TODO

		/* check if this wall has windows */
		ws.clear();
		this->windows.get_windows_for(edges[i], ws);
		
		/* simple case of no windows */
		if(ws.empty())
		{
			/* print edge as a rectangle */
			// TODO
		}
		else
		{
			// TODO
		}
	}
}
