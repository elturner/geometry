#include "wrl_io.h"
#include "../structs/building_model.h"
#include <mesh/floorplan/floorplan.h>
#include <util/error_codes.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/**
 * @file     wrl_io.cpp
 * @author   Eric Turner <elturner@indoorreality.com>
 * @brief    Exports building models to VRML (.wrl) files
 *
 * @section DESCRIPTION
 *
 * This file contains basic functionality to export building
 * models to Virtual Reality Markup Language (VRML) world
 * files (.wrl).
 *
 * File created June 25, 2015
 */

using namespace std;
using namespace wrl_io;
using namespace fp;

/*-------------------------*/
/* function implementation */
/*-------------------------*/

int wrl_io::export_wrl(const string& filename,
			const building_model_t& bm)
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
	write_floor_to_wrl(outfile, bm);

	/* create a shape for ceiling */
	write_ceiling_to_wrl(outfile, bm);
	
	/* write the walls */
	write_wall_to_wrl(outfile, bm);

	/* clean up */
	outfile << "\t]" << endl
	        << "}" << endl;
	outfile.close();
	return 0;
}

void wrl_io::write_floor_to_wrl(std::ostream& outfile,
			const building_model_t& bm)
{
	double min_x, min_y, max_x, max_y;
	unsigned int i, num_verts, num_tris;
	
	/* compute bounds of floorplan (used for texturing) */
	bm.floorplan.compute_bounds(min_x, min_y, max_x, max_y);

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
	num_verts = bm.floorplan.verts.size();
	for(i = 0; i < num_verts; i++)
	{
		/* write out floor vertex */
		outfile << "\t\t\t\t\t\t" << bm.floorplan.verts[i].x
		        << " " << bm.floorplan.verts[i].y
			<< " " << bm.floorplan.verts[i].min_z
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
	num_tris = bm.floorplan.tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out floor triangle, referencing
		 * floor vertices, which are indexed from 0 */
		outfile << "\t\t\t\t\t" 
		        << (bm.floorplan.tris[i].verts[0]) << ","
		        << (bm.floorplan.tris[i].verts[1]) << ","
			<< (bm.floorplan.tris[i].verts[2]) << ","
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
		        << std::fmod(bm.floorplan.verts[i].x - min_x,1.0)
		        << ","
			<< std::fmod(bm.floorplan.verts[i].y - min_y,1.0)
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
		        << (bm.floorplan.tris[i].verts[0]) << ","
		        << (bm.floorplan.tris[i].verts[1]) << ","
			<< (bm.floorplan.tris[i].verts[2]) << ","
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

void wrl_io::write_ceiling_to_wrl(std::ostream& outfile,
			const building_model_t& bm)
{
	double min_x, min_y, max_x, max_y;
	unsigned int i, num_verts, num_tris;
	
	/* compute bounds of floorplan (used for texturing) */
	bm.floorplan.compute_bounds(min_x, min_y, max_x, max_y);

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
	num_verts = bm.floorplan.verts.size();
	for(i = 0; i < num_verts; i++)
	{
		/* write out floor vertex */
		outfile << "\t\t\t\t\t\t" << bm.floorplan.verts[i].x
		        << " " << bm.floorplan.verts[i].y
			<< " " << bm.floorplan.verts[i].max_z
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
	num_tris = bm.floorplan.tris.size();
	for(i = 0; i < num_tris; i++)
	{
		/* write out floor triangle, referencing
		 * floor vertices, which are indexed from 0 */
		outfile << "\t\t\t\t\t" 
		        << (bm.floorplan.tris[i].verts[2]) << ","
		        << (bm.floorplan.tris[i].verts[1]) << ","
			<< (bm.floorplan.tris[i].verts[0]) << ","
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
		        << std::fmod(bm.floorplan.verts[i].x - min_x,1.0)
		        << " "
			<< std::fmod(bm.floorplan.verts[i].y - min_y,1.0)
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
		        << (bm.floorplan.tris[i].verts[2]) << ","
		        << (bm.floorplan.tris[i].verts[1]) << ","
			<< (bm.floorplan.tris[i].verts[0]) << ","
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
	
void wrl_io::write_wall_to_wrl(ostream& outfile,
			const building_model_t& bm)
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
	bm.floorplan.compute_edges(edges);
	num_edges = edges.size();
	for(i = 0; i < num_edges; i++)
	{
		/* make a shape for this wall */
		outfile << "\t\tShape" << endl
		        << "\t\t{" << endl
			<< "\t\t\tgeometry IndexedFaceSet" << endl
			<< "\t\t\t{" << endl
			<< "\t\t\t\tcreaseAngle .5" << endl 
					/* affects smoothing */
			<< "\t\t\t\tsolid FALSE" << endl; 
					/* double-sided polys */

		/* write the vertex coordinates for this wall */
		outfile << "\t\t\t\tcoord Coordinate" << endl
			<< "\t\t\t\t{" << endl
			<< "\t\t\t\t\tpoint" << endl
			<< "\t\t\t\t\t[" << endl;

		/* export the four corners of the wall (in counter-clockwise
		 * ordering) */
		outfile << " " << bm.floorplan.verts[edges[i].verts[0]].x
		        << " " << bm.floorplan.verts[edges[i].verts[0]].y
		        << " " 
		        << bm.floorplan.verts[edges[i].verts[0]].min_z 
		        << ","
		        << " " << bm.floorplan.verts[edges[i].verts[0]].x
		        << " " << bm.floorplan.verts[edges[i].verts[0]].y
		        << " " 
		        << bm.floorplan.verts[edges[i].verts[0]].max_z
		        << ","
		        << " " << bm.floorplan.verts[edges[i].verts[1]].x
		        << " " << bm.floorplan.verts[edges[i].verts[1]].y
		        << " " 
		        << bm.floorplan.verts[edges[i].verts[1]].max_z
		        << ","
		        << " " << bm.floorplan.verts[edges[i].verts[1]].x
		        << " " << bm.floorplan.verts[edges[i].verts[1]].y
		        << " " 
		        << bm.floorplan.verts[edges[i].verts[1]].min_z
		        << " ]" << endl
		        << "\t\t\t\t}" << endl;

		/* export the wall as a quad referencing the
		 * four vertices above as (0,1,2,3) */
		outfile << "\t\t\t\tcoordIndex [ 0 1 2 3 -1 ]" << endl;
	
		/* write texture coordinates */
		outfile << "\t\t\t\ttexCoord TextureCoordinate" << endl
		        << "\t\t\t\t{" << endl
		        << "\t\t\t\t\tpoint" << endl
		        << "\t\t\t\t\t[" << endl
		        << "\t\t\t\t\t\t1.0 0.0," << endl
		        << "\t\t\t\t\t\t1.0 1.0," << endl
		        << "\t\t\t\t\t\t0.0 1.0," << endl
		        << "\t\t\t\t\t\t0.0 0.0"  << endl
		        << "\t\t\t\t\t]" << endl
		        << "\t\t\t\t}" << endl
		        << "\t\t\t\ttexCoordIndex" << endl
		        << "\t\t\t\t[ 0 1 2 3 -1]" << endl;

		/* check if this wall has windows */
		ws.clear();
		bm.windows.get_windows_for(edges[i], ws);
		if(!ws.empty())
		{
			/* functionality not yet supported */
			cerr << "[wrl_io::write_wall_to_wrl]\t"
			     << "Warning:  Wall #" << i << " has windows "
			     << "but exporting windows is not yet "
			     << "supported for .wrl files." << endl;
		}
	
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
			<< "\t\t\t\t\ttexture ImageTexture "
			<< "{ url \"wall.jpg\" }" << endl
			<< "\t\t\t\t}" << endl
			<< "\t\t\t}" << endl
			<< "\t\t}" << endl;
	}
}