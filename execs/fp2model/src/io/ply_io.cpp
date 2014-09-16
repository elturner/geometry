#include "ply_io.h"
#include "../structs/building_model.h"
#include <mesh/floorplan/floorplan.h>
#include <fstream>
#include <string>
#include <vector>
#include <set>

/**
 * @file   ply_io.cpp
 * @author Eric Turner <elturner@eecs.berkeley.edu>
 * @brief  Export floorplan information to a PLY file
 *
 * @section DESCRIPTION
 *
 * This file contains functions used to export floorplan information
 * to a PLY file.  This file type is the Stanford Polygon Format,
 * which is used to represented 3D meshes.  This file type is
 * also necessary for Peter Cheng's texture-mapping code.
 */

using namespace std;
using namespace fp;

/*-------------------------*/
/* function implementation */
/*-------------------------*/

int writeply(const std::string& filename, const building_model_t& bim)
{
	ofstream outfile;
	vector<edge_t> walls, room_edges;
	set<int>::iterator tit;
	size_t i, j, n, num_verts, num_tris, num_regions, num_edges;
	double nx, ny, mag, px, py;

	/* compute walls of floorplan */
	bim.floorplan.compute_edges(walls);

	/* determine cardinality of floorplan */
	num_verts = 2*bim.floorplan.verts.size(); /* floor and ceiling */
	num_tris = 2*bim.floorplan.tris.size() + 2*walls.size();
		/* 2 per wall, one per floor, and one per ceiling */
	num_regions = 2*bim.floorplan.rooms.size() + walls.size();
				/* floors, walls, and ceilings */

	/* open file for writing */
	outfile.open(filename.c_str());
	if(!(outfile.is_open()))
		return -1;

	/* write header */
	outfile << "ply" << endl
	        << "format ascii 1.0"  << endl
		<< "element vertex "   << num_verts << endl
		<< "property float x"  << endl
		<< "property float y"  << endl
		<< "property float z"  << endl
		<< "element face "     << num_tris << endl
		<< "property list uchar int vertex_index" << endl
		<< "element region "   << num_regions << endl
		<< "property float nx" << endl
		<< "property float ny" << endl
		<< "property float nz" << endl
		<< "property float px" << endl
		<< "property float py" << endl
		<< "property float pz" << endl
		<< "property list int int triangle_index" << endl
		<< "property list int int edge_pair_index" << endl
		<< "end_header" << endl;

	/* write vertices to disk */
	n = bim.floorplan.verts.size();
	for(i = 0; i < n; i++)
	{
		/* write two vertices, one on floor and one on ceiling */
		outfile << bim.floorplan.verts[i].x << " "
		        << bim.floorplan.verts[i].y << " "
			<< bim.floorplan.verts[i].min_z << endl
		        << bim.floorplan.verts[i].x << " "
		        << bim.floorplan.verts[i].y << " "
			<< bim.floorplan.verts[i].max_z << endl;
	
		/* even vertices are on floor, odd vertices are on
		 * ceiling */
	}

	/* write floor and ceiling triangles to disk */
	n = bim.floorplan.tris.size();
	for(i = 0; i < n; i++)
	{
		/* write floor triangles (even vertices) */
		outfile << "3 "
		        << 2*bim.floorplan.tris[i].verts[0] << " "  
		        << 2*bim.floorplan.tris[i].verts[1] << " "  
		        << 2*bim.floorplan.tris[i].verts[2] << endl;

		/* write ceiling triangles (odd vertices, reverse) */
		outfile << "3 "
		        << (1 + 2*bim.floorplan.tris[i].verts[2]) << " "  
		        << (1 + 2*bim.floorplan.tris[i].verts[1]) << " "  
		        << (1 + 2*bim.floorplan.tris[i].verts[0]) << endl;
	
		/* even indexed triangles on floor, odd triangles
		 * on ceiling */
	}

	/* write wall triangles */
	n = walls.size();
	for(i = 0; i < n; i++)
	{
		/* each wall is two triangles */
		outfile << "3 "
			<< (2*walls[i].verts[0])   << " "   /* floor 0 */
			<< (1+2*walls[i].verts[1]) << " "   /* ceiling 1 */
			<< (2*walls[i].verts[1])   << endl  /* floor 1 */
			<< "3 "
			<< (2*walls[i].verts[0])   << " "   /* floor 0 */
			<< (1+2*walls[i].verts[0]) << " "   /* ceiling 0 */
			<< (1+2*walls[i].verts[1]) << endl; /* ceiling 1 */
	}

	/* write floor and ceiling regions to disk */
	n = bim.floorplan.rooms.size();
	for(i = 0; i < n; i++)
	{
		/* room info */
		room_edges.clear();
		bim.floorplan.compute_edges_for_room(room_edges, i);
		num_edges = room_edges.size();
		
		/* floor region */

		/* print out plane geometry info for floor */
		outfile << "0 0 1 " /* normal always faces up */
		        << "0 0 " << bim.floorplan.rooms[i].min_z;
				/* point on plane */

		/* print out triangles contained in this region */
		outfile << " " << bim.floorplan.rooms[i].tris.size();
		for(tit = bim.floorplan.rooms[i].tris.begin();
				tit != bim.floorplan.rooms[i].tris.end();
					tit++)
			outfile << " " << (2*(*tit)); /* floor triangle */

		/* get edges of room */
		outfile << " " << 2*num_edges; /* each vert in each edge */
		for(j = 0; j < num_edges; j++)
		{
			/* print edge indices */
			outfile << " " << (2*room_edges[j].verts[0])
			        << " " << (2*room_edges[j].verts[1]);
					/* floor vertices are even */
		}

		/* end of region */
		outfile << endl;

		/* ceiling region */
		outfile << "0 0 -1 " /* normal always down for ceiling */
		        << "0 0 " << bim.floorplan.rooms[i].max_z;
				/* point on plane */

		/* print out triangles in region */
		outfile << " " << bim.floorplan.rooms[i].tris.size();
		for(tit = bim.floorplan.rooms[i].tris.begin();
				tit != bim.floorplan.rooms[i].tris.end();
					tit++)
			outfile << " " << (1 + 2*(*tit)); 
				/* ceiling triangle is odd */
		
		/* get edges of room */
		outfile << " " << 2*num_edges; /* each vert in each edge */
		for(j = 0; j < num_edges; j++)
		{
			/* print edge indices (reverse order) */
			outfile << " " << (1 + 2*room_edges[j].verts[1])
			        << " " << (1 + 2*room_edges[j].verts[0]);
					/* ceiling vertices are odd */
		}

		/* end this region */
		outfile << endl;
	}

	/* export wall regions to disk */
	n = walls.size();
	for(i = 0; i < n; i++)
	{
		/* compute normal of wall based on edge direction */
		nx = bim.floorplan.verts[walls[i].verts[1]].x
				- bim.floorplan.verts[walls[i].verts[0]].x;
		ny = bim.floorplan.verts[walls[i].verts[1]].y
				- bim.floorplan.verts[walls[i].verts[0]].y;
		mag = sqrt(nx*nx + ny*ny);

		/* point of plane is just average of wall points */
		px = (bim.floorplan.verts[walls[i].verts[1]].x
			+ bim.floorplan.verts[walls[i].verts[0]].x)/2;
		py = (bim.floorplan.verts[walls[i].verts[1]].y
			+ bim.floorplan.verts[walls[i].verts[0]].y)/2;
	
		/* write out plane info for wall region */
		outfile << (-ny/mag) << " " << (nx/mag) << " 0 "
					/* norm orthogonal to wall */
		        << px << " " << py << " 0"; /* point on plane */

		/* print out triangles for region */
		outfile << " 2" /* always two triangles */
		        << " " << (2*i + 2*bim.floorplan.tris.size())
		        << " " << (1 + 2*i + 2*bim.floorplan.tris.size());
			/* wall tris come after all the floor and ceiling
			 * triangles */
		
		/* print out edges for region */
		outfile << " 8" /* always four edges (eight indices) */
		        << " " << (1+2*walls[i].verts[0])
		        << " " << (1+2*walls[i].verts[1]) /* ceiling edge */
		        << " " << (2*walls[i].verts[1])
		        << " " << (2*walls[i].verts[0]) /* floor edge */
		        << " " << (2*walls[i].verts[0])
		        << " " << (1+2*walls[i].verts[0]) /* going up */
		        << " " << (1+2*walls[i].verts[1])
		        << " " << (2*walls[i].verts[1]) /* coming down */
		        << endl; /* end of region */
	}
	/* clean up */
	outfile.close();
	return 0;
}
